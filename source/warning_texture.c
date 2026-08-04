/* Generated from WARNING.tim by tools/convert_tim.py (2x sampled). */
#include "assets/warning_texture.h"

#include <stdint.h>

static const uint32_t kPalette[256] = {
    [0] = 0x000000FFu,
    [1] = 0xCDCDCDFFu,
};

static const uint16_t kRowOffsets[24] = {
    0u, 42u, 64u, 94u, 132u, 134u, 136u, 138u, 140u, 142u, 144u, 146u,
    148u, 194u, 322u, 454u, 590u, 604u, 606u, 620u, 718u, 784u, 878u, 892u,
};

static const uint8_t kRuns[892] = {
#include "assets/warning_runs_0.inc"
#include "assets/warning_runs_1.inc"
};

const TextureRle gWarningTexture = {
    120u,
    23u,
    0u,
    kRowOffsets,
    kRuns,
    kPalette,
};
