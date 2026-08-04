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
    systems->repair_frames = 0u;
}

static void handle_maintenance_input(SystemsState *systems,
                                     App *app,
                                     const InputState *input)
{
    if (systems->repairing) {
        return;
    }

    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        systems->maintenance_selection =
            (systems->maintenance_selection + MAINTENANCE_ITEM_COUNT - 1) %
            MAINTENANCE_ITEM_COUNT;
        app->needs_redraw = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        systems->maintenance_selection =
            (systems->maintenance_selection + 1) % MAINTENANCE_ITEM_COUNT;
        app->needs_redraw = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        begin_repair(systems);
        app->needs_redraw = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_BACK) ||
        input_was_pressed(input, GAME_BUTTON_SELECT)) {
        systems->maintenance_open = false;
        app->needs_redraw = true;
    }
}

int main(int argc, char **argv)
