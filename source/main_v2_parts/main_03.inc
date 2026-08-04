    }

    if (game->camera_open && (game->night_frames % 12u) == 0u) {
        game->dirty = true;
    }
}

static void handle_title_input(Game *game, const InputState *input)
{
    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        game->title_selection =
            (game->title_selection + TITLE_ITEM_COUNT - 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        game->title_selection =
            (game->title_selection + 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        if (game->title_selection == 0) {
            reset_night(game, game->unlocked_night);
        } else {
            game->screen = SCREEN_PLACEHOLDER;
            game->screen_frames = 0u;
            game->dirty = true;
        }
    }
}

static void handle_camera_input(Game *game, const InputState *input)
{
    if (input_was_pressed(input, GAME_BUTTON_PANEL) ||
        input_was_pressed(input, GAME_BUTTON_BACK)) {
        close_cameras(game);
        return;
    }
    if (input_was_pressed(input, GAME_BUTTON_SELECT)) {
        close_cameras(game);
        open_maintenance(game);
        return;
    }
    if (input_was_pressed(input, GAME_BUTTON_MODE)) {
        game->vent_map_open = !game->vent_map_open;
        restart_camera_static();
        game->dirty = true;
        return;
    }

    if (game->vent_map_open) {
        int selected = (int) game->selected_vent;
        if (input_was_pressed(input, GAME_BUTTON_LEFT) ||
            input_was_pressed(input, GAME_BUTTON_UP)) {
            selected = (selected + VENT_COUNT - 1) % VENT_COUNT;
        }
        if (input_was_pressed(input, GAME_BUTTON_RIGHT) ||
            input_was_pressed(input, GAME_BUTTON_DOWN)) {
            selected = (selected + 1) % VENT_COUNT;
        }
        if (selected != (int) game->selected_vent) {
            game->selected_vent = (SpringtrapVent) selected;
            restart_camera_static();
            game->dirty = true;
        }
        if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_SEAL)) {
            game->sealed_vent = game->sealed_vent == game->selected_vent
                ? SPRINGTRAP_VENT_NONE
                : game->selected_vent;
            audio_play(AUDIO_CUE_WAIT, 0.6f, false);
            set_message(game, MESSAGE_VENT_SEALED);
        }
        return;
    }

    int next_camera = game->selected_camera;
    if (input_was_pressed(input, GAME_BUTTON_LEFT)) {
        next_camera = camera_in_direction(next_camera, -1, 0);
    }
    if (input_was_pressed(input, GAME_BUTTON_RIGHT)) {
        next_camera = camera_in_direction(next_camera, 1, 0);
    }
    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        next_camera = camera_in_direction(next_camera, 0, -1);
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        next_camera = camera_in_direction(next_camera, 0, 1);
    }
    if (next_camera != game->selected_camera) {
        game->selected_camera = next_camera;
        restart_camera_static();
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM)) {
        trigger_audio_lure(game);
    }
}

static void handle_maintenance_input(Game *game, const InputState *input)
{
    if (game->repairing) {
        return;
    }
    if (input_was_pressed(input, GAME_BUTTON_BACK) ||
        input_was_pressed(input, GAME_BUTTON_SELECT)) {
        game->maintenance_open = false;
        game->dirty = true;
        return;
    }
    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        game->maintenance_selection =
            (game->maintenance_selection + MAINTENANCE_ITEM_COUNT - 1) %
            MAINTENANCE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        game->maintenance_selection =
            (game->maintenance_selection + 1) % MAINTENANCE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        start_repair(game);
    }
}

static void handle_office_input(Game *game, const InputState *input)
{
    if (game->camera_open) {
        handle_camera_input(game, input);
        return;
    }
    if (game->maintenance_open) {
        handle_maintenance_input(game, input);
        return;
    }

    if (input_was_pressed(input, GAME_BUTTON_BACK)) {
        stop_panel_audio();
        stop_ventilation_audio(game);
        game->screen = SCREEN_TITLE;
        game->screen_frames = 0u;
        game->dirty = true;
        return;
    }
    if (input_was_pressed(input, GAME_BUTTON_PANEL)) {
        open_cameras(game);
        return;
    }
    if (input_was_pressed(input, GAME_BUTTON_SELECT)) {
        open_maintenance(game);
        return;
    }

    int pan = game->office_pan;
    if (input_is_held(input, GAME_BUTTON_LEFT)) {
        pan -= 5;
    }
    if (input_is_held(input, GAME_BUTTON_RIGHT)) {
        pan += 5;
    }
    if (pan < -140) {
        pan = -140;
    }
    if (pan > 140) {
        pan = 140;
    }
    if (pan != game->office_pan) {
        game->office_pan = pan;
        game->dirty = true;
    }

    /* Development shortcut: hold + and press Right to advance one hour. */
    if (input_is_held(input, GAME_BUTTON_START) &&
        input_was_pressed(input, GAME_BUTTON_RIGHT)) {
        advance_hour(game);
    }
}

static void update_game(Game *game, const InputState *input)
{
    ++game->screen_frames;

    switch (game->screen) {
        case SCREEN_WARNING:
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
                input_was_pressed(input, GAME_BUTTON_START) ||
                game->screen_frames >= WARNING_FRAMES) {
                game->screen = SCREEN_TITLE;
                game->screen_frames = 0u;
                game->dirty = true;
            } else if ((game->screen_frames % 6u) == 0u) {
                game->dirty = true;
            }
            break;

        case SCREEN_TITLE: {
            const int previous_frame =
                (int) ((game->screen_frames - 1u) % 180u);
            handle_title_input(game, input);
            if ((game->screen_frames % 180u) != (uint32_t) previous_frame &&
                (game->screen_frames % 9u) == 0u) {
                game->dirty = true;
            }
            if ((game->screen_frames % 12u) == 0u) {
                game->dirty = true;
            }
            break;
        }

        case SCREEN_NIGHT_INTRO:
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
                input_was_pressed(input, GAME_BUTTON_START) ||
                game->screen_frames >= NIGHT_INTRO_FRAMES) {
                game->screen = SCREEN_OFFICE;
                game->screen_frames = 0u;
                game->dirty = true;
            }
            break;

        case SCREEN_OFFICE:
            handle_office_input(game, input);
            if (game->screen == SCREEN_OFFICE) {
                update_active_game(game, input);
            }
            break;

        case SCREEN_VICTORY:
            if ((game->screen_frames % 6u) == 0u) {
