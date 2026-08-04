#include "renderer/sprite.h"

#include <stddef.h>

#include "platform/graphics.h"

void sprite_draw_mask(uint32_t targets,
                      int x,
                      int y,
                      int scale,
                      const SpriteMask *sprite,
                      uint32_t colour)
{
    if (sprite == NULL || sprite->pixels == NULL || scale <= 0) {
        return;
    }

    for (uint16_t row = 0; row < sprite->height; ++row) {
        for (uint16_t column = 0; column < sprite->width; ++column) {
            const uint32_t index = (uint32_t) row * sprite->width + column;
            if (sprite->pixels[index] != 0u) {
                graphics_draw_rect(targets,
                                   x + (int) column * scale,
                                   y + (int) row * scale,
                                   scale,
                                   scale,
                                   colour);
            }
        }
    }
}
