static void open_maintenance(Game *game)
{
    game->maintenance_open = true;
    game->camera_open = false;
    game->vent_map_open = false;
    game->maintenance_selection = 0;
    game->idle_frames = 0u;
    process_phantom_event(game, phantoms_on_panel_opened(&game->phantoms));
    audio_stop(AUDIO_CUE_STATIC);
    audio_play(AUDIO_CUE_WAIT, 0.45f, false);
    game->dirty = true;
}

static bool office_activity(const Game *game, const InputState *input)
{
    if (game->camera_open || game->maintenance_open ||
        phantoms_blocks_input(&game->phantoms)) return true;
    return input_is_held(input, GAME_BUTTON_LEFT) ||
           input_is_held(input, GAME_BUTTON_RIGHT) ||
           input_was_pressed(input, GAME_BUTTON_PANEL) ||
           input_was_pressed(input, GAME_BUTTON_SELECT) ||
           input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
           input_was_pressed(input, GAME_BUTTON_MODE) ||
           input_was_pressed(input, GAME_BUTTON_SEAL);
}

static void update_ventilation(Game *game)
{
    if (!game->ventilation_failed) {
        game->ventilation_error_frames = 0u;
        game->player_blinded = false;
        return;
    }
    ++game->ventilation_error_frames;
    const uint32_t phase = game->ventilation_error_frames % VENT_BLACKOUT_CYCLE_FRAMES;
    game->player_blinded = phase >= VENT_BLACK_START_FRAMES;
    if (!game->breathing_started && game->ventilation_error_frames >= VENT_DIM_START_FRAMES) {
        game->breathing_started = true;
        audio_restart(AUDIO_CUE_BREATHING, 0.58f, true);
    }
    if ((game->ventilation_error_frames % 6u) == 0u) game->dirty = true;
}

static void advance_hour(Game *game)
{
    game->hour_frames = 0u;
    game->hour = game->hour == 12 ? 1 : game->hour + 1;
    process_phantom_event(game, phantoms_on_hour_changed(&game->phantoms, game->hour));
    game->dirty = true;
    if (game->hour >= 6) begin_victory(game);
}

static void update_active_game(Game *game, const InputState *input)
{
    const NightConfig *config = night_config(game->night);
    ++game->night_frames;
    ++game->hour_frames;
    if (game->hour_frames >= FRAMES_PER_HOUR) {
        advance_hour(game);
        if (game->screen != SCREEN_OFFICE) return;
    }

    if (office_activity(game, input)) game->idle_frames = 0u;
    else if (game->idle_frames < IDLE_VENT_FAILURE_FRAMES) ++game->idle_frames;
    if (game->idle_frames >= IDLE_VENT_FAILURE_FRAMES) {
        set_ventilation_failure(game);
        game->idle_frames = 0u;
    }

    if (!game->camera_failure_triggered && config->camera_failure_frame != NO_FAILURE_FRAME &&
        game->night_frames >= config->camera_failure_frame) {
        game->camera_failure_triggered = true; game->camera_failed = true; game->dirty = true;
    }
    if (!game->ventilation_failure_triggered && config->ventilation_failure_frame != NO_FAILURE_FRAME &&
        game->night_frames >= config->ventilation_failure_frame) {
        game->ventilation_failure_triggered = true; set_ventilation_failure(game);
    }
    if (!game->audio_failure_triggered && config->audio_failure_frame != NO_FAILURE_FRAME &&
        game->night_frames >= config->audio_failure_frame) {
        game->audio_failure_triggered = true; game->audio_failed = true;
        game->audio_lure_active = false; set_message(game, MESSAGE_AUDIO_FAILED);
    }

    if (game->message_frames > 0u && --game->message_frames == 0u) {
        game->message = MESSAGE_NONE; game->dirty = true;
    }
    if (game->audio_lure_cooldown > 0u) --game->audio_lure_cooldown;
    if (game->audio_lure_active && ++game->audio_lure_frames >= AUDIO_LURE_TRAVEL_FRAMES) {
        SpringtrapEvent lure_event = springtrap_ai_lure(
            &game->springtrap, game->audio_lure_target, game->night);
        game->audio_lure_active = false;
        game->audio_lure_frames = 0u;
        if ((lure_event.flags & SPRINGTRAP_EVENT_MOVED) != 0u) {
            set_message(game, MESSAGE_AUDIO_SUCCESS);
            process_springtrap_event(game, lure_event, SPRINGTRAP_OFFICE_NONE);
        } else if ((lure_event.flags & SPRINGTRAP_EVENT_LURE_IGNORED) != 0u) {
            set_message(game, MESSAGE_AUDIO_IGNORED);
        } else {
            set_message(game, MESSAGE_AUDIO_NO_TARGET);
        }
    }

    update_ventilation(game);

    PhantomEvent phantom_event = phantoms_update(
        &game->phantoms, game->hour == 12 ? 0 : game->hour, game->camera_open,
        game->maintenance_open, game->selected_camera, game->office_pan);
    process_phantom_event(game, phantom_event);

    if (config->springtrap_enabled) {
        const SpringtrapOfficeSide attack_side =
            springtrap_ai_office_side(&game->springtrap);
        SpringtrapEvent event = springtrap_ai_update(
            &game->springtrap, game->night, game->hour == 12 ? 0 : game->hour,
            springtrap_interval(game), game->sealed_vent,
            springtrap_directly_observed(game), game->player_blinded);
        process_springtrap_event(game, event, attack_side);
    }

    if (game->repairing && ++game->repair_frames >= repair_duration(game)) {
        audio_stop(AUDIO_CUE_WAIT);
        finish_repair(game);
    } else if (game->repairing) {
        game->dirty = true;
    }

    if (game->camera_open && (game->night_frames % 12u) == 0u) game->dirty = true;
}

static void handle_title_input(Game *game, const InputState *input)
{
    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        game->title_selection = (game->title_selection + TITLE_ITEM_COUNT - 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        game->title_selection = (game->title_selection + 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) || input_was_pressed(input, GAME_BUTTON_START)) {
        if (game->title_selection == 0) {
            reset_night(game, 1);
        } else if (game->title_selection == 1) {
            reset_night(game, progress_save_highest_unlocked_night());
        } else {
            game->screen = SCREEN_PLACEHOLDER;
            game->screen_frames = 0u;
            game->dirty = true;
        }
    }
}

static void handle_camera_input(Game *game, const InputState *input)
{
    if (input_was_pressed(input, GAME_BUTTON_PANEL) || input_was_pressed(input, GAME_BUTTON_BACK)) {
        close_cameras(game); return;
    }
    if (input_was_pressed(input, GAME_BUTTON_SELECT)) {
        close_cameras(game); open_maintenance(game); return;
    }
    if (input_was_pressed(input, GAME_BUTTON_MODE)) {
        game->vent_map_open = !game->vent_map_open;
        restart_camera_static(); game->dirty = true; return;
    }
    if (game->vent_map_open) {
        int selected = (int) game->selected_vent;
        if (input_was_pressed(input, GAME_BUTTON_LEFT) || input_was_pressed(input, GAME_BUTTON_UP))
            selected = (selected + VENT_COUNT - 1) % VENT_COUNT;
        if (input_was_pressed(input, GAME_BUTTON_RIGHT) || input_was_pressed(input, GAME_BUTTON_DOWN))
            selected = (selected + 1) % VENT_COUNT;
        if (selected != (int) game->selected_vent) {
            game->selected_vent = (SpringtrapVent) selected;
            restart_camera_static(); game->dirty = true;
        }
        if (input_was_pressed(input, GAME_BUTTON_CONFIRM) || input_was_pressed(input, GAME_BUTTON_SEAL)) {
            game->sealed_vent = game->sealed_vent == game->selected_vent
                ? SPRINGTRAP_VENT_NONE : game->selected_vent;
            audio_play(AUDIO_CUE_WAIT, 0.6f, false);
            set_message(game, MESSAGE_VENT_SEALED);
        }
        return;
    }
    int next_camera = game->selected_camera;
    if (input_was_pressed(input, GAME_BUTTON_LEFT)) next_camera = camera_in_direction(next_camera, -1, 0);
    if (input_was_pressed(input, GAME_BUTTON_RIGHT)) next_camera = camera_in_direction(next_camera, 1, 0);
    if (input_was_pressed(input, GAME_BUTTON_UP)) next_camera = camera_in_direction(next_camera, 0, -1);
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) next_camera = camera_in_direction(next_camera, 0, 1);
