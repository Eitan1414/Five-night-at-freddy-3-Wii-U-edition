#include "platform/graphics.h"

#include <stddef.h>
#include <stdint.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/screen.h>

typedef struct DisplayInfo {
    OSScreenID id;
    uint32_t target;
    uint32_t width;
    uint32_t height;
} DisplayInfo;

static const DisplayInfo kDisplays[] = {
    {SCREEN_TV, GRAPHICS_TARGET_TV, 1280u, 720u},
    {SCREEN_DRC, GRAPHICS_TARGET_GAMEPAD, 854u, 480u},
};

static void *sTvBuffer = NULL;
static void *sGamePadBuffer = NULL;

static uint32_t scale_x(const DisplayInfo *display, int x)
{
    return (uint32_t) ((x * (int) display->width) / GRAPHICS_LOGICAL_WIDTH);
}

static uint32_t scale_y(const DisplayInfo *display, int y)
{
    return (uint32_t) ((y * (int) display->height) / GRAPHICS_LOGICAL_HEIGHT);
}

static void draw_rect_on_display(const DisplayInfo *display,
                                 int x,
                                 int y,
                                 int width,
                                 int height,
                                 uint32_t colour)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    int right = x + width;
    int bottom = y + height;

    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (right > GRAPHICS_LOGICAL_WIDTH) {
        right = GRAPHICS_LOGICAL_WIDTH;
    }
    if (bottom > GRAPHICS_LOGICAL_HEIGHT) {
        bottom = GRAPHICS_LOGICAL_HEIGHT;
    }
    if (x >= right || y >= bottom) {
        return;
    }

    const uint32_t left_pixel = scale_x(display, x);
    const uint32_t top_pixel = scale_y(display, y);
    uint32_t right_pixel = scale_x(display, right);
    uint32_t bottom_pixel = scale_y(display, bottom);

    if (right_pixel <= left_pixel) {
        right_pixel = left_pixel + 1u;
    }
    if (bottom_pixel <= top_pixel) {
        bottom_pixel = top_pixel + 1u;
    }

    for (uint32_t pixel_y = top_pixel; pixel_y < bottom_pixel; ++pixel_y) {
        for (uint32_t pixel_x = left_pixel; pixel_x < right_pixel; ++pixel_x) {
            OSScreenPutPixelEx(display->id, pixel_x, pixel_y, colour);
        }
    }
}

static bool get_glyph(char character, uint8_t rows[7])
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
        case '>': GLYPH(0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10); break;
        case '<': GLYPH(0x01, 0x02, 0x04, 0x08, 0x04, 0x02, 0x01); break;
        case ' ': GLYPH(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00); break;
        default: return false;
    }

#undef GLYPH
    return true;
}

bool graphics_init(void)
{
    OSScreenInit();

    const uint32_t tv_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    const uint32_t gamepad_size = OSScreenGetBufferSizeEx(SCREEN_DRC);

    sTvBuffer = MEMAllocFromDefaultHeapEx(tv_size, 0x100);
    sGamePadBuffer = MEMAllocFromDefaultHeapEx(gamepad_size, 0x100);

    if (sTvBuffer == NULL || sGamePadBuffer == NULL) {
        graphics_shutdown();
        return false;
    }

    OSScreenSetBufferEx(SCREEN_TV, sTvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, sGamePadBuffer);
    OSScreenEnableEx(SCREEN_TV, TRUE);
    OSScreenEnableEx(SCREEN_DRC, TRUE);

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(0, 0, 0));
    graphics_present(GRAPHICS_TARGET_BOTH);
    return true;
}

void graphics_shutdown(void)
{
    OSScreenEnableEx(SCREEN_TV, FALSE);
    OSScreenEnableEx(SCREEN_DRC, FALSE);
    OSScreenShutdown();

    if (sTvBuffer != NULL) {
        MEMFreeToDefaultHeap(sTvBuffer);
        sTvBuffer = NULL;
    }
    if (sGamePadBuffer != NULL) {
        MEMFreeToDefaultHeap(sGamePadBuffer);
        sGamePadBuffer = NULL;
    }
}

void graphics_clear(uint32_t targets, uint32_t colour)
{
    for (uint32_t index = 0u;
         index < sizeof(kDisplays) / sizeof(kDisplays[0]);
         ++index) {
        if ((targets & kDisplays[index].target) != 0u) {
            OSScreenClearBufferEx(kDisplays[index].id, colour);
        }
    }
}

void graphics_present(uint32_t targets)
{
    for (uint32_t index = 0u;
         index < sizeof(kDisplays) / sizeof(kDisplays[0]);
         ++index) {
        if ((targets & kDisplays[index].target) != 0u) {
            OSScreenFlipBuffersEx(kDisplays[index].id);
        }
    }
}

void graphics_draw_rect(uint32_t targets,
                        int x,
                        int y,
                        int width,
                        int height,
                        uint32_t colour)
{
    for (uint32_t index = 0u;
         index < sizeof(kDisplays) / sizeof(kDisplays[0]);
         ++index) {
        if ((targets & kDisplays[index].target) != 0u) {
            draw_rect_on_display(&kDisplays[index], x, y, width, height, colour);
        }
    }
}

void graphics_draw_frame(uint32_t targets,
                         int x,
                         int y,
                         int width,
                         int height,
                         int thickness,
                         uint32_t colour)
{
    graphics_draw_rect(targets, x, y, width, thickness, colour);
    graphics_draw_rect(targets,
                       x,
                       y + height - thickness,
                       width,
                       thickness,
                       colour);
    graphics_draw_rect(targets, x, y, thickness, height, colour);
    graphics_draw_rect(targets,
                       x + width - thickness,
                       y,
                       thickness,
                       height,
                       colour);
}

void graphics_draw_ellipse(uint32_t targets,
                           int centre_x,
                           int centre_y,
                           int radius_x,
                           int radius_y,
                           uint32_t colour)
{
    if (radius_x <= 0 || radius_y <= 0) {
        return;
    }

    const int64_t radius_x_squared = (int64_t) radius_x * radius_x;
    const int64_t radius_y_squared = (int64_t) radius_y * radius_y;
    const int64_t limit = radius_x_squared * radius_y_squared;

    for (int y = -radius_y; y <= radius_y; ++y) {
        int first_x = radius_x;
        int last_x = -radius_x;

        for (int x = -radius_x; x <= radius_x; ++x) {
            const int64_t value = (int64_t) x * x * radius_y_squared +
                                  (int64_t) y * y * radius_x_squared;
            if (value <= limit) {
                if (x < first_x) {
                    first_x = x;
                }
                if (x > last_x) {
                    last_x = x;
                }
            }
        }

        if (last_x >= first_x) {
            graphics_draw_rect(targets,
                               centre_x + first_x,
                               centre_y + y,
                               last_x - first_x + 1,
                               1,
                               colour);
        }
    }
}

void graphics_draw_text(uint32_t targets,
                        int x,
                        int y,
                        int scale,
                        const char *text,
                        uint32_t colour)
{
    if (text == NULL || scale <= 0) {
        return;
    }

    int cursor_x = x;
    uint8_t rows[7];

    while (*text != '\0') {
        if (*text == '\n') {
            cursor_x = x;
            y += 9 * scale;
            ++text;
            continue;
        }

        if (get_glyph(*text, rows)) {
            for (int row = 0; row < 7; ++row) {
                for (int column = 0; column < 5; ++column) {
                    if ((rows[row] & (1u << (4 - column))) != 0u) {
                        graphics_draw_rect(targets,
                                           cursor_x + column * scale,
                                           y + row * scale,
                                           scale,
                                           scale,
                                           colour);
                    }
                }
            }
        }

        cursor_x += 6 * scale;
        ++text;
    }
}
