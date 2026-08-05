#pragma once

#include <stdbool.h>
#include <stdint.h>

#define GRAPHICS_LOGICAL_WIDTH 854
#define GRAPHICS_LOGICAL_HEIGHT 480

#define GRAPHICS_RGB(r, g, b) \
    ((((uint32_t) (r)) << 24) | (((uint32_t) (g)) << 16) | \
     (((uint32_t) (b)) << 8) | 0xFFu)

typedef enum GraphicsTarget {
    GRAPHICS_TARGET_TV = 1u << 0,
    GRAPHICS_TARGET_GAMEPAD = 1u << 1,
    GRAPHICS_TARGET_BOTH = GRAPHICS_TARGET_TV | GRAPHICS_TARGET_GAMEPAD
} GraphicsTarget;

bool graphics_init(void);
void graphics_shutdown(void);

void graphics_clear(uint32_t targets, uint32_t colour);
void graphics_present(uint32_t targets);

void graphics_draw_rect(uint32_t targets,
                        int x,
                        int y,
                        int width,
                        int height,
                        uint32_t colour);
void graphics_draw_frame(uint32_t targets,
                         int x,
                         int y,
                         int width,
                         int height,
                         int thickness,
                         uint32_t colour);
void graphics_draw_ellipse(uint32_t targets,
                           int centre_x,
                           int centre_y,
                           int radius_x,
                           int radius_y,
                           uint32_t colour);
void graphics_draw_text(uint32_t targets,
                        int x,
                        int y,
                        int scale,
                        const char *text,
                        uint32_t colour);

/*
 * Uploads an indexed RLE image once and draws it as a GX2-backed SDL texture.
 * cache_key must remain stable for the lifetime of the source texture.
 */
void graphics_draw_indexed_rle(uint32_t targets,
                               int x,
                               int y,
                               int width,
                               int height,
                               const void *cache_key,
                               uint16_t source_width,
                               uint16_t source_height,
                               uint8_t transparent_index,
                               const uint16_t *row_offsets,
                               const uint8_t *runs,
                               const uint32_t *palette);
