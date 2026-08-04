        return 0;
    }
    return 1 + (int) (((phase - 144u) / 9u) % 4u);
}

static void systems_reset(SystemsState *systems)
{
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
    systems->springtrap_camera = 2;
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
    return systems->ventilation_failed
        ? SPRINGTRAP_MOVE_FRAMES_VENT_ERROR
        : SPRINGTRAP_MOVE_FRAMES;
}

static void update_systems(SystemsState *systems, App *app)
{
    if (app->screen != APP_SCREEN_OFFICE_TEST) {
        return;
    }

    ++systems->night_frames;

    if (!systems->camera_failure_triggered &&
        systems->night_frames >= CAMERA_FAILURE_FRAME) {
        systems->camera_failure_triggered = true;
        systems->camera_failed = true;
        app->needs_redraw = true;
    }
    if (!systems->ventilation_failure_triggered &&
        systems->night_frames >= VENTILATION_FAILURE_FRAME) {
        systems->ventilation_failure_triggered = true;
        systems->ventilation_failed = true;
        app->needs_redraw = true;
    }
    if (!systems->audio_failure_triggered &&
        systems->night_frames >= AUDIO_FAILURE_FRAME) {
        systems->audio_failure_triggered = true;
        systems->audio_failed = true;
        systems->audio_lure_active = false;
        set_audio_lure_message(systems, AUDIO_LURE_MESSAGE_ERROR);
        app->needs_redraw = true;
    }
