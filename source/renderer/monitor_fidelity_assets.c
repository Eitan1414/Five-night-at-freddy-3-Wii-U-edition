#include "assets/monitor_fidelity_assets.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

extern const uint8_t monitor_fidelity_assets_b64[];
extern const uint8_t monitor_fidelity_assets_b64_end[];

#define MONITOR_FIDELITY_HEADER_SIZE 8u
#define MONITOR_FIDELITY_DESCRIPTOR_SIZE 24u
#define MONITOR_FIDELITY_PALETTE_BYTES 1024u
#define MONITOR_FIDELITY_UNCOMPRESSED_SIZE 39062u

static TextureRle sMonitorFidelityTextures[MONITOR_FIDELITY_TEXTURE_COUNT];
static uint8_t *sMonitorFidelityData = NULL;
static bool sMonitorFidelityAttempted = false;
static bool sMonitorFidelityReady = false;

static uint16_t monitor_fidelity_read_be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8u) | data[1]);
}

static uint32_t monitor_fidelity_read_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24u) |
           ((uint32_t)data[1] << 16u) |
           ((uint32_t)data[2] << 8u) |
           (uint32_t)data[3];
}

static bool monitor_fidelity_range_valid(size_t size, uint32_t offset,
                                         uint32_t bytes)
{
    return offset <= size && bytes <= size - offset;
}

static int monitor_fidelity_base64_value(uint8_t c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -2;
    return -1;
}

static bool monitor_fidelity_decode_base64(const uint8_t *input,
                                           size_t input_size,
                                           uint8_t **output,
                                           size_t *output_size)
{
    uint8_t *decoded = (uint8_t *)malloc((input_size / 4u + 1u) * 3u);
    if (decoded == NULL) return false;

    size_t written = 0u;
    int quartet[4];
    int count = 0;
    for (size_t index = 0u; index < input_size; ++index) {
        const int value = monitor_fidelity_base64_value(input[index]);
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

bool monitor_fidelity_assets_init(void)
{
    if (sMonitorFidelityAttempted) return sMonitorFidelityReady;
    sMonitorFidelityAttempted = true;
    memset(sMonitorFidelityTextures, 0, sizeof(sMonitorFidelityTextures));

    const size_t encoded_size = (size_t)
        (monitor_fidelity_assets_b64_end - monitor_fidelity_assets_b64);
    uint8_t *packed = NULL;
    size_t packed_size = 0u;
    if (!monitor_fidelity_decode_base64(monitor_fidelity_assets_b64,
                                        encoded_size,
                                        &packed,
                                        &packed_size)) {
        return false;
    }

    sMonitorFidelityData =
        (uint8_t *)malloc(MONITOR_FIDELITY_UNCOMPRESSED_SIZE);
    if (sMonitorFidelityData == NULL) {
        free(packed);
        return false;
    }

    uLongf unpacked_size = MONITOR_FIDELITY_UNCOMPRESSED_SIZE;
    const int zresult = uncompress(sMonitorFidelityData, &unpacked_size,
                                   packed, (uLong)packed_size);
    free(packed);
    if (zresult != Z_OK ||
        unpacked_size != MONITOR_FIDELITY_UNCOMPRESSED_SIZE) {
        free(sMonitorFidelityData);
        sMonitorFidelityData = NULL;
        return false;
    }

    const uint8_t *const data = sMonitorFidelityData;
    const size_t size = MONITOR_FIDELITY_UNCOMPRESSED_SIZE;
    if (size < MONITOR_FIDELITY_HEADER_SIZE ||
        memcmp(data, "F3MF", 4u) != 0 ||
        monitor_fidelity_read_be16(data + 4u) != 1u ||
        monitor_fidelity_read_be16(data + 6u) !=
            MONITOR_FIDELITY_TEXTURE_COUNT) {
        free(sMonitorFidelityData);
        sMonitorFidelityData = NULL;
        return false;
    }

    for (int index = 0; index < MONITOR_FIDELITY_TEXTURE_COUNT; ++index) {
        const uint8_t *descriptor = data + MONITOR_FIDELITY_HEADER_SIZE +
            (size_t)index * MONITOR_FIDELITY_DESCRIPTOR_SIZE;
        const uint16_t width = monitor_fidelity_read_be16(descriptor + 0u);
        const uint16_t height = monitor_fidelity_read_be16(descriptor + 2u);
        const uint8_t transparent = descriptor[4u];
        const uint32_t palette_offset =
            monitor_fidelity_read_be32(descriptor + 8u);
        const uint32_t rows_offset =
            monitor_fidelity_read_be32(descriptor + 12u);
        const uint32_t runs_offset =
            monitor_fidelity_read_be32(descriptor + 16u);
        const uint32_t runs_size =
            monitor_fidelity_read_be32(descriptor + 20u);
        const uint32_t rows_size = ((uint32_t)height + 1u) * 2u;

        if (width == 0u || height == 0u ||
            (palette_offset & 3u) != 0u ||
            (rows_offset & 1u) != 0u ||
            !monitor_fidelity_range_valid(size, palette_offset,
                                           MONITOR_FIDELITY_PALETTE_BYTES) ||
            !monitor_fidelity_range_valid(size, rows_offset, rows_size) ||
            !monitor_fidelity_range_valid(size, runs_offset, runs_size)) {
            free(sMonitorFidelityData);
            sMonitorFidelityData = NULL;
            memset(sMonitorFidelityTextures, 0,
                   sizeof(sMonitorFidelityTextures));
            return false;
        }

        sMonitorFidelityTextures[index].width = width;
        sMonitorFidelityTextures[index].height = height;
        sMonitorFidelityTextures[index].transparent_index = transparent;
        sMonitorFidelityTextures[index].palette =
            (const uint32_t *)(const void *)(data + palette_offset);
        sMonitorFidelityTextures[index].row_offsets =
            (const uint16_t *)(const void *)(data + rows_offset);
        sMonitorFidelityTextures[index].runs = data + runs_offset;
    }

    sMonitorFidelityReady = true;
    return true;
}

const TextureRle *monitor_fidelity_texture(MonitorFidelityTextureId id)
{
    if (!sMonitorFidelityAttempted && !monitor_fidelity_assets_init())
        return NULL;
    if (!sMonitorFidelityReady || id < 0 ||
        id >= MONITOR_FIDELITY_TEXTURE_COUNT) {
        return NULL;
    }
    return &sMonitorFidelityTextures[id];
}
