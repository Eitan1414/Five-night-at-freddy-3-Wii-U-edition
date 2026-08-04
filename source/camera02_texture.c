/* Generated from camera 02 CAM.TIM, left frame, 2x sampled. */
#include "assets/camera02_texture.h"
#include <stdint.h>
static const uint32_t kCamera02Palette[256] = {
    [0] = 0x080000FFu,
    [1] = 0x738B7BFFu,
    [2] = 0x394A39FFu,
    [3] = 0x83625AFFu,
    [4] = 0x4A3131FFu,
    [5] = 0x5A6A5AFFu,
    [6] = 0x202920FFu,
    [7] = 0x6A4A4AFFu,
    [8] = 0xB4CDB4FFu,
    [9] = 0xBD948BFFu,
    [10] = 0xFFF6EEFFu,
    [11] = 0x94AC9CFFu,
    [12] = 0xCDAC9CFFu,
    [13] = 0xFFD5BDFFu,
    [14] = 0x94736AFFu,
    [15] = 0xA48373FFu,
    [16 ... 255] = 0x000000FFu,
};
static const uint16_t kCamera02RowOffsets[56] = {
    0u, 10u, 26u, 50u, 58u, 84u, 98u, 128u, 152u, 178u, 204u, 232u,
    250u, 266u, 284u, 306u, 326u, 360u, 404u, 440u, 482u, 500u, 540u, 590u,
    634u, 666u, 698u, 738u, 770u, 790u, 828u, 864u, 908u, 950u, 988u, 1060u,
    1118u, 1170u, 1210u, 1262u, 1304u, 1346u, 1382u, 1414u, 1438u, 1472u, 1514u, 1546u,
    1590u, 1636u, 1682u, 1730u, 1784u, 1828u, 1898u, 1948u,
};
static const uint8_t kCamera02Runs[1948] = {
#include "assets/camera02_runs_0.inc"
#include "assets/camera02_runs_1.inc"
};
const TextureRle gCamera02Texture = {
    69u, 55u, 255u,
    kCamera02RowOffsets,
    kCamera02Runs,
    kCamera02Palette,
};
