#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <vpad/input.h>
#include <whb/proc.h>

#define LOGICAL_WIDTH 854
#define LOGICAL_HEIGHT 480
#define WARNING_FRAMES 180
#define GLITCH_INTERVAL 180

#define RGBX(r, g, b) \
    ((((uint32_t) (r)) << 24) | (((uint32_t) (g)) << 16) | \
     (((uint32_t) (b)) << 8) | 0xFFu)

typedef struct DisplayInfo {
    OSScreenID id;
    uint32_t width;
    uint32_t height;
} DisplayInfo;

typedef enum AppScreen {
    APP_SCREEN_WARNING = 0,
    APP_SCREEN_TITLE,
    APP_SCREEN_PLACEHOLDER
} AppScreen;

static const DisplayInfo kDisplays[] = {
    {SCREEN_TV, 1280, 720},
    {SCREEN_DRC, 854, 480},
};

static void *sTvBuffer = NULL;
static void *sDrcBuffer = NULL;
static uint32_t sNoiseSeed = 0xF3A30001u;

static uint32_t random_u32(void)
{
    sNoiseSeed = sNoiseSeed * 1664525u + 1013904223u;
    return sNoiseSeed;
}

static uint32_t scale_x(const DisplayInfo *display, int x)
{
    return (uint32_t) ((x * (int) display->width) / LOGICAL_WIDTH);
}

static uint32_t scale_y(const DisplayInfo *display, int y)
{
    return (uint32_t) ((y * (int) display->height) / LOGICAL_HEIGHT);
}

static void draw_rect(const DisplayInfo *display,
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
    if (right > LOGICAL_WIDTH) {
        right = LOGICAL_WIDTH;
    }
    if (bottom > LOGICAL_HEIGHT) {
        bottom = LOGICAL_HEIGHT;
    }
    if (x >= right || y >= bottom) {
        return;
    }

    const uint32_t left_px = scale_x(display, x);
    const uint32_t top_px = scale_y(display, y);
    uint32_t right_px = scale_x(display, right);
    uint32_t bottom_px = scale_y(display, bottom);

    if (right_px <= left_px) {
        right_px = left_px + 1;
    }
    if (bottom_px <= top_px) {
        bottom_px = top_px + 1;
    }

    for (uint32_t py = top_px; py < bottom_px; ++py) {
        for (uint32_t px = left_px; px < right_px; ++px) {
            OSScreenPutPixelEx(display->id, px, py, colour);
        }
    }
}

static void draw_ellipse(const DisplayInfo *display,
                         int centre_x,
                         int centre_y,
                         int radius_x,
                         int radius_y,
                         uint32_t colour)
{
    if (radius_x <= 0 || radius_y <= 0) {
        return;
    }

    const int64_t radius_x_sq = (int64_t) radius_x * radius_x;
    const int64_t radius_y_sq = (int64_t) radius_y * radius_y;
    const int64_t limit = radius_x_sq * radius_y_sq;

    for (int y = -radius_y; y <= radius_y; ++y) {
        for (int x = -radius_x; x <= radius_x; ++x) {
            const int64_t value = (int64_t) x * x * radius_y_sq +
                                  (int64_t) y * y * radius_x_sq;
            if (value <= limit) {
                draw_rect(display, centre_x + x, centre_y + y, 1, 1, colour);
            }
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
        case ' ': GLYPH(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00); break;
        default: return false;
    }

#undef GLYPH
    return true;
}

static void draw_text(const DisplayInfo *display,
                      int x,
                      int y,
                      int scale,
                      const char *text,
                      uint32_t colour)
{
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
                        draw_rect(display,
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

static void draw_frame(const DisplayInfo *display,
                       int x,
                       int y,
                       int width,
                       int height,
                       int thickness,
                       uint32_t colour)
{
    draw_rect(display, x, y, width, thickness, colour);
    draw_rect(display, x, y + height - thickness, width, thickness, colour);
    draw_rect(display, x, y, thickness, height, colour);
    draw_rect(display, x + width - thickness, y, thickness, height, colour);
}

static void draw_scanlines(const DisplayInfo *display, uint32_t seed)
{
    for (int y = 0; y < LOGICAL_HEIGHT; y += 12) {
        draw_rect(display, 0, y, LOGICAL_WIDTH, 1, RGBX(5, 12, 7));
    }

    uint32_t local_seed = seed;
    for (int i = 0; i < 9; ++i) {
        local_seed = local_seed * 1103515245u + 12345u;
        const int y = (int) ((local_seed >> 16) % LOGICAL_HEIGHT);
        const int height = 1 + (int) ((local_seed >> 28) & 0x03u);
        const uint32_t brightness = 22u + ((local_seed >> 8) & 0x1Fu);
        draw_rect(display,
                  0,
                  y,
                  LOGICAL_WIDTH,
                  height,
                  RGBX(5, brightness, 13));
    }
}

static void draw_springtrap_silhouette(const DisplayInfo *display, uint32_t seed)
{
    const uint32_t shadow = RGBX(7, 13, 8);
    const uint32_t darkest = RGBX(13, 24, 14);
    const uint32_t suit = RGBX(31, 58, 32);
    const uint32_t suit_light = RGBX(48, 83, 45);
    const uint32_t decay = RGBX(54, 46, 26);
    const uint32_t eye = RGBX(210, 255, 205);

    draw_ellipse(display, 671, 382, 145, 123, shadow);
    draw_rect(display, 568, 329, 207, 151, darkest);
    draw_ellipse(display, 671, 347, 108, 88, suit);
    draw_rect(display, 607, 390, 128, 90, suit);

    draw_ellipse(display, 674, 211, 104, 123, darkest);
    draw_ellipse(display, 674, 213, 91, 108, suit);

    draw_rect(display, 615, 39, 38, 105, suit);
    draw_rect(display, 621, 28, 26, 47, suit_light);
    draw_rect(display, 619, 82, 34, 17, darkest);
    draw_rect(display, 711, 49, 35, 91, suit);
    draw_rect(display, 720, 31, 24, 47, suit_light);
    draw_rect(display, 724, 86, 22, 21, darkest);
    draw_rect(display, 737, 33, 11, 18, RGBX(2, 4, 2));

    draw_ellipse(display, 642, 188, 31, 28, RGBX(2, 5, 3));
    draw_ellipse(display, 708, 187, 31, 29, RGBX(2, 5, 3));
    draw_ellipse(display, 645, 190, 8, 7, eye);
    draw_ellipse(display, 705, 189, 8, 7, eye);
    draw_rect(display, 646, 190, 5, 2, RGBX(255, 255, 255));
    draw_rect(display, 706, 189, 5, 2, RGBX(255, 255, 255));

    draw_ellipse(display, 675, 250, 58, 45, suit_light);
    draw_rect(display, 628, 251, 94, 24, decay);
    draw_rect(display, 637, 261, 76, 28, RGBX(5, 8, 5));
    for (int i = 0; i < 7; ++i) {
        draw_rect(display, 643 + i * 10, 259, 4, 13, RGBX(185, 179, 130));
    }

    draw_rect(display, 607, 154, 18, 37, RGBX(2, 4, 2));
    draw_rect(display, 729, 231, 17, 46, RGBX(2, 4, 2));
    draw_rect(display, 650, 118, 16, 31, decay);
    draw_rect(display, 697, 284, 28, 18, decay);
    draw_rect(display, 626, 346, 23, 48, RGBX(4, 8, 4));
    draw_rect(display, 704, 365, 25, 57, RGBX(4, 8, 4));
    draw_rect(display, 612, 297, 5, 81, RGBX(104, 87, 46));
    draw_rect(display, 735, 279, 4, 93, RGBX(104, 87, 46));
    draw_rect(display, 743, 304, 3, 69, RGBX(87, 35, 27));

    if ((seed & 0x03u) == 0u) {
        draw_rect(display, 596, 202, 151, 3, RGBX(76, 128, 69));
        draw_rect(display, 617, 274, 122, 2, RGBX(101, 148, 88));
    }
}

static void clear_and_draw_warning(const DisplayInfo *display, uint8_t intensity)
{
    const uint32_t red = RGBX(intensity, intensity / 8u, intensity / 12u);
    const uint32_t white = RGBX(intensity, intensity, intensity);

    OSScreenClearBufferEx(display->id, RGBX(0, 0, 0));
    draw_frame(display, 74, 76, 706, 326, 4, red);
    draw_text(display, 263, 111, 7, "WARNING", red);
    draw_text(display, 142, 212, 3, "FLASHING LIGHTS", white);
    draw_text(display, 214, 258, 3, "LOUD NOISES", white);
    draw_text(display, 214, 304, 3, "JUMPSCARES", white);
    draw_text(display, 268, 365, 2, "PRESS A TO SKIP", RGBX(intensity / 2u,
                                                              intensity,
                                                              intensity / 2u));
}

static void render_warning(uint8_t intensity)
{
    for (uint32_t i = 0; i < sizeof(kDisplays) / sizeof(kDisplays[0]); ++i) {
        clear_and_draw_warning(&kDisplays[i], intensity);
        OSScreenFlipBuffersEx(kDisplays[i].id);
    }
}

static void clear_and_draw_title(const DisplayInfo *display,
                                 int selected_item,
                                 uint32_t glitch_seed)
{
    static const char *const menu_items[] = {
        "NEW GAME",
        "LOAD GAME",
        "EXTRAS",
    };

    OSScreenClearBufferEx(display->id, RGBX(1, 4, 2));

    draw_rect(display, 0, 0, 410, LOGICAL_HEIGHT, RGBX(2, 7, 4));
    draw_rect(display, 405, 0, 4, LOGICAL_HEIGHT, RGBX(18, 44, 24));

    draw_text(display, 43, 48, 7, "F", RGBX(206, 231, 207));
    draw_text(display, 43, 120, 7, "N", RGBX(206, 231, 207));
    draw_text(display, 43, 192, 7, "A", RGBX(206, 231, 207));
    draw_text(display, 43, 264, 7, "F", RGBX(206, 231, 207));
    draw_text(display, 43, 350, 8, "3", RGBX(89, 151, 83));

    draw_text(display, 122, 72, 2, "FIVE NIGHTS AT", RGBX(113, 151, 111));
    draw_text(display, 122, 93, 2, "FREDDYS 3", RGBX(113, 151, 111));
    draw_text(display, 122, 125, 1, "WII U EDITION - EARLY PORT", RGBX(61, 95, 66));

    for (int item = 0; item < 3; ++item) {
        const int y = 284 + item * 48;
        if (item == selected_item) {
            draw_rect(display, 108, y - 10, 255, 36, RGBX(31, 72, 37));
            draw_rect(display, 108, y - 10, 5, 36, RGBX(129, 207, 121));
            draw_text(display, 126, y, 3, menu_items[item], RGBX(226, 244, 225));
        } else {
            draw_text(display, 126, y, 3, menu_items[item], RGBX(119, 142, 120));
        }
    }

    draw_text(display, 116, 447, 1, "UP DOWN: SELECT   A: CONFIRM", RGBX(90, 126, 93));

    draw_springtrap_silhouette(display, glitch_seed);
    draw_scanlines(display, glitch_seed);

    if ((glitch_seed & 0x07u) == 0u) {
        const int offset_y = 115 + (int) ((glitch_seed >> 8) % 220u);
        draw_rect(display, 480, offset_y, 330, 4, RGBX(23, 87, 42));
        draw_rect(display, 523, offset_y + 7, 262, 2, RGBX(78, 129, 70));
    }
}

static void render_title(int selected_item, uint32_t glitch_seed)
{
    for (uint32_t i = 0; i < sizeof(kDisplays) / sizeof(kDisplays[0]); ++i) {
        clear_and_draw_title(&kDisplays[i], selected_item, glitch_seed);
        OSScreenFlipBuffersEx(kDisplays[i].id);
    }
}

static void clear_and_draw_placeholder(const DisplayInfo *display,
                                       int selected_item)
{
    static const char *const headings[] = {
        "NEW GAME",
        "LOAD GAME",
        "EXTRAS",
    };
    static const char *const details[] = {
        "GAMEPLAY PORT IS NEXT",
        "SAVE SYSTEM NOT PORTED YET",
        "EXTRAS NOT PORTED YET",
    };

    OSScreenClearBufferEx(display->id, RGBX(1, 5, 3));
    draw_frame(display, 61, 73, 732, 334, 3, RGBX(36, 91, 44));
    draw_text(display, 128, 127, 6, headings[selected_item], RGBX(203, 235, 202));
    draw_text(display, 145, 243, 3, details[selected_item], RGBX(98, 151, 100));
    draw_text(display, 238, 335, 2, "PRESS B TO RETURN", RGBX(128, 186, 128));
    draw_scanlines(display, 0xBACC0000u + (uint32_t) selected_item);
}

static void render_placeholder(int selected_item)
{
    for (uint32_t i = 0; i < sizeof(kDisplays) / sizeof(kDisplays[0]); ++i) {
        clear_and_draw_placeholder(&kDisplays[i], selected_item);
        OSScreenFlipBuffersEx(kDisplays[i].id);
    }
}

static bool initialise_screens(void)
{
    OSScreenInit();

    const uint32_t tv_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    const uint32_t drc_size = OSScreenGetBufferSizeEx(SCREEN_DRC);

    sTvBuffer = MEMAllocFromDefaultHeapEx(tv_size, 0x100);
    sDrcBuffer = MEMAllocFromDefaultHeapEx(drc_size, 0x100);

    if (sTvBuffer == NULL || sDrcBuffer == NULL) {
        if (sTvBuffer != NULL) {
            MEMFreeToDefaultHeap(sTvBuffer);
            sTvBuffer = NULL;
        }
        if (sDrcBuffer != NULL) {
            MEMFreeToDefaultHeap(sDrcBuffer);
            sDrcBuffer = NULL;
        }
        OSScreenShutdown();
        return false;
    }

    OSScreenSetBufferEx(SCREEN_TV, sTvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, sDrcBuffer);
    OSScreenEnableEx(SCREEN_TV, TRUE);
    OSScreenEnableEx(SCREEN_DRC, TRUE);

    OSScreenClearBufferEx(SCREEN_TV, RGBX(0, 0, 0));
    OSScreenClearBufferEx(SCREEN_DRC, RGBX(0, 0, 0));
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    return true;
}

static void shutdown_screens(void)
{
    OSScreenEnableEx(SCREEN_TV, FALSE);
    OSScreenEnableEx(SCREEN_DRC, FALSE);
    OSScreenShutdown();

    if (sTvBuffer != NULL) {
        MEMFreeToDefaultHeap(sTvBuffer);
        sTvBuffer = NULL;
    }
    if (sDrcBuffer != NULL) {
        MEMFreeToDefaultHeap(sDrcBuffer);
        sDrcBuffer = NULL;
    }
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    WHBProcInit();

    if (!initialise_screens()) {
        WHBProcShutdown();
        return 1;
    }

    AppScreen screen = APP_SCREEN_WARNING;
    int selected_item = 0;
    uint32_t warning_frame = 0;
    uint32_t title_frame = 0;
    uint32_t last_warning_bucket = UINT32_MAX;

    render_warning(255);

    while (WHBProcIsRunning()) {
        VPADStatus status;
        VPADReadError read_error = VPAD_READ_SUCCESS;
        const int32_t samples = VPADRead(VPAD_CHAN_0, &status, 1, &read_error);
        const uint32_t pressed = samples > 0 ? status.trigger : 0u;

        if (screen == APP_SCREEN_WARNING) {
            ++warning_frame;

            if ((pressed & (VPAD_BUTTON_A | VPAD_BUTTON_PLUS)) != 0u ||
                warning_frame >= WARNING_FRAMES) {
                screen = APP_SCREEN_TITLE;
                render_title(selected_item, random_u32());
                title_frame = 0;
            } else {
                const uint32_t fade_start = WARNING_FRAMES - 48u;
                uint8_t intensity = 255u;
                if (warning_frame > fade_start) {
                    const uint32_t remaining = WARNING_FRAMES - warning_frame;
                    intensity = (uint8_t) ((remaining * 255u) / 48u);
                }

                const uint32_t bucket = warning_frame / 4u;
                if (bucket != last_warning_bucket) {
                    render_warning(intensity);
                    last_warning_bucket = bucket;
                }
            }
        } else if (screen == APP_SCREEN_TITLE) {
            bool redraw = false;

            if ((pressed & (VPAD_BUTTON_UP | VPAD_STICK_L_EMULATION_UP)) != 0u) {
                selected_item = (selected_item + 2) % 3;
                redraw = true;
            }
            if ((pressed & (VPAD_BUTTON_DOWN | VPAD_STICK_L_EMULATION_DOWN)) != 0u) {
                selected_item = (selected_item + 1) % 3;
                redraw = true;
            }
            if ((pressed & (VPAD_BUTTON_A | VPAD_BUTTON_PLUS)) != 0u) {
                screen = APP_SCREEN_PLACEHOLDER;
                render_placeholder(selected_item);
                redraw = false;
            }

            ++title_frame;
            if (screen == APP_SCREEN_TITLE &&
                (redraw || title_frame >= GLITCH_INTERVAL)) {
                render_title(selected_item, random_u32());
                title_frame = 0;
            }
        } else if (screen == APP_SCREEN_PLACEHOLDER) {
            if ((pressed & VPAD_BUTTON_B) != 0u) {
                screen = APP_SCREEN_TITLE;
                render_title(selected_item, random_u32());
                title_frame = 0;
            }
        }

        OSSleepTicks(OSMillisecondsToTicks(16));
    }

    shutdown_screens();
    WHBProcShutdown();
    return 0;
}
