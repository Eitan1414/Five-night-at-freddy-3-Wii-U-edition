}

static void systems_reset(SystemsState *systems, int night_number)
{
    const NightConfig *config = night_config(night_number);
    systems->night_number = config->number;
    systems->night_phase = NIGHT_PHASE_INTRO;
    systems->phase_frames = 0u;
    systems->current_hour = 12;
    systems->hour_frames = 0u;
    systems->attack_frames = 0u;
    systems->maintenance_open = false;
    systems->maintenance_selection = 0;
    systems->repairing = false;
    systems->repair_target = 0;
    systems->repair_frames = 0u;
    systems->night_frames = 0u;
    systems->camera_failed = false;
    systems->audio_failed = false;
    systems->ventilation_failed = false;
    systems->camera_failure_triggered = false;
    systems->audio_failure_triggered = false;
    systems->ventilation_failure_triggered = false;
    systems->springtrap_camera = config->springtrap_enabled ? 2 : -1;
    systems->springtrap_move_frames = 0u;
    systems->audio_lure_active = false;
    systems->audio_lure_target = 2;
    systems->audio_lure_frames = 0u;
    systems->audio_lure_cooldown_frames = 0u;
    systems->audio_lure_message = AUDIO_LURE_MESSAGE_NONE;
    systems->audio_lure_message_frames = 0u;
}

static uint32_t repair_duration(const SystemsState *systems)
{
    return systems->repair_target == MAINTENANCE_REBOOT_ALL
        ? REPAIR_ALL_FRAMES
        : REPAIR_SINGLE_FRAMES;
}

static void finish_repair(SystemsState *systems)
{
    switch (systems->repair_target) {
        case MAINTENANCE_CAMERA:
            systems->camera_failed = false;
            break;
        case MAINTENANCE_AUDIO:
            systems->audio_failed = false;
            systems->audio_lure_message = AUDIO_LURE_MESSAGE_NONE;
            systems->audio_lure_message_frames = 0u;
            break;
        case MAINTENANCE_VENTILATION:
            systems->ventilation_failed = false;
            break;
        case MAINTENANCE_REBOOT_ALL:
            systems->camera_failed = false;
            systems->audio_failed = false;
            systems->ventilation_failed = false;
            systems->audio_lure_message = AUDIO_LURE_MESSAGE_NONE;
            systems->audio_lure_message_frames = 0u;
            break;
        default:
            break;
    }
    systems->repairing = false;
    systems->repair_frames = 0u;
}

static void set_audio_lure_message(SystemsState *systems,
                                   AudioLureMessage message)
{
    systems->audio_lure_message = message;
    systems->audio_lure_message_frames = AUDIO_LURE_MESSAGE_FRAMES;
}

static void trigger_audio_lure(SystemsState *systems,
                               App *app,
                               int selected_camera)
{
    const NightConfig *config = night_config(systems->night_number);
    if (systems->night_phase != NIGHT_PHASE_ACTIVE) {
        return;
    }
    if (!config->springtrap_enabled) {
        set_audio_lure_message(systems, AUDIO_LURE_MESSAGE_NO_TARGET);
        app->needs_redraw = true;
        return;
    }
    if (systems->audio_failed) {
        systems->audio_lure_active = false;
        set_audio_lure_message(systems, AUDIO_LURE_MESSAGE_ERROR);
        app->needs_redraw = true;
        return;
    }

    if (systems->audio_lure_active ||
        systems->audio_lure_cooldown_frames > 0u) {
        set_audio_lure_message(systems, AUDIO_LURE_MESSAGE_COOLDOWN);
        app->needs_redraw = true;
        return;
    }

    systems->audio_lure_active = true;
    systems->audio_lure_target = selected_camera;
    systems->audio_lure_frames = 0u;
    systems->audio_lure_cooldown_frames = AUDIO_LURE_COOLDOWN_FRAMES;
    systems->springtrap_move_frames = 0u;
    set_audio_lure_message(systems, AUDIO_LURE_MESSAGE_PLAYING);
    app->needs_redraw = true;
}

static uint32_t springtrap_move_interval(const SystemsState *systems)
{
    const NightConfig *config = night_config(systems->night_number);
    uint32_t interval = config->springtrap_move_frames;

    if (systems->current_hour >= 4 && interval > 0u) {
        interval = (interval * 3u) / 4u;
    }
    if (systems->ventilation_failed && interval > 0u) {
        interval = (interval * 3u) / 5u;
    }
    return interval > 0u ? interval : 1u;
}

static void begin_victory(SystemsState *systems, App *app)
{
    systems->night_phase = NIGHT_PHASE_VICTORY;
    systems->phase_frames = 0u;
    systems->current_hour = 6;
    systems->hour_frames = 0u;
    systems->maintenance_open = false;
    systems->repairing = false;
    systems->audio_lure_active = false;
    app->camera_panel_open = false;
    app->needs_redraw = true;
}

static void begin_game_over(SystemsState *systems, App *app)
{
    systems->night_phase = NIGHT_PHASE_GAME_OVER;
    systems->phase_frames = 0u;
    systems->maintenance_open = false;
    systems->repairing = false;
    systems->audio_lure_active = false;
    app->camera_panel_open = false;
    app->needs_redraw = true;
}

static void advance_hour(SystemsState *systems, App *app)
{
    systems->hour_frames = 0u;
    systems->current_hour = systems->current_hour == 12
        ? 1
        : systems->current_hour + 1;
    app->needs_redraw = true;
    if (systems->current_hour >= 6) {
        begin_victory(systems, app);
    }
}

static void update_systems(SystemsState *systems, App *app)
{
    if (app->screen != APP_SCREEN_OFFICE_TEST) {
        return;
    }

    ++systems->phase_frames;

    if (systems->night_phase == NIGHT_PHASE_INTRO) {
        if (systems->phase_frames >= NIGHT_INTRO_FRAMES) {
            systems->night_phase = NIGHT_PHASE_ACTIVE;
            systems->phase_frames = 0u;
            app->needs_redraw = true;
        } else if ((systems->phase_frames % 15u) == 0u) {
            app->needs_redraw = true;
        }
        return;
    }

    if (systems->night_phase == NIGHT_PHASE_VICTORY ||
        systems->night_phase == NIGHT_PHASE_GAME_OVER) {
        if ((systems->phase_frames % 6u) == 0u) {
            app->needs_redraw = true;
        }
        return;
    }

    const NightConfig *config = night_config(systems->night_number);
    ++systems->night_frames;
