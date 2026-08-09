/*
 * Phantom Puppet / Marionette five-frame encounter animation.
 *
 * Generated from the sprite sheet supplied for the Wii U port.
 * Frame order is:
 *   1) top-left
 *   2) top-right
 *   3) middle-left
 *   4) middle-right
 *   5) bottom-left
 *
 * The small bottom-right sprite in the source sheet is intentionally ignored.
 *
 * Frames are stored as zlib-compressed 224x215 indexed images to keep the
 * repository small. They are expanded once, on first use, into the same
 * row-RLE TextureRle format used by the rest of the port.
 */
#include "assets/puppet_custom_assets.h"

#include <stddef.h>
#include <stdint.h>

#include <zlib.h>

#define PUPPET_SHEET_WIDTH 224u
#define PUPPET_SHEET_HEIGHT 215u
#define PUPPET_SHEET_PIXELS (PUPPET_SHEET_WIDTH * PUPPET_SHEET_HEIGHT)
#define PUPPET_SHEET_FRAME_COUNT 5u
#define PUPPET_SHEET_PALETTE_SIZE 24u
#define PUPPET_SHEET_MAX_COMPRESSED 12000u
#define PUPPET_SHEET_MAX_RUN_BYTES 32768u

#include "assets/puppet_sheet_frame1.inc"
#include "assets/puppet_sheet_frame2.inc"
#include "assets/puppet_sheet_frame3.inc"
#include "assets/puppet_sheet_frame4.inc"
#include "assets/puppet_sheet_frame5.inc"

static TextureRle kPuppetSheetTextures[PUPPET_SHEET_FRAME_COUNT];
static const TextureRle *const kPuppetSheetFramePointers[PUPPET_SHEET_FRAME_COUNT] = {
    &kPuppetSheetTextures[0],
    &kPuppetSheetTextures[1],
    &kPuppetSheetTextures[2],
    &kPuppetSheetTextures[3],
    &kPuppetSheetTextures[4],
};

const JumpscareSequence gPhantomPuppetSheetAnimation = {
    kPuppetSheetFramePointers,
    PUPPET_SHEET_FRAME_COUNT,
    8u,
};

static uint16_t kPuppetSheetRowOffsets[PUPPET_SHEET_FRAME_COUNT]
                                      [PUPPET_SHEET_HEIGHT + 1u];
static uint8_t kPuppetSheetRuns[PUPPET_SHEET_FRAME_COUNT]
                               [PUPPET_SHEET_MAX_RUN_BYTES];
static uint8_t kPuppetSheetCompressed[PUPPET_SHEET_MAX_COMPRESSED];
static uint8_t kPuppetSheetPixels[PUPPET_SHEET_PIXELS];
static bool kPuppetSheetPrepared = false;
static bool kPuppetSheetFailed = false;

static int base64_value(char c)
{
    if (c >= 'A' && c <= 'Z') return (int) (c - 'A');
    if (c >= 'a' && c <= 'z') return 26 + (int) (c - 'a');
    if (c >= '0' && c <= '9') return 52 + (int) (c - '0');
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static bool decode_base64(const char *source,
                          uint8_t *destination,
                          size_t capacity,
                          size_t *written)
{
    size_t out = 0u;
    uint32_t accumulator = 0u;
    unsigned bits = 0u;

    if (source == NULL || destination == NULL || written == NULL) {
        return false;
    }

    for (const char *cursor = source; *cursor != '\0'; ++cursor) {
        if (*cursor == '=') {
            break;
        }

        const int value = base64_value(*cursor);
        if (value < 0) {
            continue;
        }

        accumulator = (accumulator << 6u) | (uint32_t) value;
        bits += 6u;

        if (bits >= 8u) {
            bits -= 8u;
            if (out >= capacity) {
                return false;
            }
            destination[out++] =
                (uint8_t) ((accumulator >> bits) & 0xFFu);
        }
    }

    *written = out;
    return true;
}

static bool encode_rle_frame(unsigned frame,
                             const uint8_t *pixels)
{
    uint16_t *offsets = kPuppetSheetRowOffsets[frame];
    uint8_t *runs = kPuppetSheetRuns[frame];
    size_t encoded = 0u;

    offsets[0] = 0u;

    for (uint32_t y = 0u; y < PUPPET_SHEET_HEIGHT; ++y) {
        const uint8_t *row = pixels + y * PUPPET_SHEET_WIDTH;
        uint32_t x = 0u;

        while (x < PUPPET_SHEET_WIDTH) {
            const uint8_t index = row[x];
            uint32_t length = 1u;

            while (x + length < PUPPET_SHEET_WIDTH &&
                   row[x + length] == index &&
                   length < 255u) {
                ++length;
            }

            if (encoded + 2u > PUPPET_SHEET_MAX_RUN_BYTES) {
                return false;
            }

            runs[encoded++] = (uint8_t) length;
            runs[encoded++] = index;
            x += length;
        }

        if (encoded > 0xFFFFu) {
            return false;
        }
        offsets[y + 1u] = (uint16_t) encoded;
    }

    return true;
}

static bool prepare_frame(unsigned frame,
                          const char *compressed_base64,
                          const uint32_t *palette)
{
    size_t compressed_size = 0u;

    if (!decode_base64(compressed_base64,
                       kPuppetSheetCompressed,
                       sizeof(kPuppetSheetCompressed),
                       &compressed_size)) {
        return false;
    }

    uLongf pixel_size = (uLongf) sizeof(kPuppetSheetPixels);
    const int zlib_result = uncompress(kPuppetSheetPixels,
                                       &pixel_size,
                                       kPuppetSheetCompressed,
                                       (uLong) compressed_size);
    if (zlib_result != Z_OK ||
        pixel_size != (uLongf) sizeof(kPuppetSheetPixels)) {
        return false;
    }

    if (!encode_rle_frame(frame, kPuppetSheetPixels)) {
        return false;
    }

    kPuppetSheetTextures[frame].width = (uint16_t) PUPPET_SHEET_WIDTH;
    kPuppetSheetTextures[frame].height = (uint16_t) PUPPET_SHEET_HEIGHT;
    /*
     * Index 255 is never generated by these 24-colour frames, so the black
     * areas supplied in the sheet remain opaque instead of exposing the
     * office behind the Puppet.
     */
    kPuppetSheetTextures[frame].transparent_index = 255u;
    kPuppetSheetTextures[frame].row_offsets =
        kPuppetSheetRowOffsets[frame];
    kPuppetSheetTextures[frame].runs = kPuppetSheetRuns[frame];
    kPuppetSheetTextures[frame].palette = palette;

    return true;
}

bool phantom_puppet_sheet_prepare(void)
{
    if (kPuppetSheetPrepared) {
        return true;
    }
    if (kPuppetSheetFailed) {
        return false;
    }

    static const char *const compressed_frames[PUPPET_SHEET_FRAME_COUNT] = {
        kPuppetSheetFrame1Base64,
        kPuppetSheetFrame2Base64,
        kPuppetSheetFrame3Base64,
        kPuppetSheetFrame4Base64,
        kPuppetSheetFrame5Base64,
    };
    static const uint32_t *const palettes[PUPPET_SHEET_FRAME_COUNT] = {
        kPuppetSheetPalette1,
        kPuppetSheetPalette2,
        kPuppetSheetPalette3,
        kPuppetSheetPalette4,
        kPuppetSheetPalette5,
    };

    for (unsigned frame = 0u;
         frame < PUPPET_SHEET_FRAME_COUNT;
         ++frame) {
        if (!prepare_frame(frame,
                           compressed_frames[frame],
                           palettes[frame])) {
            kPuppetSheetFailed = true;
            return false;
        }
    }

    kPuppetSheetPrepared = true;
    return true;
}
