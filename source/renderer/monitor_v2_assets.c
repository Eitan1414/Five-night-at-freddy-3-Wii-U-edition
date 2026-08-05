#include "assets/monitor_v2_assets.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#define MONITOR_V2_CHUNK(index) \
    extern const uint8_t monitor_v2_assets_##index##_b64[]; \
    extern const uint8_t monitor_v2_assets_##index##_b64_end[]
MONITOR_V2_CHUNK(00);
MONITOR_V2_CHUNK(01);
MONITOR_V2_CHUNK(02);
MONITOR_V2_CHUNK(03);
#undef MONITOR_V2_CHUNK

#define MONITOR_V2_HEADER_SIZE 8u
#define MONITOR_V2_DESCRIPTOR_SIZE 24u
#define MONITOR_V2_PALETTE_BYTES 1024u
#define MONITOR_V2_UNCOMPRESSED_SIZE 107072u

static TextureRle sMonitorV2Textures[MONITOR_V2_TEXTURE_COUNT];
static uint8_t *sMonitorV2Data = NULL;
static bool sMonitorV2Attempted = false;
static bool sMonitorV2Ready = false;

static uint16_t monitor_v2_read_be16(const uint8_t *data)
{
    return (uint16_t) (((uint16_t) data[0] << 8u) | data[1]);
}

static uint32_t monitor_v2_read_be32(const uint8_t *data)
{
    return ((uint32_t) data[0] << 24u) |
           ((uint32_t) data[1] << 16u) |
           ((uint32_t) data[2] << 8u) |
           (uint32_t) data[3];
}

static bool monitor_v2_range_valid(size_t size, uint32_t offset,
                                   uint32_t bytes)
{
    return offset <= size && bytes <= size - offset;
}

static int monitor_v2_base64_value(uint8_t c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -2;
    return -1;
}

static bool monitor_v2_decode_base64(const uint8_t *input,
                                     size_t input_size,
                                     uint8_t **output,
                                     size_t *output_size)
{
    uint8_t *decoded = (uint8_t *) malloc((input_size / 4u + 1u) * 3u);
    if (decoded == NULL) return false;

    size_t written = 0u;
    int quartet[4];
    int count = 0;
    for (size_t index = 0u; index < input_size; ++index) {
        const int value = monitor_v2_base64_value(input[index]);
        if (value == -1) continue;
        quartet[count++] = value;
        if (count != 4) continue;
        if (quartet[0] < 0 || quartet[1] < 0) {
            free(decoded);
            return false;
        }
        decoded[written++] = (uint8_t)
            ((quartet[0] << 2) | (quartet[1] >> 4));
        if (quartet[2] != -2) {
            if (quartet[2] < 0) {
                free(decoded);
                return false;
            }
            decoded[written++] = (uint8_t)
                ((quartet[1] << 4) | (quartet[2] >> 2));
            if (quartet[3] != -2) {
                if (quartet[3] < 0) {
                    free(decoded);
                    return false;
                }
                decoded[written++] = (uint8_t)
                    ((quartet[2] << 6) | quartet[3]);
            }
        }
        count = 0;
    }
    if (count != 0) {
        free(decoded);
        return false;
    }

    *output = decoded;
    *output_size = written;
    return true;
}

bool monitor_v2_assets_init(void)
{
    if (sMonitorV2Attempted) return sMonitorV2Ready;
    sMonitorV2Attempted = true;
    memset(sMonitorV2Textures, 0, sizeof(sMonitorV2Textures));

    static const uint8_t *const starts[4] = {
        monitor_v2_assets_00_b64, monitor_v2_assets_01_b64,
        monitor_v2_assets_02_b64, monitor_v2_assets_03_b64,
    };
    static const uint8_t *const ends[4] = {
        monitor_v2_assets_00_b64_end, monitor_v2_assets_01_b64_end,
        monitor_v2_assets_02_b64_end, monitor_v2_assets_03_b64_end,
    };

    size_t encoded_size = 0u;
    for (int index = 0; index < 4; ++index)
        encoded_size += (size_t) (ends[index] - starts[index]);

    uint8_t *encoded = (uint8_t *) malloc(encoded_size);
    if (encoded == NULL) return false;

    size_t encoded_offset = 0u;
    for (int index = 0; index < 4; ++index) {
        const size_t bytes = (size_t) (ends[index] - starts[index]);
        memcpy(encoded + encoded_offset, starts[index], bytes);
        encoded_offset += bytes;
    }

    uint8_t *packed = NULL;
    size_t packed_size = 0u;
    const bool decoded = monitor_v2_decode_base64(encoded, encoded_size,
                                                   &packed, &packed_size);
    free(encoded);
    if (!decoded) return false;

    sMonitorV2Data = (uint8_t *) malloc(MONITOR_V2_UNCOMPRESSED_SIZE);
    if (sMonitorV2Data == NULL) {
        free(packed);
        return false;
    }

    uLongf unpacked_size = MONITOR_V2_UNCOMPRESSED_SIZE;
    const int result = uncompress(sMonitorV2Data, &unpacked_size,
                                  packed, (uLong) packed_size);
    free(packed);
    if (result != Z_OK || unpacked_size != MONITOR_V2_UNCOMPRESSED_SIZE) {
        free(sMonitorV2Data);
        sMonitorV2Data = NULL;
        return false;
    }

    const uint8_t *data = sMonitorV2Data;
    const size_t size = MONITOR_V2_UNCOMPRESSED_SIZE;
    if (size < MONITOR_V2_HEADER_SIZE ||
        memcmp(data, "F3M2", 4u) != 0 ||
        monitor_v2_read_be16(data + 4u) != 1u ||
        monitor_v2_read_be16(data + 6u) != MONITOR_V2_TEXTURE_COUNT) {
        free(sMonitorV2Data);
        sMonitorV2Data = NULL;
        return false;
    }

    for (int index = 0; index < MONITOR_V2_TEXTURE_COUNT; ++index) {
        const uint8_t *descriptor = data + MONITOR_V2_HEADER_SIZE +
            (size_t) index * MONITOR_V2_DESCRIPTOR_SIZE;
        const uint16_t width = monitor_v2_read_be16(descriptor + 0u);
        const uint16_t height = monitor_v2_read_be16(descriptor + 2u);
        const uint8_t transparent = descriptor[4u];
        const uint32_t palette_offset =
            monitor_v2_read_be32(descriptor + 8u);
        const uint32_t rows_offset =
            monitor_v2_read_be32(descriptor + 12u);
        const uint32_t runs_offset =
            monitor_v2_read_be32(descriptor + 16u);
        const uint32_t runs_size =
            monitor_v2_read_be32(descriptor + 20u);
        const uint32_t rows_size = ((uint32_t) height + 1u) * 2u;

        if (width == 0u || height == 0u ||
            (palette_offset & 3u) != 0u || (rows_offset & 1u) != 0u ||
            !monitor_v2_range_valid(size, palette_offset,
                                    MONITOR_V2_PALETTE_BYTES) ||
            !monitor_v2_range_valid(size, rows_offset, rows_size) ||
            !monitor_v2_range_valid(size, runs_offset, runs_size)) {
            free(sMonitorV2Data);
            sMonitorV2Data = NULL;
            memset(sMonitorV2Textures, 0, sizeof(sMonitorV2Textures));
            return false;
        }

        sMonitorV2Textures[index].width = width;
        sMonitorV2Textures[index].height = height;
        sMonitorV2Textures[index].transparent_index = transparent;
        sMonitorV2Textures[index].palette =
            (const uint32_t *) (const void *) (data + palette_offset);
        sMonitorV2Textures[index].row_offsets =
            (const uint16_t *) (const void *) (data + rows_offset);
        sMonitorV2Textures[index].runs = data + runs_offset;
    }

    sMonitorV2Ready = true;
    return true;
}

const TextureRle *monitor_v2_texture(MonitorV2TextureId id)
{
    if (!sMonitorV2Attempted && !monitor_v2_assets_init()) return NULL;
    if (!sMonitorV2Ready || id < 0 || id >= MONITOR_V2_TEXTURE_COUNT)
        return NULL;
    return &sMonitorV2Textures[id];
}
