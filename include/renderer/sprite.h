#pragma once

#include <stdint.h>

typedef struct SpriteMask {
    uint16_t width;
    uint16_t height;
    const uint8_t *pixels;
} SpriteMask;

void sprite_draw_mask(uint32_t targets,
                      int x,
                      int y,
                      int scale,
                      const SpriteMask *sprite,
                      uint32_t colour);
