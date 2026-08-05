#include "renderer/texture.h"

#include <stddef.h>

#include "platform/graphics.h"

void texture_draw_rle(uint32_t targets,
                      int x,
                      int y,
                      int width,
                      int height,
                      const TextureRle *texture)
{
    if (texture == NULL || texture->row_offsets == NULL ||
        texture->runs == NULL || texture->palette == NULL ||
        texture->width == 0u || texture->height == 0u ||
        width <= 0 || height <= 0) {
        return;
    }

    graphics_draw_indexed_rle(targets,
                              x,
                              y,
                              width,
                              height,
                              texture,
                              texture->width,
                              texture->height,
                              texture->transparent_index,
                              texture->row_offsets,
                              texture->runs,
                              texture->palette);
}
