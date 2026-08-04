#include <stdbool.h>

#include <whb/proc.h>

#include "assets/warning_texture.h"
#include "game/app.h"
#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"
#include "renderer/texture.h"

static void render_app(App *app)
{
    const bool draw_warning_texture =
        app->needs_redraw && app->screen == APP_SCREEN_WARNING;

    app_render(app);

    if (draw_warning_texture) {
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
        graphics_present(GRAPHICS_TARGET_BOTH);
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
