#include "platform/graphics.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The source sheet supplied for the title menu uses a narrow, square pixel
 * alphabet with a yellow-green fringe around a bright centre.  Keep a compact
 * 5x7 logical alphabet here, then stretch it to the original-looking 7x13
 * proportions when drawing.  This avoids introducing SDL_ttf/font files and
 * keeps the renderer deterministic on Wii U.
 */
static bool fnaf3_menu_glyph(char character, uint8_t rows[7])
{
#define GLYPH(a, b, c, d, e, f, g) \
    do {                              \
        rows[0] = (a);                \
        rows[1] = (b);                \
        rows[2] = (c);                \
        rows[3] = (d);                \
        rows[4] = (e);                \
        rows[5] = (f);                \
        rows[6] = (g);                \
    } while (0)

    switch (character) {
        case 'A': GLYPH(0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11); break;
        case 'B': GLYPH(0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E); break;
        case 'C': GLYPH(0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F); break;
        case 'D': GLYPH(0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E); break;
        case 'E': GLYPH(0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F); break;
        case 'F': GLYPH(0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10); break;
        case 'G': GLYPH(0x0F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0F); break;
        case 'H': GLYPH(0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11); break;
        case 'I': GLYPH(0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F); break;
        case 'J': GLYPH(0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E); break;
        case 'K': GLYPH(0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11); break;
        case 'L': GLYPH(0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F); break;
        case 'M': GLYPH(0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11); break;
        case 'N': GLYPH(0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11); break;
        case 'O': GLYPH(0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E); break;
        case 'P': GLYPH(0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10); break;
        case 'Q': GLYPH(0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D); break;
        case 'R': GLYPH(0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11); break;
        case 'S': GLYPH(0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E); break;
        case 'T': GLYPH(0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04); break;
        case 'U': GLYPH(0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E); break;
        case 'V': GLYPH(0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04); break;
        case 'W': GLYPH(0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A); break;
        case 'X': GLYPH(0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11); break;
        case 'Y': GLYPH(0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04); break;
        case 'Z': GLYPH(0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F); break;
        case '0': GLYPH(0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E); break;
        case '1': GLYPH(0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E); break;
        case '2': GLYPH(0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F); break;
        case '3': GLYPH(0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E); break;
        case '4': GLYPH(0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02); break;
        case '5': GLYPH(0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E); break;
        case '6': GLYPH(0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E); break;
        case '7': GLYPH(0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08); break;
        case '8': GLYPH(0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E); break;
        case '9': GLYPH(0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E); break;
        case '-': GLYPH(0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00); break;
        case ':': GLYPH(0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00); break;
        case '.': GLYPH(0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C); break;
        case '/': GLYPH(0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10); break;
        case '!': GLYPH(0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04); break;
        case '?': GLYPH(0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04); break;
        case ' ': GLYPH(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00); break;
        default: return false;
    }

#undef GLYPH
    return true;
}

static void fnaf3_menu_draw_pixel(uint32_t targets,
                                  int x,
                                  int y,
                                  int scale,
                                  uint32_t colour)
{
    graphics_draw_rect(targets, x, y, scale, scale, colour);
}

void graphics_draw_fnaf3_menu_text(uint32_t targets,
                                   int x,
                                   int y,
                                   int scale,
                                   const char *text,
                                   uint32_t colour)
{
    if (text == NULL || scale <= 0) return;

    const uint32_t fringe = GRAPHICS_RGB(103, 132, 0);
    const uint32_t shadow = GRAPHICS_RGB(18, 27, 0);
    int cursor_x = x;
    uint8_t rows[7];

    while (*text != '\0') {
        if (*text == '\n') {
            cursor_x = x;
            y += 15 * scale;
            ++text;
            continue;
        }

        if (fnaf3_menu_glyph(*text, rows)) {
            /* 5x7 logical glyph -> 7x13 menu proportions. */
            for (int destination_row = 0; destination_row < 13; ++destination_row) {
                const int source_row = (destination_row * 7) / 13;
                for (int destination_column = 0; destination_column < 7; ++destination_column) {
                    const int source_column = (destination_column * 5) / 7;
                    if ((rows[source_row] & (1u << (4 - source_column))) == 0u)
                        continue;

                    const int pixel_x = cursor_x + destination_column * scale;
                    const int pixel_y = y + destination_row * scale;

                    /* Dark offset plus the olive fringe visible on the ripped menu text. */
                    fnaf3_menu_draw_pixel(targets,
                                          pixel_x + scale,
                                          pixel_y + scale,
                                          scale,
                                          shadow);
                    fnaf3_menu_draw_pixel(targets,
                                          pixel_x - scale,
                                          pixel_y,
                                          scale,
                                          fringe);
                    fnaf3_menu_draw_pixel(targets,
                                          pixel_x + scale,
                                          pixel_y,
                                          scale,
                                          fringe);
                    fnaf3_menu_draw_pixel(targets,
                                          pixel_x,
                                          pixel_y - scale,
                                          scale,
                                          fringe);
                    fnaf3_menu_draw_pixel(targets,
                                          pixel_x,
                                          pixel_y + scale,
                                          scale,
                                          fringe);
                    fnaf3_menu_draw_pixel(targets,
                                          pixel_x,
                                          pixel_y,
                                          scale,
                                          colour);
                }
            }
        }

        cursor_x += 9 * scale;
        ++text;
    }
}
