#include "assets/office_assets.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#define OFFICE_CHUNK(index) \
    extern const uint8_t office_assets_##index##_b64[]; \
    extern const uint8_t office_assets_##index##_b64_end[]
OFFICE_CHUNK(00);
OFFICE_CHUNK(01);
OFFICE_CHUNK(02);
#undef OFFICE_CHUNK

#define OFFICE_ASSET_HEADER_SIZE 8u
#define OFFICE_ASSET_DESCRIPTOR_SIZE 24u
#define OFFICE_ASSET_PALETTE_BYTES 1024u
#define OFFICE_ASSET_UNCOMPRESSED_SIZE 46628u

static TextureRle sOfficeTextures[OFFICE_ASSET_TEXTURE_COUNT];
static uint8_t *sOfficeData = NULL;
static bool sOfficeAttempted = false;
static bool sOfficeReady = false;

static uint16_t read_be16(const uint8_t *data)
{
    return (uint16_t) (((uint16_t) data[0] << 8u) | data[1]);
}

static uint32_t read_be32(const uint8_t *data)
{
    return ((uint32_t) data[0] << 24u) |
           ((uint32_t) data[1] << 16u) |
           ((uint32_t) data[2] << 8u) |
           (uint32_t) data[3];
}

static bool range_valid(size_t size, uint32_t offset, uint32_t bytes)
{
    return offset <= size && bytes <= size - offset;
}

static int base64_value(uint8_t c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -2;
    return -1;
}

static bool decode_base64(const uint8_t *input, size_t input_size,
                          uint8_t **output, size_t *output_size)
{
    uint8_t *decoded = (uint8_t *) malloc((input_size / 4u + 1u) * 3u);
    if (decoded == NULL) return false;

    size_t written = 0u;
    int q[4];
    int count = 0;
    for (size_t index = 0u; index < input_size; ++index) {
        const int value = base64_value(input[index]);
        if (value == -1) continue;
        q[count++] = value;
        if (count != 4) continue;
        if (q[0] < 0 || q[1] < 0) { free(decoded); return false; }
        decoded[written++] = (uint8_t) ((q[0] << 2) | (q[1] >> 4));
        if (q[2] != -2) {
            if (q[2] < 0) { free(decoded); return false; }
            decoded[written++] = (uint8_t) ((q[1] << 4) | (q[2] >> 2));
            if (q[3] != -2) {
                if (q[3] < 0) { free(decoded); return false; }
                decoded[written++] = (uint8_t) ((q[2] << 6) | q[3]);
            }
        }
        count = 0;
    }
    if (count != 0) { free(decoded); return false; }
    *output = decoded;
    *output_size = written;
    return true;
}

bool office_assets_init(void)
{
    if (sOfficeAttempted) return sOfficeReady;
    sOfficeAttempted = true;
    memset(sOfficeTextures, 0, sizeof(sOfficeTextures));

    static const uint8_t *const starts[3] = {
        office_assets_00_b64, office_assets_01_b64,
        office_assets_02_b64,
    };
    static const uint8_t *const ends[3] = {
        office_assets_00_b64_end, office_assets_01_b64_end,
        office_assets_02_b64_end,
    };

    size_t encoded_size = 0u;
    for (int index = 0; index < 3; ++index)
        encoded_size += (size_t) (ends[index] - starts[index]);
    uint8_t *encoded = (uint8_t *) malloc(encoded_size);
    if (encoded == NULL) return false;

    size_t offset = 0u;
    for (int index = 0; index < 3; ++index) {
        const size_t bytes = (size_t) (ends[index] - starts[index]);
        memcpy(encoded + offset, starts[index], bytes);
        offset += bytes;
    }

    uint8_t *packed = NULL;
    size_t packed_size = 0u;
    const bool decoded = decode_base64(encoded, encoded_size,
                                       &packed, &packed_size);
    free(encoded);
    if (!decoded) return false;

    sOfficeData = (uint8_t *) malloc(OFFICE_ASSET_UNCOMPRESSED_SIZE);
    if (sOfficeData == NULL) { free(packed); return false; }

    uLongf unpacked_size = OFFICE_ASSET_UNCOMPRESSED_SIZE;
    const int result = uncompress(sOfficeData, &unpacked_size,
                                  packed, (uLong) packed_size);
    free(packed);
    if (result != Z_OK || unpacked_size != OFFICE_ASSET_UNCOMPRESSED_SIZE) {
        free(sOfficeData);
        sOfficeData = NULL;
        return false;
    }

    const uint8_t *data = sOfficeData;
    const size_t size = OFFICE_ASSET_UNCOMPRESSED_SIZE;
    if (memcmp(data, "F3OF", 4u) != 0 || read_be16(data + 4u) != 1u ||
        read_be16(data + 6u) != OFFICE_ASSET_TEXTURE_COUNT) {
        free(sOfficeData);
        sOfficeData = NULL;
        return false;
    }

    for (int index = 0; index < OFFICE_ASSET_TEXTURE_COUNT; ++index) {
        const uint8_t *descriptor = data + OFFICE_ASSET_HEADER_SIZE +
            (size_t) index * OFFICE_ASSET_DESCRIPTOR_SIZE;
        const uint16_t width = read_be16(descriptor + 0u);
        const uint16_t height = read_be16(descriptor + 2u);
        const uint8_t transparent = descriptor[4u];
        const uint32_t palette_offset = read_be32(descriptor + 8u);
        const uint32_t rows_offset = read_be32(descriptor + 12u);
        const uint32_t runs_offset = read_be32(descriptor + 16u);
        const uint32_t runs_size = read_be32(descriptor + 20u);
        const uint32_t rows_size = ((uint32_t) height + 1u) * 2u;

        if (width == 0u || height == 0u ||
            (palette_offset & 3u) != 0u || (rows_offset & 1u) != 0u ||
            !range_valid(size, palette_offset, OFFICE_ASSET_PALETTE_BYTES) ||
            !range_valid(size, rows_offset, rows_size) ||
            !range_valid(size, runs_offset, runs_size)) {
            free(sOfficeData);
            sOfficeData = NULL;
            memset(sOfficeTextures, 0, sizeof(sOfficeTextures));
            return false;
        }

        sOfficeTextures[index].width = width;
        sOfficeTextures[index].height = height;
        sOfficeTextures[index].transparent_index = transparent;
        sOfficeTextures[index].palette =
            (const uint32_t *) (const void *) (data + palette_offset);
        sOfficeTextures[index].row_offsets =
            (const uint16_t *) (const void *) (data + rows_offset);
        sOfficeTextures[index].runs = data + runs_offset;
    }

    sOfficeReady = true;
    return true;
}

const TextureRle *office_assets_texture(OfficeAssetTextureId id)
{
    if (!sOfficeAttempted && !office_assets_init()) return NULL;
    if (!sOfficeReady || id < 0 || id >= OFFICE_ASSET_TEXTURE_COUNT)
        return NULL;
    return &sOfficeTextures[id];
}
