static void set_message(Game *game, MessageKind message)
{
    game->message = message;
    game->message_frames = MESSAGE_FRAMES;
    game->dirty = true;
}

static int title_frame_index(const Game *game)
{
    const uint32_t phase = game->screen_frames % 180u;
    if (phase < 144u) {
        return 0;
    }
    return 1 + (int) (((phase - 144u) / 9u) % 4u);
}

static uint32_t springtrap_interval(const Game *game)
{
    uint32_t interval = night_config(game->night)->move_interval;
    if (game->hour >= 4 && interval > 0u) {
        interval = (interval * 4u) / 5u;
    }
    if (game->hour >= 5 && interval > 0u) {
        interval = (interval * 4u) / 5u;
    }
    if (game->ventilation_failed && interval > 0u) {
        interval = (interval * 3u) / 4u;
    }
    return interval > 0u ? interval : 1u;
}

static void stop_panel_audio(void)
{
    audio_stop(AUDIO_CUE_STATIC);
    audio_stop(AUDIO_CUE_WAIT);
}

static void stop_ventilation_audio(Game *game)
{
    audio_stop(AUDIO_CUE_ALARM);
    audio_stop(AUDIO_CUE_BREATHING);
    game->breathing_started = false;
}

static void set_ventilation_failure(Game *game)
{
    if (game->ventilation_failed) {
        return;
    }
    game->ventilation_failed = true;
    game->ventilation_error_frames = 0u;
    game->player_blinded = false;
    game->breathing_started = false;
    audio_restart(AUDIO_CUE_ALARM, 0.72f, true);
    game->dirty = true;
}

static void reset_night(Game *game, int night)
{
    game->night = night;
    game->hour = 12;
    game->hour_frames = 0u;
    game->night_frames = 0u;
    game->office_pan = 0;
    game->camera_open = false;
    game->maintenance_open = false;
    game->vent_map_open = false;
    game->selected_camera = 2;
    game->selected_vent = SPRINGTRAP_VENT_11;
    game->sealed_vent = SPRINGTRAP_VENT_NONE;

    game->camera_failed = false;
    game->audio_failed = false;
    game->ventilation_failed = false;
    game->camera_failure_triggered = false;
    game->audio_failure_triggered = false;
    game->ventilation_failure_triggered = false;

    game->maintenance_selection = 0;
    game->repairing = false;
    game->repair_target = 0;
    game->repair_frames = 0u;
    game->idle_frames = 0u;
    game->ventilation_error_frames = 0u;
    game->breathing_started = false;
    game->player_blinded = false;

    game->audio_lure_active = false;
    game->audio_lure_target = 2;
    game->audio_lure_frames = 0u;
    game->audio_lure_cooldown = 0u;
    game->message = MESSAGE_NONE;
    game->message_frames = 0u;

    springtrap_ai_reset(&game->springtrap, night, game_random(game));
    stop_panel_audio();
    stop_ventilation_audio(game);
    game->screen = SCREEN_NIGHT_INTRO;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void game_init(Game *game)
{
    memset(game, 0, sizeof(*game));
    game->screen = SCREEN_WARNING;
    game->seed = 0xF3A30A10u;
    game->unlocked_night = 1;
    game->night = 1;
    game->selected_camera = 2;
    game->selected_vent = SPRINGTRAP_VENT_11;
    game->sealed_vent = SPRINGTRAP_VENT_NONE;
    game->dirty = true;
}

static bool springtrap_directly_observed(const Game *game)
{
    if (game->camera_open || game->maintenance_open || game->player_blinded) {
        return false;
    }

    const SpringtrapOfficeSide side =
        springtrap_ai_office_side(&game->springtrap);
    if (side == SPRINGTRAP_OFFICE_WINDOW) {
        return game->office_pan >= 72;
    }
    if (side == SPRINGTRAP_OFFICE_LEFT) {
        return game->office_pan <= -72;
    }
    return false;
}

static void begin_game_over(Game *game)
{
    if (game->screen == SCREEN_GAME_OVER) {
        return;
    }
    stop_panel_audio();
    stop_ventilation_audio(game);
    audio_stop_all();
    audio_play(AUDIO_CUE_SCREAM, 1.0f, false);
    game->camera_open = false;
    game->maintenance_open = false;
    game->screen = SCREEN_GAME_OVER;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void begin_victory(Game *game)
{
    stop_panel_audio();
    stop_ventilation_audio(game);
    game->camera_open = false;
    game->maintenance_open = false;
    game->hour = 6;
    game->screen = SCREEN_VICTORY;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void process_springtrap_event(Game *game, SpringtrapEvent event)
{
    if ((event.flags & SPRINGTRAP_EVENT_VENT_ENTER) != 0u) {
        AudioCue cue = (game_random(game) & 1u) != 0u
            ? AUDIO_CUE_VENT_QUIET_1
            : AUDIO_CUE_VENT_QUIET_2;
        audio_play(cue, 0.62f, false);
    }
    if ((event.flags & SPRINGTRAP_EVENT_VENT_EXIT) != 0u) {
        AudioCue cue = (game_random(game) & 1u) != 0u
            ? AUDIO_CUE_VENT_CLOSER_1
            : AUDIO_CUE_VENT_LOUDER_2;
        audio_play(cue, 0.82f, false);
    }
    if ((event.flags & SPRINGTRAP_EVENT_DANGER_SOFT) != 0u) {
        /* danger2b hook: play softly when the user supplies that cue. */
        set_message(game, MESSAGE_DANGER_SOFT);
    }
    if ((event.flags & SPRINGTRAP_EVENT_DANGER_LOUD) != 0u) {
        /* danger2b hook: play loudly when the user supplies that cue. */
        set_message(game, MESSAGE_DANGER_LOUD);
    }
    if ((event.flags & SPRINGTRAP_EVENT_ATTACK) != 0u) {
        begin_game_over(game);
    }
    if (event.flags != 0u) {
        game->dirty = true;
    }
}

static void finish_repair(Game *game)
{
    if (game->repair_target == MAINT_CAMERA ||
        game->repair_target == MAINT_REBOOT_ALL) {
        game->camera_failed = false;
    }
    if (game->repair_target == MAINT_AUDIO ||
        game->repair_target == MAINT_REBOOT_ALL) {
        game->audio_failed = false;
    }
    if (game->repair_target == MAINT_VENTILATION ||
        game->repair_target == MAINT_REBOOT_ALL) {
        game->ventilation_failed = false;
        game->ventilation_error_frames = 0u;
        game->player_blinded = false;
        game->idle_frames = 0u;
        stop_ventilation_audio(game);
    }
    game->repairing = false;
    game->repair_frames = 0u;
    game->dirty = true;
}

static void start_repair(Game *game)
{
    if (game->maintenance_selection == MAINT_EXIT) {
        game->maintenance_open = false;
        audio_stop(AUDIO_CUE_WAIT);
        game->dirty = true;
        return;
    }
    game->repair_target = game->maintenance_selection;
    game->repair_frames = 0u;
    game->repairing = true;
    audio_restart(AUDIO_CUE_WAIT, 0.65f, true);
    game->dirty = true;
}

static uint32_t repair_duration(const Game *game)
{
