/* Generated from the supplied original FNaF 3 PC minigame sheets.
 * The indexed RLE payload is zlib-compressed to keep the source and Wii U
 * executable compact. Pure/near black and sheet-cyan are transparent.
 */
#include "assets/minigame_pc_extended_assets.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

TextureRle gMiniMangleLeft;
TextureRle gMiniMangleRight;
TextureRle gMiniManglePart1;
TextureRle gMiniManglePart2;
TextureRle gMiniManglePart3;
TextureRle gMiniManglePart4;
TextureRle gMiniChildSad;
TextureRle gMiniChildHappy;
TextureRle gMiniChicaLeft;
TextureRle gMiniChicaRight;
TextureRle gMiniCupcakeSmall;
TextureRle gMiniCupcakeLarge;
TextureRle gMiniStageRight;
TextureRle gMiniStageLeft;
TextureRle gMiniStageFront;
TextureRle gMiniStageChild;
TextureRle gMiniShadowLeft;
TextureRle gMiniShadowRight;
TextureRle gMiniShadowChildSad;
TextureRle gMiniShadowChildHappy;
TextureRle gMiniPuppetLeft;
TextureRle gMiniPuppetRight;
TextureRle gMiniPartyChild1;
TextureRle gMiniPartyChild2;
TextureRle gMiniPartyChild3;
TextureRle gMiniPartyChild4;
TextureRle gMiniPartyChild5;
TextureRle gMiniPartyCake;
TextureRle gMiniExitDoor;

static bool sMiniPcExtendedInitialized;
static uint8_t sMiniPcPacked[29860];
static uint8_t sMiniPcCompressed[13306];
static uint32_t sMiniPcPalettePool[1377];
static uint16_t sMiniPcOffsetPool[1114];
static uint8_t sMiniPcRunsPool[21770];

static const char kMiniPcExtendedPayload[] =
#include "minigame_pc_extended_payload_01.inc"
#include "minigame_pc_extended_payload_02.inc"
#include "minigame_pc_extended_payload_03.inc"
#include "minigame_pc_extended_payload_04.inc"
;

static int mini_pc_b64_value(char value)
{
    if (value >= 'A' && value <= 'Z') return value - 'A';
    if (value >= 'a' && value <= 'z') return value - 'a' + 26;
    if (value >= '0' && value <= '9') return value - '0' + 52;
    if (value == '+') return 62;
    if (value == '/') return 63;
    return -1;
}

static size_t mini_pc_decode_base64(uint8_t *output, size_t capacity)
{
    size_t written = 0u;
    uint32_t bits = 0u;
    int bit_count = 0;
    for (size_t index = 0u;
         kMiniPcExtendedPayload[index] != '\0';
         ++index) {
        const char value = kMiniPcExtendedPayload[index];
        if (value == '=') break;
        const int decoded = mini_pc_b64_value(value);
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

static bool mini_pc_read_u16(const uint8_t **cursor,
                             const uint8_t *end,
                             uint16_t *value)
{
    if ((size_t) (end - *cursor) < 2u) return false;
    *value = (uint16_t) (((uint16_t) (*cursor)[0] << 8) |
                         (uint16_t) (*cursor)[1]);
    *cursor += 2;
    return true;
}

static bool mini_pc_read_u32(const uint8_t **cursor,
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

void minigame_pc_extended_assets_init(void)
{
    if (sMiniPcExtendedInitialized) return;

    const size_t compressed_size =
        mini_pc_decode_base64(sMiniPcCompressed,
                              sizeof(sMiniPcCompressed));
    if (compressed_size == 0u) return;

    uLongf unpacked_size = (uLongf) sizeof(sMiniPcPacked);
    if (uncompress((Bytef *) sMiniPcPacked,
                   &unpacked_size,
                   (const Bytef *) sMiniPcCompressed,
                   (uLong) compressed_size) != Z_OK ||
        unpacked_size != sizeof(sMiniPcPacked)) {
        return;
    }

    const uint8_t *cursor = sMiniPcPacked;
    const uint8_t *const end = sMiniPcPacked + sizeof(sMiniPcPacked);
    if ((size_t) (end - cursor) < 6u ||
        memcmp(cursor, "MGS1", 4u) != 0) {
        return;
    }
    cursor += 4;

    uint16_t sprite_count = 0u;
    if (!mini_pc_read_u16(&cursor, end, &sprite_count) ||
        sprite_count != 29u) {
        return;
    }

    TextureRle *const targets[29] = {
        &gMiniMangleLeft,
        &gMiniMangleRight,
        &gMiniManglePart1,
        &gMiniManglePart2,
        &gMiniManglePart3,
        &gMiniManglePart4,
        &gMiniChildSad,
        &gMiniChildHappy,
        &gMiniChicaLeft,
        &gMiniChicaRight,
        &gMiniCupcakeSmall,
        &gMiniCupcakeLarge,
        &gMiniStageRight,
        &gMiniStageLeft,
        &gMiniStageFront,
        &gMiniStageChild,
        &gMiniShadowLeft,
        &gMiniShadowRight,
        &gMiniShadowChildSad,
        &gMiniShadowChildHappy,
        &gMiniPuppetLeft,
        &gMiniPuppetRight,
        &gMiniPartyChild1,
        &gMiniPartyChild2,
        &gMiniPartyChild3,
        &gMiniPartyChild4,
        &gMiniPartyChild5,
        &gMiniPartyCake,
        &gMiniExitDoor
    };

    size_t palette_cursor = 0u;
    size_t offset_cursor = 0u;
    size_t runs_cursor = 0u;

    for (uint16_t index = 0u; index < sprite_count; ++index) {
        uint16_t width = 0u;
        uint16_t height = 0u;
        uint16_t palette_count = 0u;
        uint16_t offset_count = 0u;
        uint32_t runs_count = 0u;

        if (!mini_pc_read_u16(&cursor, end, &width) ||
            !mini_pc_read_u16(&cursor, end, &height) ||
            !mini_pc_read_u16(&cursor, end, &palette_count) ||
            !mini_pc_read_u16(&cursor, end, &offset_count) ||
            !mini_pc_read_u32(&cursor, end, &runs_count)) {
            return;
        }

        if (offset_count != (uint16_t) (height + 1u) ||
            palette_cursor + palette_count >
                sizeof(sMiniPcPalettePool) /
                    sizeof(sMiniPcPalettePool[0]) ||
            offset_cursor + offset_count >
                sizeof(sMiniPcOffsetPool) /
                    sizeof(sMiniPcOffsetPool[0]) ||
            runs_cursor + runs_count > sizeof(sMiniPcRunsPool)) {
            return;
        }

        TextureRle *const texture = targets[index];
        texture->width = width;
        texture->height = height;
        texture->transparent_index = 255u;
        texture->palette = &sMiniPcPalettePool[palette_cursor];
        texture->row_offsets = &sMiniPcOffsetPool[offset_cursor];
        texture->runs = &sMiniPcRunsPool[runs_cursor];

        for (uint16_t colour = 0u; colour < palette_count; ++colour) {
            uint32_t rgba = 0u;
            if (!mini_pc_read_u32(&cursor, end, &rgba)) return;
            sMiniPcPalettePool[palette_cursor++] = rgba;
        }
        for (uint16_t row = 0u; row < offset_count; ++row) {
            uint16_t offset = 0u;
            if (!mini_pc_read_u16(&cursor, end, &offset)) return;
            sMiniPcOffsetPool[offset_cursor++] = offset;
        }
        if ((size_t) (end - cursor) < runs_count) return;
        memcpy(&sMiniPcRunsPool[runs_cursor], cursor, runs_count);
        runs_cursor += runs_count;
        cursor += runs_count;
    }

    if (cursor != end) return;
    sMiniPcExtendedInitialized = true;
}
