#include <stdbool.h>
#include <stdint.h>
#include <whb/proc.h>
#include "assets/camera01_texture.h"
#include "assets/camera02_texture.h"
#include "assets/camera03_texture.h"
#include "assets/menu_springtrap_texture.h"
#include "assets/office_texture.h"
#include "assets/warning_texture.h"
#include "game/app.h"
#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"
#include "renderer/texture.h"

#define CAMERA_COUNT 3

static const TextureRle *camera_texture(int camera_index)
{
    static const TextureRle *const textures[CAMERA_COUNT] = {
        &gCamera01Texture,
        &gCamera02Texture,
        &gCamera03Texture,
    };
    if (camera_index < 0 || camera_index >= CAMERA_COUNT) {
        camera_index = 0;
    }
    return textures[camera_index];
}

static const char *camera_label(int camera_index)
{
    static const char *const labels[CAMERA_COUNT] = {
        "CAM 01",
        "CAM 02",
        "CAM 03",
    };
    if (camera_index < 0 || camera_index >= CAMERA_COUNT) {
        camera_index = 0;
    }
    return labels[camera_index];
}

static void draw_warning_texture(void)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 60, 154, 734, 174, GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 67, 174, 720, 138, &gWarningTexture);
}

static void draw_title_texture(const App *app)
{
    const int glitch_x = (int) ((app->effect_seed >> 10) % 7u) - 3;
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 410, 0, 444, GRAPHICS_LOGICAL_HEIGHT, GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 410 + glitch_x, 16, 444, 448, &gMenuSpringtrapTexture);
    if ((app->effect_seed & 0x03u) == 0u) {
        const int glitch_y = 90 + (int) ((app->effect_seed >> 8) % 300u);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 410, glitch_y, 444, 3, GRAPHICS_RGB(36, 92, 40));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 448, glitch_y + 7, 361, 1, GRAPHICS_RGB(105, 153, 84));
    }
}

static void draw_office_texture(const App *app)
{
    const int texture_x = -133 + app->office_pan;
    graphics_draw_rect(GRAPHICS_TARGET_TV, 0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT, GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_TV, texture_x, 24, 1120, 430, &gOfficeTexture);
    for (int y = 30; y < 454; y += 12) {
        graphics_draw_rect(GRAPHICS_TARGET_TV, 0, y, GRAPHICS_LOGICAL_WIDTH, 1, GRAPHICS_RGB(4, 8, 5));
    }
    graphics_draw_text(GRAPHICS_TARGET_TV, 18, 5, 2, "NIGHT 1 - OFFICE TEST", GRAPHICS_RGB(166, 191, 151));
    graphics_draw_text(GRAPHICS_TARGET_TV, 18, 458, 1, "LEFT RIGHT: LOOK   X/Y: CAMERAS   B: MENU", GRAPHICS_RGB(119, 150, 111));
}

static void draw_camera_feed(const App *app, int selected_camera)
{
    const uint32_t seed = app->effect_seed + (uint32_t) selected_camera * 0x1021u;
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 28, 20, 3, "CAMERA SYSTEM", GRAPHICS_RGB(167, 205, 157));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 35, 61, 784, 326, 4, GRAPHICS_RGB(61, 112, 62));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 47, 73, 760, 300, GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 57, 79, 740, 292, camera_texture(selected_camera));
    for (int y = 82; y < 371; y += 10) {
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 57, y, 740, 1, GRAPHICS_RGB(5, 10, 6));
    }
    for (int bar = 0; bar < 4; ++bar) {
        const uint32_t shifted = seed >> (bar * 5);
        const int y = 86 + (int) (shifted % 270u);
        const int x = 57 + (int) ((shifted >> 8) % 130u);
        const int width = 390 + (int) ((shifted >> 16) % 220u);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, x, y, width, 1 + (bar & 1), GRAPHICS_RGB(25, 61, 30));
    }
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 55, 399, 3, camera_label(selected_camera), GRAPHICS_RGB(216, 239, 209));
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const int x = 594 + camera * 70;
        const uint32_t colour = camera == selected_camera ? GRAPHICS_RGB(151, 210, 143) : GRAPHICS_RGB(45, 78, 47);
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, x, 397, 54, 30, 3, colour);
    }
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 55, 449, 1, "LEFT RIGHT: SWITCH CAMERA   X/Y: CLOSE", GRAPHICS_RGB(130, 174, 126));
}

static void render_app(App *app, int selected_camera)
{
    const bool redraw = app->needs_redraw;
    const AppScreen screen = app->screen;
    app_render(app);
    if (!redraw) return;
    if (screen == APP_SCREEN_WARNING) {
        draw_warning_texture();
        graphics_present(GRAPHICS_TARGET_BOTH);
    } else if (screen == APP_SCREEN_TITLE) {
        draw_title_texture(app);
        graphics_present(GRAPHICS_TARGET_BOTH);
    } else if (screen == APP_SCREEN_OFFICE_TEST) {
        draw_office_texture(app);
        if (app->camera_panel_open) {
            draw_camera_feed(app, selected_camera);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else {
            graphics_present(GRAPHICS_TARGET_TV);
        }
    }
}

int main(int argc, char **argv)
{
    (void) argc; (void) argv;
    WHBProcInit();
    if (!graphics_init()) { WHBProcShutdown(); return 1; }
    input_init(); frame_clock_reset();
    App app; int selected_camera = 0;
    app_init(&app); render_app(&app, selected_camera);
    while (WHBProcIsRunning()) {
        InputState input; input_update(&input);
        const AppScreen previous_screen = app.screen;
        const bool panel_was_open = app.camera_panel_open;
        if (app.screen == APP_SCREEN_OFFICE_TEST && app.camera_panel_open) {
            if (input_was_pressed(&input, GAME_BUTTON_LEFT) || input_was_pressed(&input, GAME_BUTTON_UP)) {
                selected_camera = (selected_camera + CAMERA_COUNT - 1) % CAMERA_COUNT;
                app.needs_redraw = true;
            }
            if (input_was_pressed(&input, GAME_BUTTON_RIGHT) || input_was_pressed(&input, GAME_BUTTON_DOWN)) {
                selected_camera = (selected_camera + 1) % CAMERA_COUNT;
                app.needs_redraw = true;
            }
        }
        app_update(&app, &input);
        if (previous_screen != APP_SCREEN_OFFICE_TEST && app.screen == APP_SCREEN_OFFICE_TEST) selected_camera = 0;
        if (!panel_was_open && app.camera_panel_open) { selected_camera = 0; app.needs_redraw = true; }
        render_app(&app, selected_camera);
        frame_clock_wait_next();
    }
    graphics_shutdown(); WHBProcShutdown(); return 0;
}
