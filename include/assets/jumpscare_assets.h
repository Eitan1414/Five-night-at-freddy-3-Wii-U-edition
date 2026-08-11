#pragma once

#include <stdint.h>

#include "renderer/texture.h"

/*
 * Shared animation-sequence descriptor.
 *
 * This header intentionally contains no PSX assets.  The runtime sequences are
 * generated from the PC General Sprites bank by convert_pc_character_visuals.py.
 */
typedef struct JumpscareSequence {
    const TextureRle *const *frames;
    uint32_t frame_count;
    uint32_t ticks_per_frame;
} JumpscareSequence;
