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
            break;
        case MAINTENANCE_VENTILATION:
            systems->ventilation_failed = false;
            break;
        case MAINTENANCE_REBOOT_ALL:
            systems->camera_failed = false;
            systems->audio_failed = false;
            systems->ventilation_failed = false;
            break;
        default:
            break;
    }
    systems->repairing = false;
    systems->repair_frames = 0u;
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
        app->needs_redraw = true;
    }

    if (systems->repairing) {
        ++systems->repair_frames;
        app->needs_redraw = true;
        if (systems->repair_frames >= repair_duration(systems)) {
            finish_repair(systems);
        }
    }

    if (systems->maintenance_open || systems->ventilation_failed ||
        systems->camera_failed) {
        if ((systems->night_frames % 12u) == 0u) {
            app->needs_redraw = true;
        }
    }
}

static void draw_warning_texture(void)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       60, 154, 734, 174,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     67, 174, 720, 138,
                     &gWarningTexture);
}

static void draw_title_texture(const App *app)
{
    const int frame = title_frame_index(app);
    const int glitch_x = frame == 0
        ? (int) ((app->effect_seed >> 10) % 7u) - 3
        : (frame - 2) * 3;
