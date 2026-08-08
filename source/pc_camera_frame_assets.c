#include "assets/pc_camera_frame_assets.h"

#include <stddef.h>

#define EMPTY_SET { NULL, 0u }
const PcCameraFrameSet gPcCameraBaseSets[10] = {
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
};
const PcCameraFrameSet gPcCameraBackSets[10] = {
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
};
const PcCameraFrameSet gPcCameraExitSets[10] = {
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
};
const PcCameraFrameSet gPcCameraPhantomSets[10] = {
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
    EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET, EMPTY_SET,
};
const TextureRle *const gPcVentEmptyTextures[5] = {
    NULL, NULL, NULL, NULL, NULL,
};
const TextureRle *const gPcVentSpringtrapTextures[5] = {
    NULL, NULL, NULL, NULL, NULL,
};
#undef EMPTY_SET
