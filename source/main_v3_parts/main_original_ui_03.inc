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

static void original_ui_draw_maintenance_sprite(OriginalUiTextureId id,
                                                 int x,
                                                 int y,
                                                 int max_width,
                                                 int height)
{
    const TextureRle *texture = original_ui_get(id);
    if (texture == NULL || texture->width == 0u || texture->height == 0u ||
        height <= 0 || max_width <= 0) {
        return;
    }

    int width = (int)(((uint32_t)texture->width * (uint32_t)height) /
                      (uint32_t)texture->height);
    if (width > max_width) {
        width = max_width;
        height = (int)(((uint32_t)texture->height * (uint32_t)width) /
                       (uint32_t)texture->width);
    }
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     x, y, width, height, texture);
}

static void original_ui_draw_maintenance_cursor(int y)
{
    const uint32_t colour = GRAPHICS_RGB(224, 238, 219);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 66, y + 13, 18, 3, colour);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 78, y + 8, 3, 13, colour);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 81, y + 11, 3, 7, colour);
}

static void original_ui_draw_maintenance(const Game *game)
{
    static const int display_order[MAINTENANCE_ITEM_COUNT] = {
        MAINT_AUDIO,
        MAINT_CAMERA,
        MAINT_VENTILATION,
        MAINT_REBOOT_ALL,
        MAINT_EXIT,
    };

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(0, 2, 1));

    /* ORIGINAL_UI_MAINT_MENU is the extracted PC "System Restart" heading;
     * the previous Wii U pass incorrectly reused it as the EXIT row. */
    original_ui_draw_maintenance_sprite(ORIGINAL_UI_MAINT_MENU,
                                        119, 39, 640, 58);

    for (int row = 0; row < MAINTENANCE_ITEM_COUNT; ++row) {
        const int item = display_order[row];
        const int y = 116 + row * 55;
        const bool selected = item == game->maintenance_selection;

        if (selected) original_ui_draw_maintenance_cursor(y);

        if (item == MAINT_EXIT) {
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               104, y + 8, 3, "EXIT",
                               selected ? COLOUR_WHITE
                                        : GRAPHICS_RGB(153, 171, 151));
            continue;
        }

        original_ui_draw_maintenance_sprite(
            original_ui_maintenance_texture(game, item),
            104, y, 620, 38);
    }

    if (game->repairing) {
        original_ui_draw_maintenance_sprite(ORIGINAL_UI_MAINT_RESTART,
                                            245, 399, 365, 34);

        /* Clickteam's restart indicator advances as a small repeating dot
         * animation; do not expose the artificial Wii U percentage bar. */
        const int dots = 1 + (int)((game->repair_frames / 20u) % 3u);
        for (int dot = 0; dot < dots; ++dot) {
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               562 + dot * 13, 419, 6, 6,
                               COLOUR_WHITE);
        }
    } else {
        /* Controller legend is the only Wii U-specific addition retained on
         * this screen; all game-owned labels above come from the PC asset bank. */
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           250, 452, 1,
                           "UP/DOWN SELECT   A REBOOT   B CLOSE",
                           GRAPHICS_RGB(105, 125, 103));
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
