#include "assets/follow_me_pc_env_assets.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

static bool sFollowMeEnvInitialized;
static TextureRle sFollowMeEnvTextures[FOLLOW_ME_ENV_SPRITE_COUNT];
static uint8_t sFollowMeEnvPacked[64402];
static uint8_t sFollowMeEnvCompressed[21546];
static uint32_t sFollowMeEnvPalettePool[253];
static uint16_t sFollowMeEnvOffsetPool[1149];
static uint8_t sFollowMeEnvRunsPool[60894];

static const char kFollowMeEnvPayload[] =
#include "follow_me_pc_env_payload_01.inc"
#include "follow_me_pc_env_payload_02.inc"
#include "follow_me_pc_env_payload_03.inc"
#include "follow_me_pc_env_payload_04.inc"
#include "follow_me_pc_env_payload_05.inc"
;

static int follow_me_env_b64_value(char value)
{
    if (value >= 'A' && value <= 'Z') return value - 'A';
    if (value >= 'a' && value <= 'z') return value - 'a' + 26;
    if (value >= '0' && value <= '9') return value - '0' + 52;
    if (value == '+') return 62;
    if (value == '/') return 63;
    return -1;
}

static size_t follow_me_env_decode_base64(uint8_t *output, size_t capacity)
{
    size_t written = 0u;
    uint32_t bits = 0u;
    int bit_count = 0;
    for (size_t index = 0u; kFollowMeEnvPayload[index] != '\0'; ++index) {
        const char value = kFollowMeEnvPayload[index];
        if (value == '=') break;
        const int decoded = follow_me_env_b64_value(value);
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

static bool follow_me_env_read_u16(const uint8_t **cursor,
                                   const uint8_t *end,
                                   uint16_t *value)
{
    if ((size_t) (end - *cursor) < 2u) return false;
    *value = (uint16_t) (((uint16_t) (*cursor)[0] << 8) | (*cursor)[1]);
    *cursor += 2;
    return true;
}

static bool follow_me_env_read_u32(const uint8_t **cursor,
                                   const uint8_t *end,
                                   uint32_t *value)
{
    if ((size_t) (end - *cursor) < 4u) return false;
    *value = ((uint32_t) (*cursor)[0] << 24) |
             ((uint32_t) (*cursor)[1] << 16) |
             ((uint32_t) (*cursor)[2] << 8) |
             (uint32_t) (*cursor)[3];
    *cursor += 4;
    return true;
}

static void follow_me_pc_env_assets_init(void)
{
    if (sFollowMeEnvInitialized) return;
    const size_t compressed_size = follow_me_env_decode_base64(
        sFollowMeEnvCompressed, sizeof(sFollowMeEnvCompressed));
    if (compressed_size == 0u) return;

    uLongf unpacked_size = (uLongf) sizeof(sFollowMeEnvPacked);
    if (uncompress((Bytef *) sFollowMeEnvPacked, &unpacked_size,
                   (const Bytef *) sFollowMeEnvCompressed,
                   (uLong) compressed_size) != Z_OK ||
        unpacked_size != sizeof(sFollowMeEnvPacked)) return;

    const uint8_t *cursor = sFollowMeEnvPacked;
    const uint8_t *const end = sFollowMeEnvPacked + sizeof(sFollowMeEnvPacked);
    if ((size_t) (end - cursor) < 6u || memcmp(cursor, "FME1", 4u) != 0) return;
    cursor += 4;

    uint16_t count = 0u;
    if (!follow_me_env_read_u16(&cursor, end, &count) ||
        count != FOLLOW_ME_ENV_SPRITE_COUNT) return;

    size_t pal_cursor = 0u;
    size_t off_cursor = 0u;
    size_t run_cursor = 0u;
    for (uint16_t index = 0u; index < count; ++index) {
        uint16_t width, height, palette_count, offset_count;
        uint32_t runs_count;
        if (!follow_me_env_read_u16(&cursor, end, &width) ||
            !follow_me_env_read_u16(&cursor, end, &height) ||
            !follow_me_env_read_u16(&cursor, end, &palette_count) ||
            !follow_me_env_read_u16(&cursor, end, &offset_count) ||
            !follow_me_env_read_u32(&cursor, end, &runs_count)) return;

        if (offset_count != (uint16_t) (height + 1u) ||
            pal_cursor + palette_count > 253u ||
            off_cursor + offset_count > 1149u ||
            run_cursor + runs_count > 60894u) return;

        TextureRle *texture = &sFollowMeEnvTextures[index];
        texture->width = width;
        texture->height = height;
        texture->transparent_index = 255u;
        texture->palette = &sFollowMeEnvPalettePool[pal_cursor];
        texture->row_offsets = &sFollowMeEnvOffsetPool[off_cursor];
        texture->runs = &sFollowMeEnvRunsPool[run_cursor];

        for (uint16_t p = 0u; p < palette_count; ++p) {
            uint32_t value;
            if (!follow_me_env_read_u32(&cursor, end, &value)) return;
            sFollowMeEnvPalettePool[pal_cursor++] = value;
        }
        for (uint16_t row = 0u; row < offset_count; ++row) {
            uint16_t value;
            if (!follow_me_env_read_u16(&cursor, end, &value)) return;
            sFollowMeEnvOffsetPool[off_cursor++] = value;
        }
        if ((size_t) (end - cursor) < runs_count) return;
        memcpy(&sFollowMeEnvRunsPool[run_cursor], cursor, runs_count);
        run_cursor += runs_count;
        cursor += runs_count;
    }

    if (cursor != end) return;
    sFollowMeEnvInitialized = true;
}

const TextureRle *follow_me_pc_env_sprite(FollowMePcEnvSprite sprite)
{
    follow_me_pc_env_assets_init();
    if (!sFollowMeEnvInitialized || (int) sprite < 0 ||
        sprite >= FOLLOW_ME_ENV_SPRITE_COUNT) return NULL;
    return &sFollowMeEnvTextures[(int) sprite];
}
