#include <stdbool.h>

#include <whb/proc.h>

#include "assets/menu_springtrap_texture.h"
#include "assets/office_texture.h"
#include "assets/warning_texture.h"
#include "game/app.h"
#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"
#include "renderer/texture.h"

static void draw_warning_texture(void)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       60,
                       154,
                       734,
                       174,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     67,
                     174,
                     720,
                     138,
                     &gWarningTexture);
}

static void draw_title_texture(const App *app)
{
    const int glitch_x = (int) ((app->effect_seed >> 10) % 7u) - 3;

    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       410,
                       0,
                       444,
                       GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     410 + glitch_x,
                     16,
                     444,
                     448,
                     &gMenuSpringtrapTexture);

    if ((app->effect_seed & 0x03u) == 0u) {
        const int glitch_y = 90 + (int) ((app->effect_seed >> 8) % 300u);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           410,
                           glitch_y,
                           444,
                           3,
                           GRAPHICS_RGB(36, 92, 40));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           448,
                           glitch_y + 7,
                           361,
                           1,
                           GRAPHICS_RGB(105, 153, 84));
    }
}

static void draw_office_texture(const App *app)
{
    const int texture_x = -133 + app->office_pan;

    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       0,
                       0,
                       GRAPHICS_LOGICAL_WIDTH,
                       GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_TV,
                     texture_x,
                     24,
                     1120,
                     430,
                     &gOfficeTexture);

    for (int y = 30; y < 454; y += 12) {
        graphics_draw_rect(GRAPHICS_TARGET_TV,
                           0,
                           y,
                           GRAPHICS_LOGICAL_WIDTH,
                           1,
                           GRAPHICS_RGB(4, 8, 5));
    }

    graphics_draw_text(GRAPHICS_TARGET_TV,
                       18,
                       5,
                       2,
                       "NIGHT 1 - OFFICE TEST",
                       GRAPHICS_RGB(166, 191, 151));
    graphics_draw_text(GRAPHICS_TARGET_TV,
                       18,
                       458,
                       1,
                       "LEFT RIGHT: LOOK   X/Y: CAMERAS   B: MENU",
                       GRAPHICS_RGB(119, 150, 111));
}

static void render_app(App *app)
{
    const bool redraw = app->needs_redraw;
    const AppScreen screen = app->screen;

    app_render(app);

    if (!redraw) {
        return;
    }

    if (screen == APP_SCREEN_WARNING) {
        draw_warning_texture();
        graphics_present(GRAPHICS_TARGET_BOTH);
    } else if (screen == APP_SCREEN_TITLE) {
        draw_title_texture(app);
        graphics_present(GRAPHICS_TARGET_BOTH);
    } else if (screen == APP_SCREEN_OFFICE_TEST) {
        draw_office_texture(app);
        graphics_present(GRAPHICS_TARGET_TV);
    }
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    WHBProcInit();

    if (!graphics_init()) {
        WHBProcShutdown();
        return 1;
    }

    input_init();
    frame_clock_reset();

    App app;
    app_init(&app);
    render_app(&app);

    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);
        app_update(&app, &input);
        render_app(&app);
        frame_clock_wait_next();
    }

    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
