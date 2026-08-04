
    if (systems->audio_lure_cooldown_frames > 0u) {
        --systems->audio_lure_cooldown_frames;
    }
    if (systems->audio_lure_message_frames > 0u) {
        --systems->audio_lure_message_frames;
        if (systems->audio_lure_message_frames == 0u) {
            systems->audio_lure_message = AUDIO_LURE_MESSAGE_NONE;
            app->needs_redraw = true;
        }
    }

    if (systems->audio_lure_active) {
        ++systems->audio_lure_frames;
        if (systems->audio_lure_frames >= AUDIO_LURE_TRAVEL_FRAMES) {
            systems->springtrap_camera = systems->audio_lure_target;
            systems->springtrap_move_frames = 0u;
            systems->audio_lure_active = false;
            systems->audio_lure_frames = 0u;
            set_audio_lure_message(systems, AUDIO_LURE_MESSAGE_SUCCESS);
            app->needs_redraw = true;
        }
    } else {
        ++systems->springtrap_move_frames;
        if (systems->springtrap_move_frames >=
            springtrap_move_interval(systems)) {
            systems->springtrap_move_frames = 0u;
            systems->springtrap_camera =
                (systems->springtrap_camera + CAMERA_COUNT - 1) % CAMERA_COUNT;
            app->needs_redraw = true;
        }
    }

    if (systems->repairing) {
        ++systems->repair_frames;
        app->needs_redraw = true;
        if (systems->repair_frames >= repair_duration(systems)) {
            finish_repair(systems);
        }
    }

    if (systems->maintenance_open || systems->ventilation_failed ||
        systems->camera_failed || systems->audio_lure_active) {
        if ((systems->night_frames % 12u) == 0u) {
            app->needs_redraw = true;
        }
    }
}

static void draw_warning_texture(void)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       60, 154, 734, 174,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     67, 174, 720, 138,
                     &gWarningTexture);
}

static void draw_title_texture(const App *app)
{
    const int frame = title_frame_index(app);
    const int glitch_x = frame == 0
        ? (int) ((app->effect_seed >> 10) % 7u) - 3
        : (frame - 2) * 3;

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

    if (systems->springtrap_camera == 0) {
        const uint32_t pulse = (systems->night_frames / 18u) & 1u;
        const uint32_t warning = pulse
            ? GRAPHICS_RGB(212, 154, 70)
            : GRAPHICS_RGB(105, 71, 31);
        graphics_draw_rect(GRAPHICS_TARGET_TV,
                           545, 58, 291, 34,
                           GRAPHICS_RGB(13, 7, 2));
        graphics_draw_frame(GRAPHICS_TARGET_TV,
                            545, 58, 291, 34, 2, warning);
        graphics_draw_text(GRAPHICS_TARGET_TV,
                           569, 69, 2,
                           "MOVEMENT NEAR OFFICE",
                           warning);
    }

    if (systems->ventilation_failed) {
        const uint32_t pulse = (systems->night_frames / 12u) & 1u;
        const uint32_t warning = pulse
            ? GRAPHICS_RGB(225, 56, 39)
            : GRAPHICS_RGB(120, 18, 12);
        graphics_draw_rect(GRAPHICS_TARGET_TV,
                           568, 12, 268, 38,
                           GRAPHICS_RGB(20, 0, 0));
