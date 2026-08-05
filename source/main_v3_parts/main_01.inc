    if ((event.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
        audio_restart(AUDIO_CUE_SCREAM, 0.9f, false);
    }
    if ((event.flags & PHANTOM_EVENT_JUMPSCARE_ENDED) != 0u) {
        audio_stop(AUDIO_CUE_SCREAM);
    }
    if (event.flags != PHANTOM_EVENT_NONE) game->dirty = true;
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
    game->springtrap_jumpscare_right = false;

    springtrap_ai_reset(&game->springtrap, night, game_random(game));
    phantoms_reset(&game->phantoms, night, night >= 6, game_random(game));
    audio_stop_all();
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
    phantoms_reset(&game->phantoms, 1, false, game->seed);
    game->dirty = true;
}

static bool springtrap_directly_observed(const Game *game)
{
    if (game->camera_open || game->maintenance_open || game->player_blinded ||
        phantoms_blocks_input(&game->phantoms) ||
        phantoms_active_jumpscare(&game->phantoms) != PHANTOM_NONE) {
        return false;
    }
    const SpringtrapOfficeSide side = springtrap_ai_office_side(&game->springtrap);
    if (side == SPRINGTRAP_OFFICE_WINDOW) return game->office_pan >= 72;
    if (side == SPRINGTRAP_OFFICE_LEFT) return game->office_pan <= -72;
    return false;
}

static void begin_game_over(Game *game)
{
    audio_stop(AUDIO_CUE_SCREAM);
    game->screen = SCREEN_GAME_OVER;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void begin_springtrap_jumpscare(Game *game,
                                      SpringtrapOfficeSide attack_side)
{
    if (game->screen == SCREEN_SPRINGTRAP_JUMPSCARE ||
        game->screen == SCREEN_GAME_OVER) {
        return;
    }
    audio_stop_all();
    audio_play(AUDIO_CUE_SCREAM, 1.0f, false);
    game->camera_open = false;
    game->maintenance_open = false;
    game->vent_map_open = false;
    game->springtrap_jumpscare_right =
        attack_side == SPRINGTRAP_OFFICE_WINDOW ||
        (attack_side == SPRINGTRAP_OFFICE_NONE && game->office_pan >= 0);
    game->screen = SCREEN_SPRINGTRAP_JUMPSCARE;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void begin_victory(Game *game)
{
    audio_stop_all();
    game->camera_open = false;
    game->maintenance_open = false;
    game->hour = 6;
    game->screen = SCREEN_VICTORY;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void process_springtrap_event(Game *game,
                                     SpringtrapEvent event,
                                     SpringtrapOfficeSide attack_side)
{
    if ((event.flags & SPRINGTRAP_EVENT_VENT_ENTER) != 0u) {
        audio_play((game_random(game) & 1u) != 0u
            ? AUDIO_CUE_VENT_QUIET_1 : AUDIO_CUE_VENT_QUIET_2,
            0.62f, false);
    }
    if ((event.flags & SPRINGTRAP_EVENT_VENT_EXIT) != 0u) {
        audio_play((game_random(game) & 1u) != 0u
            ? AUDIO_CUE_VENT_CLOSER_1 : AUDIO_CUE_VENT_LOUDER_2,
            0.82f, false);
    }
    if ((event.flags & SPRINGTRAP_EVENT_DANGER_SOFT) != 0u)
        set_message(game, MESSAGE_DANGER_SOFT);
    if ((event.flags & SPRINGTRAP_EVENT_DANGER_LOUD) != 0u)
        set_message(game, MESSAGE_DANGER_LOUD);
    if ((event.flags & SPRINGTRAP_EVENT_ATTACK) != 0u)
        begin_springtrap_jumpscare(game, attack_side);
    if (event.flags != 0u) game->dirty = true;
}

static void finish_repair(Game *game)
{
    if (game->repair_target == MAINT_CAMERA || game->repair_target == MAINT_REBOOT_ALL)
        game->camera_failed = false;
    if (game->repair_target == MAINT_AUDIO || game->repair_target == MAINT_REBOOT_ALL)
        game->audio_failed = false;
    if (game->repair_target == MAINT_VENTILATION || game->repair_target == MAINT_REBOOT_ALL) {
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
    return game->repair_target == MAINT_REBOOT_ALL
        ? REPAIR_ALL_FRAMES : REPAIR_SINGLE_FRAMES;
}

static void play_random_lure(Game *game)
{
    const uint32_t pick = game_random(game) % 3u;
    AudioCue cue = pick == 0u ? AUDIO_CUE_LURE_ECHO_1
                   : pick == 1u ? AUDIO_CUE_LURE_ECHO_3B
                                : AUDIO_CUE_LURE_ECHO_4B;
    audio_play(cue, 0.72f, false);
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
    play_random_lure(game);
    set_message(game, MESSAGE_AUDIO_PLAYING);
}

static int camera_in_direction(int current, int horizontal, int vertical)
{
    const MapPoint origin = kCameraMapPoints[current];
    int best = current;
    int best_score = 0x7FFFFFFF;
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        if (camera == current) continue;
        const int dx = kCameraMapPoints[camera].x - origin.x;
        const int dy = kCameraMapPoints[camera].y - origin.y;
        if ((horizontal < 0 && dx >= 0) || (horizontal > 0 && dx <= 0) ||
            (vertical < 0 && dy >= 0) || (vertical > 0 && dy <= 0)) continue;
        const int primary = horizontal != 0 ? (dx < 0 ? -dx : dx)
                                            : (dy < 0 ? -dy : dy);
        const int perpendicular = horizontal != 0 ? (dy < 0 ? -dy : dy)
                                                   : (dx < 0 ? -dx : dx);
        const int score = primary + perpendicular * 3;
        if (score < best_score) { best_score = score; best = camera; }
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
    process_phantom_event(game, phantoms_on_panel_opened(&game->phantoms));
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

