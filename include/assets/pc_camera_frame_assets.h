#pragma once

#include <stdint.h>
#include "renderer/texture.h"

typedef struct PcCameraFrameSet {
    const TextureRle *const *frames;
    uint8_t count;
} PcCameraFrameSet;

extern const PcCameraFrameSet gPcCameraBaseSets[10];
extern const PcCameraFrameSet gPcCameraBackSets[10];
extern const PcCameraFrameSet gPcCameraExitSets[10];
extern const PcCameraFrameSet gPcCameraPhantomSets[10];
extern const TextureRle *const gPcVentEmptyTextures[5];
extern const TextureRle *const gPcVentSpringtrapTextures[5];
