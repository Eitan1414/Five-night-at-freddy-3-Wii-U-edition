/* Generated from camera 03 CAM.TIM, left frame, 2x sampled. */
#include "assets/camera03_texture.h"
#include <stdint.h>
static const uint32_t kCamera03Palette[256] = {
    [0] = 0x000000FFu,
    [1] = 0x738362FFu,
    [2] = 0x414A39FFu,
    [3] = 0x202920FFu,
    [4] = 0x735A5AFFu,
    [5] = 0x5A6A52FFu,
    [6] = 0x393131FFu,
    [7] = 0x201818FFu,
    [8] = 0x5A4A4AFFu,
    [9] = 0xCDD5CDFFu,
    [10] = 0xB48383FFu,
    [11] = 0xFFFFFFFFu,
    [12] = 0xA4A4A4FFu,
    [13] = 0xBDB4B4FFu,
    [14] = 0x737B62FFu,
    [15] = 0x94948BFFu,
    [16 ... 255] = 0x000000FFu,
};
static const uint16_t kCamera03RowOffsets[56] = {
    0u, 64u, 112u, 172u, 222u, 256u, 310u, 372u, 424u, 474u, 536u, 594u,
    642u, 690u, 742u, 794u, 840u, 880u, 922u, 956u, 982u, 1000u, 1018u, 1044u,
    1064u, 1074u, 1086u, 1104u, 1134u, 1172u, 1212u, 1256u, 1300u, 1338u, 1388u, 1426u,
    1458u, 1506u, 1562u, 1608u, 1646u, 1702u, 1746u, 1782u, 1802u, 1826u, 1840u, 1868u,
    1898u, 1938u, 1990u, 2040u, 2112u, 2186u, 2264u, 2282u,
};
static const uint8_t kCamera03Runs[2282] = {
#include "assets/camera03_runs_0.inc"
#include "assets/camera03_runs_1.inc"
};
const TextureRle gCamera03Texture = {
    69u, 55u, 255u,
    kCamera03RowOffsets,
    kCamera03Runs,
    kCamera03Palette,
};
