static OriginalUiTextureId original_ui_maintenance_texture(const Game *game,
                                                            int item)
{
    switch (item) {
        case MAINT_CAMERA:
            return game->camera_failed
                ? ORIGINAL_UI_MAINT_CAMERA_ERROR
                : ORIGINAL_UI_MAINT_CAMERA;
        case MAINT_AUDIO:
            return game->audio_failed
                ? ORIGINAL_UI_MAINT_AUDIO_ERROR
                : ORIGINAL_UI_MAINT_AUDIO;
        case MAINT_VENTILATION:
            return game->ventilation_failed
                ? ORIGINAL_UI_MAINT_VENT_ERROR
                : ORIGINAL_UI_MAINT_VENT;
        case MAINT_REBOOT_ALL:
            return ORIGINAL_UI_MAINT_REBOOT_ALL;
        case MAINT_EXIT:
        default:
            return ORIGINAL_UI_MAINT_MENU;
    }
}

static void original_ui_draw_maintenance(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 31, 28, 792, 424, 4,
                        GRAPHICS_RGB(55, 113, 60));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 62, 49, 4,
                       "SYSTEM RESTART", COLOUR_GREEN);

    for (int item = 0; item < MAINTENANCE_ITEM_COUNT; ++item) {
        const int y = 105 + item * 58;
        const bool selected = item == game->maintenance_selection;
        if (selected) {
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 58, y - 10,
                               730, 48, GRAPHICS_RGB(22, 58, 27));
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 58, y - 10,
                               7, 48, GRAPHICS_RGB(194, 255, 77));
        }
        const TextureRle *label = original_ui_get(
            original_ui_maintenance_texture(game, item));
        if (label != NULL) {
            const int width = (int) label->width * 2;
            texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                             82, y, width, 40, label);
        }
        if (item < MAINT_REBOOT_ALL) {
            const bool failed = maintenance_failed(game, item);
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 657, y + 13, 2,
                               failed ? "ERROR" : "ONLINE",
                               failed ? COLOUR_RED : COLOUR_GREEN);
        }
    }

    if (game->repairing) {
        const uint32_t duration = repair_duration(game);
        const int progress = duration == 0u ? 0
            : (int) ((game->repair_frames * 620u) / duration);
        original_ui_draw_texture(ORIGINAL_UI_MAINT_RESTART,
                                 GRAPHICS_TARGET_GAMEPAD,
                                 272, 375, 300, 40);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 112, 423, 620, 13,
                           GRAPHICS_RGB(6, 16, 8));
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 112, 423, progress, 13,
                           GRAPHICS_RGB(194, 255, 77));
    } else {
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 222, 421, 1,
                           "UP/DOWN: SELECT   A: REBOOT   B/MINUS: CLOSE",
                           GRAPHICS_RGB(174, 199, 169));
    }
}

static void original_ui_draw_transition(void)
{
    if (sMonitorV2.assets_ready) {
        monitor_v2_draw_transition();
        return;
    }

    if (!sOriginalUi.transition_active ||
        sOriginalUi.transition_frame < 0 ||
        sOriginalUi.transition_frame >
            ORIGINAL_UI_TRANSITION_6 - ORIGINAL_UI_TRANSITION_0) {
        return;
    }
    const OriginalUiTextureId id = (OriginalUiTextureId)
        (ORIGINAL_UI_TRANSITION_0 + sOriginalUi.transition_frame);
    original_ui_draw_texture(id, GRAPHICS_TARGET_GAMEPAD,
                             0, 0, 854, 480);
}

static void original_ui_draw_office(Game *game)
{
    game->dirty = false;
    draw_office_tv(game);

    if (game->maintenance_open)
        original_ui_draw_maintenance(game);
    else if (game->camera_open) {
        if (game->vent_map_open)
            original_ui_draw_vent_map(game);
        else
            original_ui_draw_camera_feed(game);
    } else {
        draw_office_gamepad(game);
        office_touch_draw_overlay(game);
    }

    draw_ventilation_overlay(game);
    draw_phantom_jumpscare(game);
    if (phantoms_active_jumpscare(&game->phantoms) == PHANTOM_NONE)
        original_ui_draw_transition();
    finishing_draw_phone_overlay();
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void render_game(Game *game)
{
    if (!sOriginalUi.assets_ready || game->screen != SCREEN_OFFICE) {
        fnaf3_full_audio_render_game(game);
        return;
    }
    if (!game->dirty) return;
    original_ui_draw_office(game);
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
    memset(&sFinishing, 0, sizeof(sFinishing));
    memset(&sSecret, 0, sizeof(sSecret));
    memset(&sSecretTriggers, 0, sizeof(sSecretTriggers));
    memset(&sCompleteCheats, 0, sizeof(sCompleteCheats));
    memset(&sOriginalUi, 0, sizeof(sOriginalUi));
    memset(&sMonitorV2, 0, sizeof(sMonitorV2));
    sTitleMusicPlaying = false;
    sOriginalUi.assets_ready = original_ui_assets_init();
    sMonitorV2.assets_ready = monitor_v2_assets_init();

    Game game;
    game_init(&game);
    (void) audio_init();

    render_game(&game);
    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);
        update_game(&game, &input);
        render_game(&game);
        frame_clock_wait_next();
    }

    audio_stop_all();
    audio_shutdown();
    progress_save_shutdown();
    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
