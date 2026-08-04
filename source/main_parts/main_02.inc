
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       410, 0, 444, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     410 + glitch_x, 16, 444, 448,
                     title_texture(frame));

    if (frame != 0 || (app->effect_seed & 0x03u) == 0u) {
        const int glitch_y = 90 +
            (int) ((app->effect_seed + (uint32_t) frame * 43u) % 300u);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           410, glitch_y, 444, 3,
                           GRAPHICS_RGB(36, 92, 40));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           448, glitch_y + 7, 361, 1,
                           GRAPHICS_RGB(105, 153, 84));
    }
}

static void draw_office_texture(const App *app, const SystemsState *systems)
{
    const int texture_x = -133 + app->office_pan;
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_TV,
                     texture_x, 24, 1120, 430,
                     &gOfficeTexture);

    for (int y = 30; y < 454; y += 12) {
        graphics_draw_rect(GRAPHICS_TARGET_TV,
                           0, y, GRAPHICS_LOGICAL_WIDTH, 1,
                           GRAPHICS_RGB(4, 8, 5));
    }

    graphics_draw_text(GRAPHICS_TARGET_TV,
                       18, 5, 2,
                       "NIGHT 1 - OFFICE TEST",
                       GRAPHICS_RGB(166, 191, 151));
    graphics_draw_text(GRAPHICS_TARGET_TV,
                       18, 458, 1,
                       "LEFT RIGHT: LOOK   X/Y: CAMERAS   MINUS: MAINTENANCE",
                       GRAPHICS_RGB(119, 150, 111));

    if (systems->ventilation_failed) {
        const uint32_t pulse = (systems->night_frames / 12u) & 1u;
        const uint32_t warning = pulse
            ? GRAPHICS_RGB(225, 56, 39)
            : GRAPHICS_RGB(120, 18, 12);
        graphics_draw_rect(GRAPHICS_TARGET_TV,
                           568, 12, 268, 38,
                           GRAPHICS_RGB(20, 0, 0));
        graphics_draw_frame(GRAPHICS_TARGET_TV,
                            568, 12, 268, 38, 3, warning);
        graphics_draw_text(GRAPHICS_TARGET_TV,
                           589, 24, 2,
                           "VENTILATION ERROR",
                           warning);
    }
}

static void draw_camera_error(const SystemsState *systems)
{
    const uint32_t pulse = (systems->night_frames / 12u) & 1u;
    const uint32_t warning = pulse
        ? GRAPHICS_RGB(240, 63, 43)
        : GRAPHICS_RGB(129, 22, 15);

    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(2, 0, 0));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        58, 67, 738, 306, 5, warning);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       123, 132, 5,
                       "CAMERA SYSTEM ERROR",
                       warning);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       191, 232, 3,
                       "SIGNAL LOST",
                       GRAPHICS_RGB(203, 171, 157));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       139, 300, 2,
                       "PRESS MINUS TO OPEN MAINTENANCE",
                       GRAPHICS_RGB(176, 118, 105));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       246, 443, 1,
                       "X/Y: CLOSE CAMERAS",
                       GRAPHICS_RGB(122, 82, 74));
}

static void draw_camera_feed(const App *app,
                             const SystemsState *systems,
                             int selected_camera)
{
    if (systems->camera_failed) {
        draw_camera_error(systems);
        return;
    }

    const uint32_t seed = app->effect_seed +
        (uint32_t) selected_camera * 0x1021u + systems->night_frames;
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
