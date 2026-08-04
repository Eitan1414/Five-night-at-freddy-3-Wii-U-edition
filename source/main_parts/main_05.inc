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
    SystemsState systems;
    int selected_camera = 0;
    app_init(&app);
    systems_reset(&systems, g_title_night);
    render_app(&app, &systems, selected_camera);

    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);

        const AppScreen previous_screen = app.screen;
        const bool camera_was_open = app.camera_panel_open;
        const int previous_title_frame =
            app.screen == APP_SCREEN_TITLE ? title_frame_index(&app) : 0;

        InputState game_input = input;

        if (app.screen == APP_SCREEN_OFFICE_TEST &&
            systems.night_phase != NIGHT_PHASE_ACTIVE) {
            if (systems.night_phase == NIGHT_PHASE_INTRO &&
                (input_was_pressed(&input, GAME_BUTTON_CONFIRM) ||
                 input_was_pressed(&input, GAME_BUTTON_START))) {
                systems.night_phase = NIGHT_PHASE_ACTIVE;
                systems.phase_frames = 0u;
                app.needs_redraw = true;
            } else if (systems.night_phase == NIGHT_PHASE_VICTORY &&
                       systems.phase_frames >= VICTORY_MIN_FRAMES &&
                       (input_was_pressed(&input, GAME_BUTTON_CONFIRM) ||
                        input_was_pressed(&input, GAME_BUTTON_START) ||
                        systems.phase_frames >= VICTORY_AUTO_FRAMES)) {
                if (systems.night_number < NIGHT_COUNT) {
                    g_title_night = systems.night_number + 1;
                    selected_camera = 0;
                    app.office_pan = 0;
                    app.camera_panel_open = false;
                    systems_reset(&systems, g_title_night);
                    app.needs_redraw = true;
                } else {
                    g_title_night = NIGHT_COUNT;
                    app.screen = APP_SCREEN_TITLE;
                    app.state_frames = 0u;
                    app.camera_panel_open = false;
                    app.needs_redraw = true;
                }
            } else if (systems.night_phase == NIGHT_PHASE_GAME_OVER) {
                if (input_was_pressed(&input, GAME_BUTTON_CONFIRM) ||
                    input_was_pressed(&input, GAME_BUTTON_START)) {
                    selected_camera = 0;
                    app.office_pan = 0;
                    app.camera_panel_open = false;
                    systems_reset(&systems, systems.night_number);
                    app.needs_redraw = true;
                } else if (input_was_pressed(&input, GAME_BUTTON_BACK)) {
                    g_title_night = systems.night_number;
                    app.screen = APP_SCREEN_TITLE;
                    app.state_frames = 0u;
                    app.camera_panel_open = false;
                    app.needs_redraw = true;
                }
            }
            game_input.held = 0u;
            game_input.pressed = 0u;
            game_input.released = 0u;
        }

        bool maintenance_toggled = false;
        if (app.screen == APP_SCREEN_OFFICE_TEST &&
            systems.night_phase == NIGHT_PHASE_ACTIVE &&
            !systems.repairing &&
            input_was_pressed(&input, GAME_BUTTON_SELECT)) {
            systems.maintenance_open = !systems.maintenance_open;
            systems.maintenance_selection = 0;
            app.camera_panel_open = false;
            app.needs_redraw = true;
            maintenance_toggled = true;
        }

        if (app.screen == APP_SCREEN_OFFICE_TEST &&
            systems.maintenance_open) {
            if (!maintenance_toggled) {
                handle_maintenance_input(&systems, &app, &input);
            }
            game_input.held = 0u;
            game_input.pressed = 0u;
            game_input.released = 0u;
        } else if (app.screen == APP_SCREEN_OFFICE_TEST &&
                   systems.night_phase == NIGHT_PHASE_ACTIVE &&
                   app.camera_panel_open) {
            if (input_was_pressed(&input, GAME_BUTTON_LEFT) ||
                input_was_pressed(&input, GAME_BUTTON_UP)) {
                selected_camera =
                    (selected_camera + CAMERA_COUNT - 1) % CAMERA_COUNT;
                app.needs_redraw = true;
            }
            if (input_was_pressed(&input, GAME_BUTTON_RIGHT) ||
                input_was_pressed(&input, GAME_BUTTON_DOWN)) {
                selected_camera = (selected_camera + 1) % CAMERA_COUNT;
                app.needs_redraw = true;
            }
            if (input_was_pressed(&input, GAME_BUTTON_CONFIRM)) {
                trigger_audio_lure(&systems, &app, selected_camera);
            }
        }

        if (app.screen == APP_SCREEN_OFFICE_TEST &&
            systems.night_phase == NIGHT_PHASE_ACTIVE &&
            !systems.maintenance_open && !app.camera_panel_open &&
            input_is_held(&input, GAME_BUTTON_START) &&
            input_was_pressed(&input, GAME_BUTTON_RIGHT)) {
            advance_hour(&systems, &app);
            game_input.held = 0u;
            game_input.pressed = 0u;
            game_input.released = 0u;
        }

        app_update(&app, &game_input);

        if (app.screen == APP_SCREEN_TITLE &&
            previous_title_frame != title_frame_index(&app)) {
            app.needs_redraw = true;
        }

        if (previous_screen != APP_SCREEN_OFFICE_TEST &&
            app.screen == APP_SCREEN_OFFICE_TEST) {
            selected_camera = 0;
            systems_reset(&systems, g_title_night);
        }
        if (previous_screen == APP_SCREEN_OFFICE_TEST &&
            app.screen != APP_SCREEN_OFFICE_TEST) {
            systems.maintenance_open = false;
            systems.repairing = false;
        }
        if (!camera_was_open && app.camera_panel_open) {
            selected_camera = 0;
            app.needs_redraw = true;
        }

        update_systems(&systems, &app);
        render_app(&app, &systems, selected_camera);
        frame_clock_wait_next();
    }

    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
