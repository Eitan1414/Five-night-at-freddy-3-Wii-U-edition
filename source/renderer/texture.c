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

    for (uint32_t source_y = 0u; source_y < texture->height; ++source_y) {
        const int destination_y0 = y +
            (int) ((source_y * (uint32_t) height) / texture->height);
        const int destination_y1 = y +
            (int) (((source_y + 1u) * (uint32_t) height) / texture->height);
        const int destination_height = destination_y1 - destination_y0;

        if (destination_height <= 0) {
            continue;
        }

        const uint16_t row_start = texture->row_offsets[source_y];
        const uint16_t row_end = texture->row_offsets[source_y + 1u];
        uint32_t source_x = 0u;

        for (uint16_t offset = row_start; offset + 1u < row_end; offset += 2u) {
            const uint8_t run_length = texture->runs[offset];
            const uint8_t palette_index = texture->runs[offset + 1u];
            const uint32_t next_source_x = source_x + run_length;

            if (palette_index != texture->transparent_index) {
                const int destination_x0 = x +
                    (int) ((source_x * (uint32_t) width) / texture->width);
                const int destination_x1 = x +
                    (int) ((next_source_x * (uint32_t) width) / texture->width);
                const int destination_width = destination_x1 - destination_x0;

                if (destination_width > 0) {
                    graphics_draw_rect(targets,
                                       destination_x0,
                                       destination_y0,
                                       destination_width,
                                       destination_height,
                                       texture->palette[palette_index]);
                }
            }

            source_x = next_source_x;
        }
    }
}
