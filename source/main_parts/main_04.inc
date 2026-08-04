    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       55, 460, 1,
                       "LEFT RIGHT: CAMERA   A: LURE   X/Y: CLOSE",
                       GRAPHICS_RGB(130, 174, 126));
}

static bool system_failed(const SystemsState *systems, int item)
{
    switch (item) {
        case MAINTENANCE_CAMERA:
            return systems->camera_failed;
        case MAINTENANCE_AUDIO:
            return systems->audio_failed;
        case MAINTENANCE_VENTILATION:
            return systems->ventilation_failed;
        default:
            return false;
    }
}

static void draw_maintenance_panel(const SystemsState *systems)
{
    static const char *const labels[MAINTENANCE_ITEM_COUNT] = {
        "CAMERA SYSTEM",
        "AUDIO DEVICES",
        "VENTILATION",
        "REBOOT ALL",
        "EXIT",
    };

    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(1, 5, 3));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        38, 31, 778, 412, 4,
                        GRAPHICS_RGB(54, 112, 59));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       81, 58, 4,
                       "MAINTENANCE PANEL",
                       GRAPHICS_RGB(174, 220, 166));

    for (int item = 0; item < MAINTENANCE_ITEM_COUNT; ++item) {
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
        graphics_present(GRAPHICS_TARGET_BOTH);
    } else if (screen == APP_SCREEN_OFFICE_TEST) {
        draw_office_texture(app, systems);
        if (systems->maintenance_open) {
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
