        const int y = 126 + item * 55;
        const bool selected = item == systems->maintenance_selection;
        if (selected) {
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               76, y - 12, 701, 40,
                               GRAPHICS_RGB(24, 60, 29));
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               76, y - 12, 6, 40,
                               GRAPHICS_RGB(131, 211, 124));
        }

        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           101, y, 3, labels[item],
                           selected
                               ? GRAPHICS_RGB(222, 242, 216)
                               : GRAPHICS_RGB(111, 148, 108));

        if (item < MAINTENANCE_REBOOT_ALL) {
            const bool failed = system_failed(systems, item);
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               602, y, 2,
                               failed ? "ERROR" : "ONLINE",
                               failed
                                   ? GRAPHICS_RGB(237, 69, 47)
                                   : GRAPHICS_RGB(112, 199, 108));
        }
    }

    if (systems->repairing) {
        const uint32_t duration = repair_duration(systems);
        const int progress = (int) ((systems->repair_frames * 626u) / duration);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           103, 406, 626, 13,
                           GRAPHICS_RGB(9, 20, 11));
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           103, 406, progress, 13,
                           GRAPHICS_RGB(89, 184, 85));
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            100, 403, 632, 19, 2,
                            GRAPHICS_RGB(118, 180, 112));
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           303, 375, 2,
                           "REBOOTING...",
                           GRAPHICS_RGB(176, 219, 169));
    } else {
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           103, 407, 1,
                           "UP DOWN: SELECT   A: REBOOT   B/MINUS: CLOSE",
                           GRAPHICS_RGB(110, 151, 107));
    }
}

static void draw_night_intro(const SystemsState *systems)
{
    const NightConfig *config = night_config(systems->night_number);
    char night_text[24];
    snprintf(night_text, sizeof(night_text), "NIGHT %d", systems->night_number);

    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(0, 0, 0));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       286, 139, 7, night_text,
                       GRAPHICS_RGB(220, 235, 215));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       329, 239, 4, "12:00 AM",
                       GRAPHICS_RGB(137, 181, 131));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       245, 315, 2, config->subtitle,
                       GRAPHICS_RGB(84, 124, 82));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       306, 397, 1, "A: SKIP INTRO",
                       GRAPHICS_RGB(72, 102, 71));
}

static void draw_victory(const SystemsState *systems)
{
    char complete_text[32];
    snprintf(complete_text, sizeof(complete_text),
             "NIGHT %d COMPLETE", systems->night_number);
    const bool six_am = systems->phase_frames >= 90u;
    const uint32_t pulse = ((systems->phase_frames / 6u) & 1u)
        ? GRAPHICS_RGB(224, 222, 93)
        : GRAPHICS_RGB(80, 176, 79);

    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(0, 0, 0));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       six_am ? 301 : 275, 145, 8,
                       six_am ? "6:00" : "5:59",
                       six_am ? pulse : GRAPHICS_RGB(231, 231, 231));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       577, 193, 4, "AM",
                       GRAPHICS_RGB(186, 208, 180));
    if (systems->phase_frames >= VICTORY_MIN_FRAMES) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           248, 305, 3, complete_text,
                           GRAPHICS_RGB(139, 204, 132));
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           236, 379, 2,
                           systems->night_number < NIGHT_COUNT
                               ? "A: START NEXT NIGHT"
                               : "A: RETURN TO TITLE",
                           GRAPHICS_RGB(104, 151, 101));
    }
}

static void draw_game_over(const SystemsState *systems)
{
    const uint32_t pulse = ((systems->phase_frames / 5u) & 1u)
        ? GRAPHICS_RGB(223, 46, 32)
        : GRAPHICS_RGB(93, 10, 7);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(4, 0, 0));
    for (int y = 0; y < GRAPHICS_LOGICAL_HEIGHT; y += 9) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           0, y, GRAPHICS_LOGICAL_WIDTH, 2, pulse);
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       183, 135, 7, "GAME OVER", pulse);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       227, 269, 3, "SPRINGTRAP GOT YOU",
                       GRAPHICS_RGB(213, 170, 155));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       257, 370, 2, "A: RETRY   B: MENU",
                       GRAPHICS_RGB(169, 118, 108));
}

static void render_app(App *app,
                       const SystemsState *systems,
                       int selected_camera)
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
        if (g_title_night > 1) {
            char continue_text[32];
            snprintf(continue_text, sizeof(continue_text),
                     "CONTINUE: NIGHT %d", g_title_night);
            graphics_draw_text(GRAPHICS_TARGET_BOTH,
                               126, 253, 1, continue_text,
                               GRAPHICS_RGB(105, 151, 101));
        }
        graphics_present(GRAPHICS_TARGET_BOTH);
    } else if (screen == APP_SCREEN_OFFICE_TEST) {
        draw_office_texture(app, systems);
        if (systems->night_phase == NIGHT_PHASE_INTRO) {
            draw_night_intro(systems);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else if (systems->night_phase == NIGHT_PHASE_VICTORY) {
            draw_victory(systems);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else if (systems->night_phase == NIGHT_PHASE_GAME_OVER) {
            draw_game_over(systems);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else if (systems->maintenance_open) {
            draw_maintenance_panel(systems);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else if (app->camera_panel_open) {
            draw_camera_feed(app, systems, selected_camera);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else {
            graphics_present(GRAPHICS_TARGET_TV);
        }
    }
}

static void begin_repair(SystemsState *systems)
{
    if (systems->maintenance_selection == MAINTENANCE_EXIT) {
        systems->maintenance_open = false;
        return;
    }
    systems->repairing = true;
    systems->repair_target = systems->maintenance_selection;
    systems->repair_frames = 0u;
}

static void handle_maintenance_input(SystemsState *systems,
