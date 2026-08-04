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
                       GRAPHICS_RGB(1, 4, 2));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       28, 20, 3,
                       "CAMERA SYSTEM",
                       GRAPHICS_RGB(167, 205, 157));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        35, 61, 784, 326, 4,
                        GRAPHICS_RGB(61, 112, 62));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       47, 73, 760, 300,
                       GRAPHICS_RGB(0, 0, 0));
    const TextureRle *feed = selected_camera == systems->springtrap_camera
        ? camera_springtrap_texture(selected_camera)
        : camera_texture(selected_camera);
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     57, 79, 740, 292,
                     feed);

    for (int y = 82; y < 371; y += 10) {
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           57, y, 740, 1,
                           GRAPHICS_RGB(5, 10, 6));
    }
    for (int bar = 0; bar < 4; ++bar) {
        const uint32_t shifted = seed >> (bar * 5);
        const int y = 86 + (int) (shifted % 270u);
        const int x = 57 + (int) ((shifted >> 8) % 130u);
        const int width = 390 + (int) ((shifted >> 16) % 220u);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           x, y, width, 1 + (bar & 1),
                           GRAPHICS_RGB(25, 61, 30));
    }

    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       55, 399, 3,
                       camera_label(selected_camera),
                       GRAPHICS_RGB(216, 239, 209));
    if (selected_camera == systems->springtrap_camera) {
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           214, 405, 2,
                           "MOTION DETECTED",
                           GRAPHICS_RGB(232, 104, 67));
    }
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const int x = 594 + camera * 70;
        const uint32_t colour = camera == selected_camera
            ? GRAPHICS_RGB(151, 210, 143)
            : GRAPHICS_RGB(45, 78, 47);
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            x, 397, 54, 30, 3, colour);
    }
    const char *lure_status = "A: PLAY AUDIO";
    uint32_t lure_colour = GRAPHICS_RGB(137, 190, 132);
    switch (systems->audio_lure_message) {
        case AUDIO_LURE_MESSAGE_PLAYING:
            lure_status = "PLAYING AUDIO...";
            lure_colour = GRAPHICS_RGB(211, 221, 135);
            break;
        case AUDIO_LURE_MESSAGE_SUCCESS:
            lure_status = "AUDIO LURE SUCCESS";
            lure_colour = GRAPHICS_RGB(119, 218, 112);
            break;
        case AUDIO_LURE_MESSAGE_ERROR:
            lure_status = "AUDIO DEVICES ERROR";
            lure_colour = GRAPHICS_RGB(237, 75, 52);
            break;
        case AUDIO_LURE_MESSAGE_COOLDOWN:
            lure_status = "AUDIO RECHARGING";
            lure_colour = GRAPHICS_RGB(173, 157, 101);
            break;
        case AUDIO_LURE_MESSAGE_NONE:
        default:
            if (systems->audio_lure_cooldown_frames > 0u) {
                lure_status = "AUDIO RECHARGING";
                lure_colour = GRAPHICS_RGB(173, 157, 101);
            }
            break;
    }
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       55, 435, 2,
                       lure_status,
                       lure_colour);
