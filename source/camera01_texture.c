/* Generated from camera 01 CAM.TIM, left frame, 2x sampled. */
#include "assets/camera01_texture.h"
#include <stdint.h>
static const uint32_t kCamera01Palette[256] = {
    [0] = 0x000000FFu,
    [1] = 0xAC8B8BFFu,
    [2] = 0x7B4A4AFFu,
    [3] = 0x624141FFu,
    [4] = 0x4A2029FFu,
    [5] = 0x946262FFu,
    [6] = 0x292020FFu,
    [7] = 0xFFD5D5FFu,
    [8] = 0x291010FFu,
    [9] = 0xD59494FFu,
    [10] = 0xB47373FFu,
    [11] = 0xF6ACB4FFu,
    [12] = 0xB4ACB4FFu,
    [13] = 0xFFFFFFFFu,
    [14] = 0x5A5A4AFFu,
    [15] = 0x9C7373FFu,
    [16 ... 255] = 0x000000FFu,
};
static const uint16_t kCamera01RowOffsets[56] = {
    0u, 70u, 130u, 204u, 264u, 342u, 400u, 478u, 558u, 624u, 690u, 754u,
    828u, 884u, 938u, 992u, 1046u, 1084u, 1138u, 1206u, 1260u, 1304u, 1346u, 1370u,
    1388u, 1404u, 1414u, 1424u, 1434u, 1444u, 1450u, 1460u, 1474u, 1482u, 1494u, 1512u,
    1540u, 1572u, 1600u, 1626u, 1648u, 1668u, 1684u, 1706u, 1720u, 1736u, 1752u, 1772u,
    1798u, 1820u, 1842u, 1874u, 1896u, 1916u, 1932u, 1954u,
};
static const uint8_t kCamera01Runs[1954] = {
#include "assets/camera01_runs_0.inc"
#include "assets/camera01_runs_1.inc"
};
const TextureRle gCamera01Texture = {
    69u, 55u, 255u,
    kCamera01RowOffsets,
    kCamera01Runs,
    kCamera01Palette,
};
