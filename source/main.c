#include <whb/proc.h>

#include "game/app.h"
#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"

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
    app_render(&app);

    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);
        app_update(&app, &input);
        app_render(&app);
        frame_clock_wait_next();
    }

    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
