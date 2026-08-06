/* Generated from the supplied original FNaF 3 PC minigame sprite dump.
 * Source images are reduced 4x with nearest-neighbour sampling. Pure black is
 * treated as transparent, matching the original Clickteam sprite colorkey.
 */
#include "assets/minigame_pc_assets.h"

#include <stdint.h>

static const uint32_t kMiniBbLeftPalette[256] = {
    [0] = 0xFE0000FFu,
    [1] = 0x97479BFFu,
    [2] = 0xFFFFFEFFu,
    [3] = 0xE70000FFu,
    [4] = 0xFFFFFFFFu,
    [5] = 0x335FB3FFu,
    [6] = 0xFFBF7FFFu,
    [7] = 0x434343FFu,
    [8] = 0x2B5397FFu,
    [9] = 0xFF2323FFu,
    [10] = 0x0F0F0FFFu,
    [11] = 0x3B6BCBFFu,
    [12] = 0xA7573BFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBbLeftRowOffsets[31] = {
    0u, 6u, 16u, 26u, 48u, 70u, 90u, 112u, 124u, 142u, 156u, 172u,
    182u, 196u, 210u, 224u, 238u, 252u, 266u, 276u, 290u, 304u, 314u, 320u,
    326u, 332u, 338u, 344u, 354u, 364u, 374u,
};
static const uint8_t kMiniBbLeftRuns[374] = {
    11u, 255u, 6u, 0u, 11u, 255u, 2u, 255u, 7u, 1u, 5u, 255u, 1u, 0u, 13u, 255u, 2u, 255u, 7u, 1u,
    5u, 255u, 1u, 0u, 13u, 255u, 1u, 255u, 2u, 1u, 3u, 2u, 5u, 1u, 1u, 3u, 1u, 4u, 1u, 3u,
    1u, 4u, 1u, 3u, 1u, 4u, 11u, 255u, 1u, 255u, 2u, 1u, 3u, 2u, 5u, 1u, 1u, 3u, 1u, 4u,
    1u, 3u, 1u, 4u, 1u, 3u, 1u, 4u, 11u, 255u, 1u, 255u, 2u, 1u, 3u, 2u, 5u, 1u, 2u, 4u,
    1u, 3u, 1u, 4u, 2u, 3u, 1u, 4u, 10u, 255u, 1u, 255u, 2u, 1u, 3u, 2u, 5u, 1u, 1u, 4u,
    2u, 3u, 1u, 4u, 2u, 3u, 1u, 4u, 1u, 3u, 9u, 255u, 1u, 255u, 10u, 1u, 3u, 5u, 1u, 6u,
    4u, 5u, 9u, 255u, 1u, 255u, 10u, 1u, 2u, 2u, 1u, 5u, 1u, 6u, 1u, 5u, 1u, 2u, 2u, 5u,
    9u, 255u, 2u, 255u, 7u, 1u, 2u, 6u, 3u, 5u, 1u, 6u, 4u, 5u, 9u, 255u, 3u, 255u, 5u, 1u,
    1u, 255u, 2u, 6u, 3u, 5u, 1u, 6u, 4u, 5u, 9u, 255u, 3u, 255u, 5u, 1u, 1u, 255u, 10u, 6u,
    9u, 255u, 5u, 255u, 1u, 7u, 3u, 255u, 2u, 6u, 6u, 8u, 2u, 6u, 9u, 255u, 5u, 255u, 1u, 7u,
    4u, 255u, 1u, 6u, 6u, 8u, 1u, 6u, 10u, 255u, 5u, 255u, 1u, 7u, 3u, 255u, 2u, 9u, 6u, 6u,
    2u, 9u, 9u, 255u, 5u, 255u, 1u, 7u, 1u, 255u, 7u, 9u, 2u, 10u, 4u, 9u, 8u, 255u, 3u, 255u,
    5u, 6u, 6u, 9u, 2u, 10u, 4u, 9u, 2u, 6u, 6u, 255u, 3u, 255u, 5u, 6u, 6u, 9u, 2u, 10u,
    4u, 9u, 2u, 6u, 6u, 255u, 3u, 255u, 5u, 6u, 12u, 9u, 2u, 6u, 6u, 255u, 3u, 255u, 5u, 6u,
    6u, 9u, 2u, 10u, 4u, 9u, 2u, 6u, 6u, 255u, 3u, 255u, 5u, 6u, 6u, 9u, 2u, 10u, 4u, 9u,
    2u, 6u, 6u, 255u, 7u, 255u, 7u, 9u, 2u, 10u, 4u, 9u, 8u, 255u, 7u, 255u, 13u, 9u, 8u, 255u,
    9u, 255u, 10u, 9u, 9u, 255u, 9u, 255u, 10u, 11u, 9u, 255u, 9u, 255u, 10u, 11u, 9u, 255u, 9u, 255u,
    10u, 11u, 9u, 255u, 9u, 255u, 3u, 11u, 4u, 255u, 3u, 11u, 9u, 255u, 9u, 255u, 5u, 12u, 2u, 255u,
    5u, 12u, 7u, 255u, 9u, 255u, 5u, 12u, 2u, 255u, 5u, 12u, 7u, 255u,
};
const TextureRle gMiniBbLeft = {
    28u, 30u, 255u,
    kMiniBbLeftRowOffsets,
    kMiniBbLeftRuns,
    kMiniBbLeftPalette,
};

static const uint32_t kMiniBbRightPalette[256] = {
    [0] = 0xFE0000FFu,
    [1] = 0x97479BFFu,
    [2] = 0xFFFFFFFFu,
    [3] = 0xE70000FFu,
    [4] = 0xFFFFFEFFu,
    [5] = 0x335FB3FFu,
    [6] = 0xFFBF7FFFu,
    [7] = 0x2B5397FFu,
    [8] = 0x434343FFu,
    [9] = 0xFF2323FFu,
    [10] = 0x0F0F0FFFu,
    [11] = 0x3B6BCBFFu,
    [12] = 0xA7573BFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBbRightRowOffsets[31] = {
    0u, 6u, 16u, 26u, 44u, 62u, 82u, 104u, 116u, 134u, 148u, 162u,
    170u, 184u, 196u, 210u, 224u, 238u, 252u, 262u, 276u, 290u, 300u, 306u,
    312u, 318u, 324u, 330u, 340u, 350u, 360u,
};
static const uint8_t kMiniBbRightRuns[360] = {
    11u, 255u, 5u, 0u, 12u, 255u, 13u, 255u, 1u, 0u, 4u, 255u, 8u, 1u, 2u, 255u, 13u, 255u, 1u, 0u,
    4u, 255u, 8u, 1u, 2u, 255u, 10u, 255u, 4u, 2u, 1u, 3u, 1u, 2u, 1u, 3u, 5u, 1u, 3u, 4u,
    2u, 1u, 1u, 255u, 10u, 255u, 4u, 2u, 1u, 3u, 1u, 2u, 1u, 3u, 5u, 1u, 3u, 4u, 2u, 1u,
    1u, 255u, 10u, 255u, 1u, 2u, 1u, 3u, 2u, 2u, 1u, 3u, 2u, 2u, 5u, 1u, 3u, 4u, 2u, 1u,
    1u, 255u, 9u, 255u, 1u, 3u, 1u, 2u, 1u, 3u, 2u, 2u, 1u, 3u, 2u, 2u, 5u, 1u, 3u, 4u,
    2u, 1u, 1u, 255u, 9u, 255u, 3u, 5u, 1u, 6u, 4u, 5u, 10u, 1u, 1u, 255u, 9u, 255u, 2u, 5u,
    1u, 4u, 1u, 6u, 2u, 5u, 1u, 4u, 1u, 5u, 10u, 1u, 1u, 255u, 9u, 255u, 3u, 5u, 1u, 6u,
    4u, 5u, 1u, 6u, 8u, 1u, 2u, 255u, 9u, 255u, 3u, 5u, 1u, 6u, 4u, 5u, 2u, 6u, 6u, 1u,
    3u, 255u, 9u, 255u, 10u, 6u, 6u, 1u, 3u, 255u, 9u, 255u, 1u, 6u, 6u, 7u, 3u, 6u, 2u, 255u,
    2u, 8u, 5u, 255u, 10u, 255u, 6u, 7u, 2u, 6u, 3u, 255u, 2u, 8u, 5u, 255u, 9u, 255u, 2u, 9u,
    5u, 6u, 3u, 9u, 2u, 255u, 2u, 8u, 5u, 255u, 7u, 255u, 5u, 9u, 2u, 10u, 6u, 9u, 1u, 255u,
    2u, 8u, 5u, 255u, 5u, 255u, 2u, 6u, 5u, 9u, 2u, 10u, 6u, 9u, 4u, 6u, 4u, 255u, 5u, 255u,
    2u, 6u, 5u, 9u, 2u, 10u, 6u, 9u, 4u, 6u, 4u, 255u, 5u, 255u, 2u, 6u, 13u, 9u, 4u, 6u,
    4u, 255u, 5u, 255u, 2u, 6u, 4u, 9u, 3u, 10u, 6u, 9u, 4u, 6u, 4u, 255u, 5u, 255u, 2u, 6u,
    4u, 9u, 3u, 10u, 6u, 9u, 4u, 6u, 4u, 255u, 7u, 255u, 4u, 9u, 3u, 10u, 6u, 9u, 8u, 255u,
    7u, 255u, 13u, 9u, 8u, 255u, 9u, 255u, 10u, 9u, 9u, 255u, 9u, 255u, 10u, 11u, 9u, 255u, 9u, 255u,
    10u, 11u, 9u, 255u, 9u, 255u, 10u, 11u, 9u, 255u, 9u, 255u, 3u, 11u, 4u, 255u, 3u, 11u, 9u, 255u,
    7u, 255u, 5u, 12u, 2u, 255u, 5u, 12u, 9u, 255u, 7u, 255u, 5u, 12u, 2u, 255u, 5u, 12u, 9u, 255u,
};
const TextureRle gMiniBbRight = {
    28u, 30u, 255u,
    kMiniBbRightRowOffsets,
    kMiniBbRightRuns,
    kMiniBbRightPalette,
};

static const uint32_t kMiniBalloonSmallRedPalette[256] = {
    [0] = 0xFF2323FFu,
    [1] = 0xFF3C3CFFu,
    [2] = 0xFF2828FFu,
    [3] = 0xFFFFFEFFu,
    [4] = 0xFF4D4CFFu,
    [5] = 0xFF4746FFu,
    [6] = 0xFF2A2AFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBalloonSmallRedRowOffsets[16] = {
    0u, 2u, 8u, 20u, 30u, 38u, 46u, 48u, 54u, 60u, 66u, 72u,
    78u, 84u, 90u, 96u,
};
static const uint8_t kMiniBalloonSmallRedRuns[96] = {
    9u, 255u, 2u, 255u, 5u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 2u, 1u, 1u, 2u, 3u, 0u, 1u, 255u,
    2u, 0u, 2u, 3u, 1u, 4u, 3u, 0u, 1u, 255u, 2u, 0u, 2u, 3u, 1u, 4u, 4u, 0u, 2u, 0u,
    2u, 5u, 1u, 6u, 4u, 0u, 9u, 0u, 1u, 255u, 7u, 0u, 1u, 255u, 2u, 255u, 5u, 0u, 2u, 255u,
    2u, 255u, 5u, 0u, 2u, 255u, 3u, 255u, 3u, 0u, 3u, 255u, 4u, 255u, 1u, 3u, 4u, 255u, 4u, 255u,
    1u, 3u, 4u, 255u, 4u, 255u, 1u, 3u, 4u, 255u, 4u, 255u, 1u, 3u, 4u, 255u,
};
const TextureRle gMiniBalloonSmallRed = {
    9u, 15u, 255u,
    kMiniBalloonSmallRedRowOffsets,
    kMiniBalloonSmallRedRuns,
    kMiniBalloonSmallRedPalette,
};

static const uint32_t kMiniBalloonRedPalette[256] = {
    [0] = 0xFF2323FFu,
    [1] = 0xFFFFFEFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBalloonRedRowOffsets[29] = {
    0u, 2u, 4u, 10u, 16u, 22u, 32u, 40u, 46u, 52u, 58u, 60u,
    62u, 64u, 66u, 72u, 78u, 84u, 90u, 96u, 102u, 108u, 114u, 120u,
    126u, 132u, 138u, 144u, 150u,
};
static const uint8_t kMiniBalloonRedRuns[150] = {
    17u, 255u, 17u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 3u, 255u, 11u, 0u, 3u, 255u, 3u, 255u, 11u, 0u,
    3u, 255u, 2u, 255u, 1u, 0u, 5u, 1u, 7u, 0u, 2u, 255u, 3u, 0u, 5u, 1u, 7u, 0u, 2u, 255u,
    3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 17u, 0u,
    17u, 0u, 17u, 0u, 17u, 0u, 2u, 255u, 13u, 0u, 2u, 255u, 2u, 255u, 13u, 0u, 2u, 255u, 3u, 255u,
    11u, 0u, 3u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 6u, 255u, 5u, 0u,
    6u, 255u, 6u, 255u, 5u, 0u, 6u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
    8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u,
    1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
};
const TextureRle gMiniBalloonRed = {
    17u, 28u, 255u,
    kMiniBalloonRedRowOffsets,
    kMiniBalloonRedRuns,
    kMiniBalloonRedPalette,
};

static const uint32_t kMiniBalloonPurplePalette[256] = {
    [0] = 0x6F1F73FFu,
    [1] = 0xFFFFFEFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBalloonPurpleRowOffsets[29] = {
    0u, 2u, 4u, 10u, 16u, 22u, 32u, 40u, 46u, 52u, 58u, 60u,
    62u, 64u, 66u, 72u, 78u, 84u, 90u, 96u, 102u, 108u, 114u, 120u,
    126u, 132u, 138u, 144u, 150u,
};
static const uint8_t kMiniBalloonPurpleRuns[150] = {
    17u, 255u, 17u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 3u, 255u, 11u, 0u, 3u, 255u, 3u, 255u, 11u, 0u,
    3u, 255u, 2u, 255u, 1u, 0u, 5u, 1u, 7u, 0u, 2u, 255u, 3u, 0u, 5u, 1u, 7u, 0u, 2u, 255u,
    3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 17u, 0u,
    17u, 0u, 17u, 0u, 17u, 0u, 2u, 255u, 13u, 0u, 2u, 255u, 2u, 255u, 13u, 0u, 2u, 255u, 3u, 255u,
    11u, 0u, 3u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 6u, 255u, 5u, 0u,
    6u, 255u, 6u, 255u, 5u, 0u, 6u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
    8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u,
    1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
};
const TextureRle gMiniBalloonPurple = {
    17u, 28u, 255u,
    kMiniBalloonPurpleRowOffsets,
    kMiniBalloonPurpleRuns,
    kMiniBalloonPurplePalette,
};

static const uint32_t kMiniBalloonGreenPalette[256] = {
    [0] = 0x23EB1FFFu,
    [1] = 0xFFFFFEFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBalloonGreenRowOffsets[29] = {
    0u, 2u, 4u, 10u, 16u, 22u, 32u, 40u, 46u, 52u, 58u, 60u,
    62u, 64u, 66u, 72u, 78u, 84u, 90u, 96u, 102u, 108u, 114u, 120u,
    126u, 132u, 138u, 144u, 150u,
};
static const uint8_t kMiniBalloonGreenRuns[150] = {
    17u, 255u, 17u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 3u, 255u, 11u, 0u, 3u, 255u, 3u, 255u, 11u, 0u,
    3u, 255u, 2u, 255u, 1u, 0u, 5u, 1u, 7u, 0u, 2u, 255u, 3u, 0u, 5u, 1u, 7u, 0u, 2u, 255u,
    3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 17u, 0u,
    17u, 0u, 17u, 0u, 17u, 0u, 2u, 255u, 13u, 0u, 2u, 255u, 2u, 255u, 13u, 0u, 2u, 255u, 3u, 255u,
    11u, 0u, 3u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 6u, 255u, 5u, 0u,
    6u, 255u, 6u, 255u, 5u, 0u, 6u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
    8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u,
    1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
};
const TextureRle gMiniBalloonGreen = {
    17u, 28u, 255u,
    kMiniBalloonGreenRowOffsets,
    kMiniBalloonGreenRuns,
    kMiniBalloonGreenPalette,
};

static const uint32_t kMiniBalloonOrangePalette[256] = {
    [0] = 0xF37700FFu,
    [1] = 0xFFFFFEFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBalloonOrangeRowOffsets[29] = {
    0u, 2u, 4u, 10u, 16u, 22u, 32u, 40u, 46u, 52u, 58u, 60u,
    62u, 64u, 66u, 72u, 78u, 84u, 90u, 96u, 102u, 108u, 114u, 120u,
    126u, 132u, 138u, 144u, 150u,
};
static const uint8_t kMiniBalloonOrangeRuns[150] = {
    17u, 255u, 17u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 3u, 255u, 11u, 0u, 3u, 255u, 3u, 255u, 11u, 0u,
    3u, 255u, 2u, 255u, 1u, 0u, 5u, 1u, 7u, 0u, 2u, 255u, 3u, 0u, 5u, 1u, 7u, 0u, 2u, 255u,
    3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 3u, 0u, 5u, 1u, 9u, 0u, 17u, 0u,
    17u, 0u, 17u, 0u, 17u, 0u, 2u, 255u, 13u, 0u, 2u, 255u, 2u, 255u, 13u, 0u, 2u, 255u, 3u, 255u,
    11u, 0u, 3u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 4u, 255u, 8u, 0u, 5u, 255u, 6u, 255u, 5u, 0u,
    6u, 255u, 6u, 255u, 5u, 0u, 6u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
    8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u, 8u, 255u,
    1u, 1u, 8u, 255u, 8u, 255u, 1u, 1u, 8u, 255u,
};
const TextureRle gMiniBalloonOrange = {
    17u, 28u, 255u,
    kMiniBalloonOrangeRowOffsets,
    kMiniBalloonOrangeRuns,
    kMiniBalloonOrangePalette,
};

static const uint32_t kMiniBalloonLargeRedPalette[256] = {
    [0] = 0xFF2323FFu,
    [1] = 0xFFFFFEFFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniBalloonLargeRedRowOffsets[29] = {
    0u, 2u, 4u, 6u, 8u, 10u, 12u, 14u, 20u, 26u, 36u, 46u,
    56u, 66u, 72u, 82u, 92u, 98u, 104u, 110u, 116u, 122u, 128u, 134u,
    140u, 146u, 152u, 158u, 164u,
};
static const uint8_t kMiniBalloonLargeRedRuns[164] = {
    32u, 255u, 32u, 255u, 32u, 255u, 32u, 255u, 32u, 255u, 32u, 255u, 32u, 255u, 5u, 255u, 22u, 0u, 5u, 255u,
    5u, 255u, 22u, 0u, 5u, 255u, 4u, 255u, 3u, 0u, 7u, 1u, 14u, 0u, 4u, 255u, 3u, 255u, 4u, 0u,
    7u, 1u, 15u, 0u, 3u, 255u, 3u, 255u, 4u, 0u, 7u, 1u, 15u, 0u, 3u, 255u, 3u, 255u, 4u, 0u,
    7u, 1u, 15u, 0u, 3u, 255u, 3u, 255u, 26u, 0u, 3u, 255u, 3u, 255u, 3u, 0u, 3u, 1u, 20u, 0u,
    3u, 255u, 3u, 255u, 3u, 0u, 3u, 1u, 20u, 0u, 3u, 255u, 3u, 255u, 26u, 0u, 3u, 255u, 4u, 255u,
    24u, 0u, 4u, 255u, 4u, 255u, 24u, 0u, 4u, 255u, 5u, 255u, 22u, 0u, 5u, 255u, 5u, 255u, 22u, 0u,
    5u, 255u, 8u, 255u, 16u, 0u, 8u, 255u, 8u, 255u, 16u, 0u, 8u, 255u, 13u, 255u, 7u, 0u, 12u, 255u,
    16u, 255u, 1u, 1u, 15u, 255u, 16u, 255u, 1u, 1u, 15u, 255u, 16u, 255u, 1u, 1u, 15u, 255u, 16u, 255u,
    1u, 1u, 15u, 255u,
};
const TextureRle gMiniBalloonLargeRed = {
    32u, 28u, 255u,
    kMiniBalloonLargeRedRowOffsets,
    kMiniBalloonLargeRedRuns,
    kMiniBalloonLargeRedPalette,
};

static const uint32_t kMiniExitCloudPalette[256] = {
    [0] = 0x434343FFu,
    [1] = 0xEBEBEBFFu,
    [2] = 0xD4D4D4FFu,
    [3] = 0x666666FFu,
    [4] = 0xF1F1F0FFu,
    [5] = 0xA5A5A4FFu,
    [6] = 0x636363FFu,
    [255] = 0x00000000u,
};
static const uint16_t kMiniExitCloudRowOffsets[32] = {
    0u, 2u, 4u, 10u, 22u, 34u, 46u, 78u, 106u, 134u, 162u, 190u,
    218u, 230u, 242u, 254u, 266u, 286u, 302u, 318u, 330u, 342u, 354u, 366u,
    378u, 390u, 402u, 414u, 426u, 438u, 450u, 462u,
};
static const uint8_t kMiniExitCloudRuns[462] = {
    26u, 255u, 26u, 255u, 1u, 255u, 23u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u,
    2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u,
    1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 2u, 1u, 3u, 3u, 2u, 1u, 1u, 3u, 1u, 1u,
    1u, 3u, 1u, 1u, 3u, 3u, 2u, 1u, 3u, 3u, 1u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u,
    1u, 0u, 2u, 1u, 1u, 3u, 4u, 1u, 1u, 3u, 4u, 1u, 1u, 3u, 4u, 1u, 2u, 3u, 1u, 1u,
    1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 2u, 1u, 3u, 3u, 2u, 1u, 3u, 3u, 2u, 1u,
    1u, 3u, 4u, 1u, 1u, 3u, 2u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 2u, 1u,
    1u, 3u, 4u, 1u, 3u, 3u, 2u, 1u, 1u, 3u, 4u, 1u, 1u, 3u, 2u, 1u, 1u, 2u, 1u, 0u,
    2u, 255u, 1u, 255u, 1u, 0u, 2u, 1u, 3u, 3u, 4u, 1u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 1u,
    1u, 3u, 2u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 2u, 1u, 3u, 3u, 4u, 1u,
    1u, 3u, 1u, 1u, 3u, 3u, 3u, 1u, 1u, 3u, 2u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u,
    1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u,
    2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u,
    1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 1u, 1u, 1u, 4u, 1u, 5u, 2u, 6u, 15u, 1u,
    1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 1u, 1u, 4u, 6u, 15u, 1u, 1u, 2u, 1u, 0u,
    2u, 255u, 1u, 255u, 1u, 0u, 1u, 1u, 4u, 6u, 15u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u,
    1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u,
    2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u,
    1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u,
    1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u,
    2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u,
    1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u,
    1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u, 2u, 255u, 1u, 255u, 1u, 0u, 20u, 1u, 1u, 2u, 1u, 0u,
    2u, 255u,
};
const TextureRle gMiniExitCloud = {
    26u, 31u, 255u,
    kMiniExitCloudRowOffsets,
    kMiniExitCloudRuns,
    kMiniExitCloudPalette,
};
