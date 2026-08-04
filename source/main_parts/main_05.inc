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
    systems_reset(&systems);
    render_app(&app, &systems, selected_camera);

    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);

        const AppScreen previous_screen = app.screen;
        const bool camera_was_open = app.camera_panel_open;
        const int previous_title_frame =
            app.screen == APP_SCREEN_TITLE ? title_frame_index(&app) : 0;

        InputState game_input = input;

        bool maintenance_toggled = false;
        if (app.screen == APP_SCREEN_OFFICE_TEST &&
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
        }

        app_update(&app, &game_input);

        if (app.screen == APP_SCREEN_TITLE &&
            previous_title_frame != title_frame_index(&app)) {
            app.needs_redraw = true;
        }

        if (previous_screen != APP_SCREEN_OFFICE_TEST &&
            app.screen == APP_SCREEN_OFFICE_TEST) {
            selected_camera = 0;
            systems_reset(&systems);
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
