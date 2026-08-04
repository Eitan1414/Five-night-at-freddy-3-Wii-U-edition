    return game->repair_target == MAINT_REBOOT_ALL
        ? REPAIR_ALL_FRAMES
        : REPAIR_SINGLE_FRAMES;
}

static void trigger_audio_lure(Game *game)
{
    if (!night_config(game->night)->springtrap_enabled) {
        set_message(game, MESSAGE_AUDIO_NO_TARGET);
        return;
    }
    if (game->audio_failed) {
        set_message(game, MESSAGE_AUDIO_FAILED);
        return;
    }
    if (game->audio_lure_active || game->audio_lure_cooldown > 0u) {
        set_message(game, MESSAGE_AUDIO_RECHARGING);
        return;
    }

    game->audio_lure_active = true;
    game->audio_lure_target = game->selected_camera;
    game->audio_lure_frames = 0u;
    game->audio_lure_cooldown = AUDIO_LURE_COOLDOWN_FRAMES;
    audio_play(AUDIO_CUE_WAIT, 0.48f, false);
    set_message(game, MESSAGE_AUDIO_PLAYING);
}

static int camera_in_direction(int current, int horizontal, int vertical)
{
    const MapPoint origin = kCameraMapPoints[current];
    int best = current;
    int best_score = 0x7FFFFFFF;

    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        if (camera == current) {
            continue;
        }
        const int dx = kCameraMapPoints[camera].x - origin.x;
        const int dy = kCameraMapPoints[camera].y - origin.y;
        if ((horizontal < 0 && dx >= 0) || (horizontal > 0 && dx <= 0) ||
            (vertical < 0 && dy >= 0) || (vertical > 0 && dy <= 0)) {
            continue;
        }
        const int primary = horizontal != 0 ? (dx < 0 ? -dx : dx)
                                            : (dy < 0 ? -dy : dy);
        const int perpendicular = horizontal != 0 ? (dy < 0 ? -dy : dy)
                                                   : (dx < 0 ? -dx : dx);
        const int score = primary + perpendicular * 3;
        if (score < best_score) {
            best_score = score;
            best = camera;
        }
    }
    return best;
}

static void restart_camera_static(void)
{
    audio_restart(AUDIO_CUE_STATIC, 0.28f, true);
}

static void open_cameras(Game *game)
{
    game->camera_open = true;
    game->maintenance_open = false;
    game->vent_map_open = false;
    game->idle_frames = 0u;
    restart_camera_static();
    game->dirty = true;
}

static void close_cameras(Game *game)
{
    game->camera_open = false;
    game->vent_map_open = false;
    audio_stop(AUDIO_CUE_STATIC);
    game->dirty = true;
}

static void open_maintenance(Game *game)
{
    game->maintenance_open = true;
    game->camera_open = false;
    game->vent_map_open = false;
    game->maintenance_selection = 0;
    game->idle_frames = 0u;
    audio_stop(AUDIO_CUE_STATIC);
    audio_play(AUDIO_CUE_WAIT, 0.45f, false);
    game->dirty = true;
}

static bool office_activity(const Game *game, const InputState *input)
{
    if (game->camera_open || game->maintenance_open) {
        return true;
    }
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
    const uint32_t phase =
        game->ventilation_error_frames % VENT_BLACKOUT_CYCLE_FRAMES;
    game->player_blinded = phase >= VENT_BLACK_START_FRAMES;

    if (!game->breathing_started &&
        game->ventilation_error_frames >= VENT_DIM_START_FRAMES) {
        game->breathing_started = true;
        audio_restart(AUDIO_CUE_BREATHING, 0.58f, true);
    }
    if ((game->ventilation_error_frames % 6u) == 0u) {
        game->dirty = true;
    }
}

static void advance_hour(Game *game)
{
    game->hour_frames = 0u;
    game->hour = game->hour == 12 ? 1 : game->hour + 1;
    game->dirty = true;
    if (game->hour >= 6) {
        begin_victory(game);
    }
}

static void update_active_game(Game *game, const InputState *input)
{
    const NightConfig *config = night_config(game->night);
    ++game->night_frames;
    ++game->hour_frames;

    if (game->hour_frames >= FRAMES_PER_HOUR) {
        advance_hour(game);
        if (game->screen != SCREEN_OFFICE) {
            return;
        }
    }

    if (office_activity(game, input)) {
        game->idle_frames = 0u;
    } else if (game->idle_frames < IDLE_VENT_FAILURE_FRAMES) {
        ++game->idle_frames;
    }
    if (game->idle_frames >= IDLE_VENT_FAILURE_FRAMES) {
        set_ventilation_failure(game);
        game->idle_frames = 0u;
    }

    if (!game->camera_failure_triggered &&
        config->camera_failure_frame != NO_FAILURE_FRAME &&
        game->night_frames >= config->camera_failure_frame) {
        game->camera_failure_triggered = true;
        game->camera_failed = true;
        game->dirty = true;
    }
    if (!game->ventilation_failure_triggered &&
        config->ventilation_failure_frame != NO_FAILURE_FRAME &&
        game->night_frames >= config->ventilation_failure_frame) {
        game->ventilation_failure_triggered = true;
        set_ventilation_failure(game);
    }
    if (!game->audio_failure_triggered &&
        config->audio_failure_frame != NO_FAILURE_FRAME &&
        game->night_frames >= config->audio_failure_frame) {
        game->audio_failure_triggered = true;
        game->audio_failed = true;
        game->audio_lure_active = false;
        set_message(game, MESSAGE_AUDIO_FAILED);
    }

    if (game->message_frames > 0u) {
        --game->message_frames;
        if (game->message_frames == 0u) {
            game->message = MESSAGE_NONE;
            game->dirty = true;
        }
    }
    if (game->audio_lure_cooldown > 0u) {
        --game->audio_lure_cooldown;
    }
    if (game->audio_lure_active) {
        ++game->audio_lure_frames;
        if (game->audio_lure_frames >= AUDIO_LURE_TRAVEL_FRAMES) {
            SpringtrapEvent lure_event = springtrap_ai_lure(
                &game->springtrap, game->audio_lure_target);
            game->audio_lure_active = false;
            game->audio_lure_frames = 0u;
            if ((lure_event.flags & SPRINGTRAP_EVENT_MOVED) != 0u) {
                set_message(game, MESSAGE_AUDIO_SUCCESS);
                process_springtrap_event(game, lure_event);
            } else {
                set_message(game, MESSAGE_AUDIO_NO_TARGET);
            }
        }
    }

    update_ventilation(game);

    if (config->springtrap_enabled) {
        const SpringtrapEvent event = springtrap_ai_update(
            &game->springtrap,
            game->night,
            springtrap_interval(game),
            game->sealed_vent,
            springtrap_directly_observed(game),
            game->player_blinded);
        process_springtrap_event(game, event);
    }

    if (game->repairing) {
        ++game->repair_frames;
        game->dirty = true;
        if (game->repair_frames >= repair_duration(game)) {
            audio_stop(AUDIO_CUE_WAIT);
            finish_repair(game);
        }
