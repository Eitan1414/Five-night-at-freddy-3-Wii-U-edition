#include "assets/good_ending_pc_asset.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

static bool sGoodEndingReady;
static TextureRle sGoodEndingTexture;
static uint8_t sGoodEndingPacked[59882];
static uint8_t sGoodEndingCompressed[25388];
static uint32_t sGoodEndingPalette[16];
static uint16_t sGoodEndingOffsets[481];
static uint8_t sGoodEndingRuns[58840];

static const char kGoodEndingPayload[] =
#include "generated/good_ending_pc_payload_01.inc"
#include "generated/good_ending_pc_payload_02.inc"
#include "generated/good_ending_pc_payload_03.inc"
#include "generated/good_ending_pc_payload_04.inc"
;

static int good_ending_b64_value(char value)
{
    if (value >= 'A' && value <= 'Z') return value - 'A';
    if (value >= 'a' && value <= 'z') return value - 'a' + 26;
    if (value >= '0' && value <= '9') return value - '0' + 52;
    if (value == '+') return 62;
    if (value == '/') return 63;
    return -1;
}

static size_t good_ending_decode_base64(uint8_t *output, size_t capacity)
{
    size_t written = 0u;
    uint32_t bits = 0u;
    int bit_count = 0;
    for (size_t index = 0u; kGoodEndingPayload[index] != '\0'; ++index) {
        const char value = kGoodEndingPayload[index];
        if (value == '=') break;
        const int decoded = good_ending_b64_value(value);
        if (decoded < 0) continue;
        bits = (bits << 6) | (uint32_t) decoded;
        bit_count += 6;
        if (bit_count >= 8) {
            bit_count -= 8;
            if (written >= capacity) return 0u;
            output[written++] = (uint8_t) ((bits >> bit_count) & 0xFFu);
        }
    }
    return written;
}

static bool good_ending_read_u16(const uint8_t **cursor,
                                 const uint8_t *end, uint16_t *value)
{
    if ((size_t) (end - *cursor) < 2u) return false;
    *value = (uint16_t) (((uint16_t) (*cursor)[0] << 8) | (*cursor)[1]);
    *cursor += 2;
    return true;
}

static bool good_ending_read_u32(const uint8_t **cursor,
                                 const uint8_t *end, uint32_t *value)
{
    if ((size_t) (end - *cursor) < 4u) return false;
    *value = ((uint32_t) (*cursor)[0] << 24) |
             ((uint32_t) (*cursor)[1] << 16) |
             ((uint32_t) (*cursor)[2] << 8) | (uint32_t) (*cursor)[3];
    *cursor += 4;
    return true;
}

static void good_ending_init(void)
{
    if (sGoodEndingReady) return;
    const size_t compressed_size = good_ending_decode_base64(
        sGoodEndingCompressed, sizeof(sGoodEndingCompressed));
    if (compressed_size == 0u) return;

    uLongf unpacked_size = (uLongf) sizeof(sGoodEndingPacked);
    if (uncompress((Bytef *) sGoodEndingPacked, &unpacked_size,
                   (const Bytef *) sGoodEndingCompressed,
                   (uLong) compressed_size) != Z_OK ||
        unpacked_size != sizeof(sGoodEndingPacked)) return;

    const uint8_t *cursor = sGoodEndingPacked;
    const uint8_t *const end = sGoodEndingPacked + sizeof(sGoodEndingPacked);
    if ((size_t) (end - cursor) < 16u || memcmp(cursor, "GED1", 4u) != 0) return;
    cursor += 4;

    uint16_t width, height, palette_count, offset_count;
    uint32_t run_count;
    if (!good_ending_read_u16(&cursor, end, &width) ||
        !good_ending_read_u16(&cursor, end, &height) ||
        !good_ending_read_u16(&cursor, end, &palette_count) ||
        !good_ending_read_u16(&cursor, end, &offset_count) ||
        !good_ending_read_u32(&cursor, end, &run_count)) return;
    if (width != 640u || height != 480u || palette_count != 16u ||
        offset_count != 481u || run_count != 58840u) return;

    for (uint16_t index = 0u; index < 16u; ++index)
        if (!good_ending_read_u32(&cursor, end, &sGoodEndingPalette[index])) return;
    for (uint16_t index = 0u; index < 481u; ++index)
        if (!good_ending_read_u16(&cursor, end, &sGoodEndingOffsets[index])) return;
    if ((size_t) (end - cursor) != 58840u) return;
    memcpy(sGoodEndingRuns, cursor, 58840u);

    sGoodEndingTexture.width = 640u;
    sGoodEndingTexture.height = 480u;
    sGoodEndingTexture.transparent_index = 255u;
    sGoodEndingTexture.row_offsets = sGoodEndingOffsets;
    sGoodEndingTexture.runs = sGoodEndingRuns;
    sGoodEndingTexture.palette = sGoodEndingPalette;
    sGoodEndingReady = true;
}

const TextureRle *good_ending_pc_texture(void)
{
    good_ending_init();
    return sGoodEndingReady ? &sGoodEndingTexture : NULL;
}
