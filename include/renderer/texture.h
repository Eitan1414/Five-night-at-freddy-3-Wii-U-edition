#pragma once

#include <stdint.h>

typedef struct TextureRle {
    uint16_t width;
    uint16_t height;
    uint8_t transparent_index;
    const uint16_t *row_offsets;
    const uint8_t *runs;
    const uint32_t *palette;
} TextureRle;

void texture_draw_rle(uint32_t targets,
                      int x,
                      int y,
                      int width,
                      int height,
                      const TextureRle *texture);
