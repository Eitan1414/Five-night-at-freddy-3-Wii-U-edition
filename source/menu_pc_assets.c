/* Generated from the supplied original FNaF 3 PC title assets.
 * The title logo and five Springtrap menu frames are indexed, RLE encoded,
 * then zlib-compressed into eight small data/menu_pc_title_*.bin chunks.
 */
#include "assets/menu_pc_assets.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

extern const uint8_t menu_pc_title_0_bin[];
extern const uint8_t menu_pc_title_0_bin_end[];
extern const uint8_t menu_pc_title_1_bin[];
extern const uint8_t menu_pc_title_1_bin_end[];
extern const uint8_t menu_pc_title_2_bin[];
extern const uint8_t menu_pc_title_2_bin_end[];
extern const uint8_t menu_pc_title_3_bin[];
extern const uint8_t menu_pc_title_3_bin_end[];
extern const uint8_t menu_pc_title_4_bin[];
extern const uint8_t menu_pc_title_4_bin_end[];
extern const uint8_t menu_pc_title_5_bin[];
extern const uint8_t menu_pc_title_5_bin_end[];
extern const uint8_t menu_pc_title_6_bin[];
extern const uint8_t menu_pc_title_6_bin_end[];
extern const uint8_t menu_pc_title_7_bin[];
extern const uint8_t menu_pc_title_7_bin_end[];

TextureRle gPcTitleLogo;
TextureRle gPcTitleSpringtrap1;
TextureRle gPcTitleSpringtrap2;
TextureRle gPcTitleSpringtrap3;
TextureRle gPcTitleSpringtrap4;
TextureRle gPcTitleSpringtrap5;

static bool sMenuPcAttempted;
static bool sMenuPcInitialized;
static uint8_t sMenuPcPacked[158760];
static uint8_t sMenuPcCompressed[68411];
static uint32_t sMenuPcPalettePool[176];
static uint16_t sMenuPcOffsetPool[1352];
static uint8_t sMenuPcRunsPool[155274];

static bool menu_pc_read_u16(const uint8_t **cursor,
                             const uint8_t *end,
                             uint16_t *value)
{
    if ((size_t) (end - *cursor) < 2u) return false;
    *value = (uint16_t) (((uint16_t) (*cursor)[0] << 8) |
                         (uint16_t) (*cursor)[1]);
    *cursor += 2;
    return true;
}

static bool menu_pc_read_u32(const uint8_t **cursor,
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

void menu_pc_assets_init(void)
{
    if (sMenuPcInitialized || sMenuPcAttempted) return;
    sMenuPcAttempted = true;

    static const uint8_t *const chunk_starts[8] = {
        menu_pc_title_0_bin, menu_pc_title_1_bin,
        menu_pc_title_2_bin, menu_pc_title_3_bin,
        menu_pc_title_4_bin, menu_pc_title_5_bin,
        menu_pc_title_6_bin, menu_pc_title_7_bin
    };
    static const uint8_t *const chunk_ends[8] = {
        menu_pc_title_0_bin_end, menu_pc_title_1_bin_end,
        menu_pc_title_2_bin_end, menu_pc_title_3_bin_end,
        menu_pc_title_4_bin_end, menu_pc_title_5_bin_end,
        menu_pc_title_6_bin_end, menu_pc_title_7_bin_end
    };

    size_t compressed_size = 0u;
    for (int index = 0; index < 8; ++index) {
        const size_t chunk_size =
            (size_t) (chunk_ends[index] - chunk_starts[index]);
        if (compressed_size + chunk_size > sizeof(sMenuPcCompressed))
            return;
        memcpy(sMenuPcCompressed + compressed_size,
               chunk_starts[index], chunk_size);
        compressed_size += chunk_size;
    }
    if (compressed_size != sizeof(sMenuPcCompressed)) return;

    uLongf unpacked_size = (uLongf) sizeof(sMenuPcPacked);
    if (uncompress((Bytef *) sMenuPcPacked,
                   &unpacked_size,
                   (const Bytef *) sMenuPcCompressed,
                   (uLong) compressed_size) != Z_OK ||
        unpacked_size != sizeof(sMenuPcPacked)) {
        return;
    }

    const uint8_t *cursor = sMenuPcPacked;
    const uint8_t *const end = sMenuPcPacked + sizeof(sMenuPcPacked);
    if ((size_t) (end - cursor) < 6u ||
        memcmp(cursor, "MTP1", 4u) != 0) {
        return;
    }
    cursor += 4;

    uint16_t sprite_count = 0u;
    if (!menu_pc_read_u16(&cursor, end, &sprite_count) ||
        sprite_count != 6u) {
        return;
    }

    TextureRle *const targets[6] = {
        &gPcTitleLogo,
        &gPcTitleSpringtrap1,
        &gPcTitleSpringtrap2,
        &gPcTitleSpringtrap3,
        &gPcTitleSpringtrap4,
        &gPcTitleSpringtrap5
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

        if (!menu_pc_read_u16(&cursor, end, &width) ||
            !menu_pc_read_u16(&cursor, end, &height) ||
            !menu_pc_read_u16(&cursor, end, &palette_count) ||
            !menu_pc_read_u16(&cursor, end, &offset_count) ||
            !menu_pc_read_u32(&cursor, end, &runs_count)) {
            return;
        }

        if (offset_count != (uint16_t) (height + 1u) ||
            palette_cursor + palette_count >
                sizeof(sMenuPcPalettePool) / sizeof(sMenuPcPalettePool[0]) ||
            offset_cursor + offset_count >
                sizeof(sMenuPcOffsetPool) / sizeof(sMenuPcOffsetPool[0]) ||
            runs_cursor + runs_count > sizeof(sMenuPcRunsPool)) {
            return;
        }

        TextureRle *const texture = targets[index];
        texture->width = width;
        texture->height = height;
        texture->transparent_index = 255u;
        texture->palette = &sMenuPcPalettePool[palette_cursor];
        texture->row_offsets = &sMenuPcOffsetPool[offset_cursor];
        texture->runs = &sMenuPcRunsPool[runs_cursor];

        for (uint16_t colour = 0u; colour < palette_count; ++colour) {
            uint32_t rgba = 0u;
            if (!menu_pc_read_u32(&cursor, end, &rgba)) return;
            sMenuPcPalettePool[palette_cursor++] = rgba;
        }

        for (uint16_t row = 0u; row < offset_count; ++row) {
            uint16_t offset = 0u;
            if (!menu_pc_read_u16(&cursor, end, &offset)) return;
            sMenuPcOffsetPool[offset_cursor++] = offset;
        }

        if ((size_t) (end - cursor) < runs_count) return;
        memcpy(&sMenuPcRunsPool[runs_cursor], cursor, runs_count);
        runs_cursor += runs_count;
        cursor += runs_count;
    }

    if (cursor != end) return;
    sMenuPcInitialized = true;
}
