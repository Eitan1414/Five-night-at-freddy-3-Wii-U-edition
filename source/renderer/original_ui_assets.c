#include "assets/original_ui_assets.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

extern const uint8_t original_ui_z0_bin[];
extern const uint8_t original_ui_z0_bin_end[];
extern const uint8_t original_ui_z1_bin[];
extern const uint8_t original_ui_z1_bin_end[];
extern const uint8_t original_ui_z2_bin[];
extern const uint8_t original_ui_z2_bin_end[];
extern const uint8_t original_ui_z3_bin[];
extern const uint8_t original_ui_z3_bin_end[];

#define ORIGINAL_UI_MAGIC_SIZE 4u
#define ORIGINAL_UI_HEADER_SIZE 8u
#define ORIGINAL_UI_DESCRIPTOR_SIZE 24u
#define ORIGINAL_UI_PALETTE_BYTES 64u
#define ORIGINAL_UI_UNCOMPRESSED_SIZE 81748u

static TextureRle sOriginalUiTextures[ORIGINAL_UI_TEXTURE_COUNT];
static bool sOriginalUiReady = false;
static uint8_t *sOriginalUiData = NULL;

static uint16_t original_ui_read_be16(const uint8_t *data)
{
    return (uint16_t) (((uint16_t) data[0] << 8u) | data[1]);
}

static uint32_t original_ui_read_be32(const uint8_t *data)
{
    return ((uint32_t) data[0] << 24u) |
           ((uint32_t) data[1] << 16u) |
           ((uint32_t) data[2] << 8u) |
           (uint32_t) data[3];
}

static bool original_ui_range_valid(size_t size, uint32_t offset,
                                    uint32_t bytes)
{
    return offset <= size && bytes <= size - offset;
}

bool original_ui_assets_init(void)
{
    memset(sOriginalUiTextures, 0, sizeof(sOriginalUiTextures));
    sOriginalUiReady = false;

    free(sOriginalUiData);
    sOriginalUiData = (uint8_t *) malloc(ORIGINAL_UI_UNCOMPRESSED_SIZE);
    if (sOriginalUiData == NULL) return false;

    static const uint8_t *const chunk_starts[4] = {
        original_ui_z0_bin, original_ui_z1_bin,
        original_ui_z2_bin, original_ui_z3_bin,
    };
    static const uint8_t *const chunk_ends[4] = {
        original_ui_z0_bin_end, original_ui_z1_bin_end,
        original_ui_z2_bin_end, original_ui_z3_bin_end,
    };
    size_t packed_size = 0u;
    for (int index = 0; index < 4; ++index)
        packed_size += (size_t) (chunk_ends[index] - chunk_starts[index]);
    uint8_t *packed = (uint8_t *) malloc(packed_size);
    if (packed == NULL) {
        free(sOriginalUiData);
        sOriginalUiData = NULL;
        return false;
    }
    size_t packed_offset = 0u;
    for (int index = 0; index < 4; ++index) {
        const size_t chunk_size =
            (size_t) (chunk_ends[index] - chunk_starts[index]);
        memcpy(packed + packed_offset, chunk_starts[index], chunk_size);
        packed_offset += chunk_size;
    }

    uLongf unpacked_size = ORIGINAL_UI_UNCOMPRESSED_SIZE;
    const int zlib_result = uncompress(sOriginalUiData, &unpacked_size,
                                       packed, (uLong) packed_size);
    free(packed);
    if (zlib_result != Z_OK ||
        unpacked_size != ORIGINAL_UI_UNCOMPRESSED_SIZE) {
        free(sOriginalUiData);
        sOriginalUiData = NULL;
        return false;
    }

    const uint8_t *const data = sOriginalUiData;
    const size_t size = ORIGINAL_UI_UNCOMPRESSED_SIZE;
    if (size < ORIGINAL_UI_HEADER_SIZE ||
        memcmp(data, "F3UI", ORIGINAL_UI_MAGIC_SIZE) != 0) {
        free(sOriginalUiData);
        sOriginalUiData = NULL;
        return false;
    }

    const uint16_t version = original_ui_read_be16(data + 4u);
    const uint16_t count = original_ui_read_be16(data + 6u);
    if (version != 1u || count != ORIGINAL_UI_TEXTURE_COUNT ||
        size < ORIGINAL_UI_HEADER_SIZE +
               (size_t) count * ORIGINAL_UI_DESCRIPTOR_SIZE) {
        return false;
    }

    for (uint16_t index = 0u; index < count; ++index) {
        const uint8_t *descriptor = data + ORIGINAL_UI_HEADER_SIZE +
            (size_t) index * ORIGINAL_UI_DESCRIPTOR_SIZE;
        const uint16_t width = original_ui_read_be16(descriptor + 0u);
        const uint16_t height = original_ui_read_be16(descriptor + 2u);
        const uint8_t transparent = descriptor[4u];
        const uint32_t palette_offset = original_ui_read_be32(descriptor + 8u);
        const uint32_t rows_offset = original_ui_read_be32(descriptor + 12u);
        const uint32_t runs_offset = original_ui_read_be32(descriptor + 16u);
        const uint32_t runs_size = original_ui_read_be32(descriptor + 20u);
        const uint32_t rows_size = ((uint32_t) height + 1u) * 2u;

        if (width == 0u || height == 0u ||
            (palette_offset & 3u) != 0u || (rows_offset & 1u) != 0u ||
            !original_ui_range_valid(size, palette_offset,
                                     ORIGINAL_UI_PALETTE_BYTES) ||
            !original_ui_range_valid(size, rows_offset, rows_size) ||
            !original_ui_range_valid(size, runs_offset, runs_size)) {
            memset(sOriginalUiTextures, 0, sizeof(sOriginalUiTextures));
            return false;
        }

        sOriginalUiTextures[index].width = width;
        sOriginalUiTextures[index].height = height;
        sOriginalUiTextures[index].transparent_index = transparent;
        sOriginalUiTextures[index].palette =
            (const uint32_t *) (const void *) (data + palette_offset);
        sOriginalUiTextures[index].row_offsets =
            (const uint16_t *) (const void *) (data + rows_offset);
        sOriginalUiTextures[index].runs = data + runs_offset;
    }

    sOriginalUiReady = true;
    return true;
}

const TextureRle *original_ui_texture(OriginalUiTextureId id)
{
    if (!sOriginalUiReady || id < 0 || id >= ORIGINAL_UI_TEXTURE_COUNT)
        return NULL;
    return &sOriginalUiTextures[id];
}
