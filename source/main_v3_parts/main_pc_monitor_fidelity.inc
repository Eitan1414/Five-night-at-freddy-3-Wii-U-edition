/*
 * Retail-PC monitor presentation pass.
 *
 * The legacy Wii U layer remains responsible for controller navigation and
 * the extracted Clickteam assets, but this final renderer removes debug-like
 * status panels that leaked AI outcomes and keeps the game-owned PC monitor
 * art in control of the screen.  Controller hints are intentionally retained
 * as a small Wii U adaptation.
 */

typedef struct PcMonitorFidelityState {
    bool camera_error_static_looping;
} PcMonitorFidelityState;

static PcMonitorFidelityState sPcMonitorFidelity;

static void pc_monitor_draw_selected_camera(const Game *game)
{
    if (game->selected_camera < 0 || game->selected_camera >= CAMERA_COUNT)
        return;

    const MapPoint point = kOriginalCameraUiPoints[game->selected_camera];
    const uint32_t selected = GRAPHICS_RGB(194, 255, 77);

    /* The PC map artwork already owns the camera labels.  Only add the
     * selection outline required for D-pad navigation instead of painting a
     * second set of synthetic C01..C10 buttons over the extracted art. */
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        point.x - 22, point.y - 11, 44, 22, 2,
                        selected);
}

static void pc_monitor_draw_camera_map(const Game *game)
{
    const OriginalUiTextureId map_id =
        ((game->night_frames / 12u) & 1u)
            ? ORIGINAL_UI_CAMERA_MAP_ALT
            : ORIGINAL_UI_CAMERA_MAP_BASE;

    original_ui_draw_texture(map_id, GRAPHICS_TARGET_GAMEPAD,
                             591, 177, 252, 235);
    pc_monitor_draw_selected_camera(game);
    original_ui_draw_radar(game);
}

static void pc_monitor_draw_small_help(bool vent_map)
{
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       22, 455, 1,
                       vent_map
                           ? "D-PAD SELECT   A SEAL   R MAP   X/Y CLOSE"
                           : "D-PAD CAMERA   A AUDIO   R MAP   X/Y CLOSE",
                       GRAPHICS_RGB(102, 120, 100));
}

static void pc_monitor_draw_audio_state(const Game *game)
{
    /* Do not reveal lure success/ignored/target state: those prototype
     * messages exposed information the retail game never gives the player. */
    if (game->audio_failed) {
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           567, 78, 1, "AUDIO ERROR", COLOUR_RED);
    } else if (game->audio_lure_active) {
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           567, 78, 1, "PLAYING...", COLOUR_GREEN);
    }
}

static void pc_monitor_draw_signal_loss(const Game *game)
{
    original_ui_draw_static(game, false);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       195, 192, 3, "VIDEO ERROR", COLOUR_RED);
}

static void original_ui_draw_camera_feed(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(0, 2, 1));
    original_ui_draw_toolbar();

    if (sFinishing.phone_active) {
        original_ui_draw_texture(ORIGINAL_UI_MUTE_CALL,
                                 GRAPHICS_TARGET_GAMEPAD,
                                 306, 100, 94, 22);
    }

    /* The final PC camera layer draws the verified 532x295 feed/composites. */
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       28, 65, 532, 295, COLOUR_BLACK);
    if (game->camera_failed)
        pc_monitor_draw_signal_loss(game);
    else
        pc_camera_fidelity_redraw_viewport(game);

    pc_monitor_draw_camera_map(game);
    monitor_v2_draw_controls();
    pc_monitor_draw_audio_state(game);
    pc_monitor_draw_small_help(false);

    /* Camera changes/toggles use the extracted three-frame static burst. */
    if (sOriginalUi.static_flash_frames > 0u)
        original_ui_draw_static(game, true);
}

static void pc_monitor_draw_vent_selector(const Game *game)
{
    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const int y = 150 + vent * 48;
        const bool selected = vent == (int) game->selected_vent;
        const bool sealed = vent == (int) game->sealed_vent;
        const uint32_t colour = selected
            ? GRAPHICS_RGB(194, 255, 77)
            : GRAPHICS_RGB(176, 193, 172);

        if (selected) {
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               34, y, 2, ">", colour);
        }

        char label[16];
        snprintf(label, sizeof(label), "CAM %02d", 11 + vent);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           66, y, 2, label, colour);

        if (sealed) {
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               185, y + 3, 1, "SEALED",
                               GRAPHICS_RGB(194, 255, 77));
        }
    }
}

static void pc_monitor_draw_seal_control(const Game *game)
{
    const uint32_t border = game->ventilation_failed
        ? COLOUR_RED : GRAPHICS_RGB(190, 210, 186);
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        555, 9, 118, 64, 2, border);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       571, 33, 2,
                       game->ventilation_failed ? "ERROR" : "SEAL VENT",
                       border);

    const TextureRle *toggle = monitor_v2_texture(MONITOR_V2_MAP_TOGGLE);
    if (toggle != NULL) {
        texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                         699, 7, 140, 70, toggle);
    }
}

static void original_ui_draw_vent_map(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(0, 2, 1));
    original_ui_draw_toolbar();

    const OriginalUiTextureId map_id =
        ((game->night_frames / 12u) & 1u)
            ? ORIGINAL_UI_VENT_MAP_ALT
            : ORIGINAL_UI_VENT_MAP_BASE;

    /* Preserve the extracted PC vent map at its original Wii U fit.  The old
     * giant framed list is replaced by a compact controller selector, leaving
     * the majority of the panel to the game-owned map artwork. */
    original_ui_draw_texture(map_id, GRAPHICS_TARGET_GAMEPAD,
                             397, 68, 430, 401);
    pc_monitor_draw_vent_selector(game);
    pc_monitor_draw_seal_control(game);
    pc_monitor_draw_small_help(true);

    if (sOriginalUi.static_flash_frames > 0u)
        original_ui_draw_static(game, true);
}

static void pc_monitor_fidelity_update_game(Game *game,
                                            const InputState *input)
{
    const GameScreen before_screen = game->screen;
    const bool before_camera = game->camera_open;
    const bool before_maintenance = game->maintenance_open;
    const bool before_vent_map = game->vent_map_open;
    const bool before_camera_failed = game->camera_failed;
    const int before_camera_index = game->selected_camera;
    const SpringtrapVent before_vent = game->selected_vent;

    pc_system_fidelity_update_game(game, input);

    if (before_screen != SCREEN_OFFICE || game->screen != SCREEN_OFFICE) {
        if (sPcMonitorFidelity.camera_error_static_looping) {
            audio_stop(AUDIO_CUE_STATIC);
            sPcMonitorFidelity.camera_error_static_looping = false;
        }
        return;
    }

    /* The original PC crank/lever samples were already embedded but were not
     * connected to the normal night UI.  Direct panel-to-panel switches use
     * the sound of the panel being raised, avoiding two overlapping samples. */
    if (!before_camera && game->camera_open) {
        audio_restart(AUDIO_CUE_CAMERA_OPEN, 0.82f, false);
    } else if (before_camera && !game->camera_open && !game->maintenance_open) {
        audio_restart(AUDIO_CUE_CAMERA_CLOSE, 0.82f, false);
    }

    if (!before_maintenance && game->maintenance_open) {
        audio_restart(AUDIO_CUE_MAINTENANCE_OPEN, 0.82f, false);
    } else if (before_maintenance && !game->maintenance_open &&
               !game->camera_open) {
        audio_restart(AUDIO_CUE_MAINTENANCE_CLOSE, 0.82f, false);
    }

    const bool camera_burst = game->camera_open && !game->camera_failed &&
        (!before_camera || before_vent_map != game->vent_map_open ||
         before_camera_index != game->selected_camera ||
         before_vent != game->selected_vent);

    if (camera_burst) {
        /* Override the old loop=true restart performed by legacy navigation.
         * Static on a healthy monitor is a switching burst, not a permanent
         * audio bed for the entire time the monitor is raised. */
        audio_restart(AUDIO_CUE_STATIC, 0.34f, false);
    }

    if (game->camera_open && game->camera_failed) {
        if (!sPcMonitorFidelity.camera_error_static_looping ||
            !before_camera_failed) {
            audio_restart(AUDIO_CUE_STATIC, 0.27f, true);
            sPcMonitorFidelity.camera_error_static_looping = true;
        }
    } else if (sPcMonitorFidelity.camera_error_static_looping) {
        audio_stop(AUDIO_CUE_STATIC);
        sPcMonitorFidelity.camera_error_static_looping = false;
    }
}
