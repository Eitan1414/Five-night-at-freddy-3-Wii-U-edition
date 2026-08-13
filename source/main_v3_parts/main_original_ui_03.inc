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
    texture_draw_rle(sOriginalUiPanelTargets,
                     x, y, width, height, texture);
}

static void original_ui_draw_maintenance_cursor(int x, int y)
{
    const uint32_t colour = GRAPHICS_RGB(224, 238, 219);
    graphics_draw_rect(sOriginalUiPanelTargets, x, y + 13, 18, 3, colour);
    graphics_draw_rect(sOriginalUiPanelTargets, x + 12, y + 8, 3, 13, colour);
    graphics_draw_rect(sOriginalUiPanelTargets, x + 15, y + 11, 3, 7, colour);
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

    /* TV already has the PC office. In mirrored mode only, copy the same office
     * to the GamePad before drawing the System Restart overlay on both. */
    if ((sOriginalUiPanelTargets & GRAPHICS_TARGET_GAMEPAD) != 0u)
        pc_draw_gamepad_office_background(game);

    graphics_draw_rect(sOriginalUiPanelTargets,
                       214, 48, 604, 390,
                       GRAPHICS_RGB(0, 1, 0));
    graphics_draw_frame(sOriginalUiPanelTargets,
                        214, 48, 604, 390, 2,
                        GRAPHICS_RGB(48, 67, 48));

    original_ui_draw_maintenance_sprite(ORIGINAL_UI_MAINT_MENU,
                                        275, 67, 485, 50);

    for (int row = 0; row < MAINTENANCE_ITEM_COUNT; ++row) {
        const int item = display_order[row];
        const int y = 127 + row * 50;
        const bool selected = item == game->maintenance_selection;

        if (selected) original_ui_draw_maintenance_cursor(252, y);

        if (item == MAINT_EXIT) {
            graphics_draw_text(sOriginalUiPanelTargets,
                               291, y + 8, 3, "EXIT",
                               selected ? COLOUR_WHITE
                                        : GRAPHICS_RGB(153, 171, 151));
            continue;
        }

        original_ui_draw_maintenance_sprite(
            original_ui_maintenance_texture(game, item),
            291, y, 470, 34);
    }

    if (game->repairing) {
        original_ui_draw_maintenance_sprite(ORIGINAL_UI_MAINT_RESTART,
                                            352, 376, 330, 31);
        const int dots = 1 + (int)((game->repair_frames / 20u) % 3u);
        for (int dot = 0; dot < dots; ++dot) {
            graphics_draw_rect(sOriginalUiPanelTargets,
                               640 + dot * 13, 395, 6, 6,
                               COLOUR_WHITE);
        }
    } else {
        graphics_draw_text(sOriginalUiPanelTargets,
                           369, 414, 1,
                           "UP/DOWN   A REBOOT   B CLOSE",
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
    original_ui_draw_texture(id, sOriginalUiPanelTargets,
                             174, 76, 674, 379);
}

static void original_ui_draw_office(Game *game)
{
    game->dirty = false;
    sOriginalUiPanelTargets = wiiu_control_panel_targets();

    /* The television always owns the main game presentation. */
    draw_office_tv(game);

    if (game->maintenance_open)
        original_ui_draw_maintenance(game);
    else if (game->camera_open) {
        if (game->vent_map_open)
            original_ui_draw_vent_map(game);
        else
            original_ui_draw_camera_feed(game);
    } else if (wiiu_controls_mode() == WIIU_CONTROL_MIRROR) {
        /* Identical mode keeps the office itself on both displays too. */
        pc_draw_gamepad_office_background(game);
    }

    draw_ventilation_overlay(game);
    draw_phantom_jumpscare(game);
    if (phantoms_active_jumpscare(&game->phantoms) == PHANTOM_NONE)
        original_ui_draw_transition();
    finishing_draw_phone_overlay();

    /* Last GamePad pass is deliberate: in TOUCH/TV modes it erases any shared
     * overlays and leaves exactly the requested black/contextual DRC image. */
    wiiu_control_draw_gamepad(game);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void pc_warning_fidelity_render(Game *game)
{
    if (game == NULL || !game->dirty) return;
    game->dirty = false;

    /* The supplied PC MFA stores the warning wording as text. gWarningTexture
     * is the PC-only rasterization of that exact wording. The recovery renderer
     * added a flashing red frame and visible `A: SKIP` prompt; those are Wii U
     * prototype presentation and are intentionally omitted here.  Confirm/Start
     * may still skip the warning as an invisible platform convenience. */
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     67, 174, 720, 138,
                     &gWarningTexture);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static const char *pc_night_intro_label(int night)
{
    static const char *const labels[6] = {
        "1ST NIGHT", "2ND NIGHT", "3RD NIGHT",
        "4TH NIGHT", "5TH NIGHT", "6TH NIGHT",
    };
    if (night < 1) night = 1;
    if (night > 6) night = 6;
    return labels[night - 1];
}

static void pc_night_intro_fidelity_render(Game *game)
{
    if (game == NULL || !game->dirty) return;
    game->dirty = false;

    /* Retail-PC observed presentation: 12:00 AM above an ordinal night label;
     * Night 1 is green while subsequent nights use white. Nightmare is still
     * introduced as `6TH NIGHT`. The raw MFA object coordinates/font metrics
     * have not yet been re-extracted, so placement/scaling here remains an
     * explicitly documented Wii U adaptation rather than a claimed MFA match. */
    const uint32_t colour = game->night == 1
        ? GRAPHICS_RGB(68, 190, 88) : COLOUR_WHITE;

    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       329, 145, 4, "12:00 AM", colour);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       278, 241, 5,
                       pc_night_intro_label(game->night), colour);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void render_game(Game *game)
{
    if (sWiiUOptionsOpen) {
        game->dirty = false;
        wiiu_draw_options(game);
        return;
    }

    if (game->screen == SCREEN_WARNING) {
        pc_warning_fidelity_render(game);
        return;
    }
    if (game->screen == SCREEN_NIGHT_INTRO) {
        pc_night_intro_fidelity_render(game);
        return;
    }

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
    /* game_init mounts storage through progress_save_init; load the separate
     * Wii U control preference only after that mount exists. */
    wiiu_controls_init();
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
