/* Source-faithful touch translation pass.
 *
 * Touch controls must enter the same legacy/MFA event path as physical buttons.
 * Directly calling trigger_audio_lure/start_repair would bypass wrappers that
 * observe button-driven state transitions (audio health, scanner SFX, etc.).
 */

static void wiiu_v2_clear_panel_buttons(InputState *filtered,
                                        bool maintenance)
{
    uint32_t mask = GAME_BUTTON_UP | GAME_BUTTON_DOWN |
                    GAME_BUTTON_LEFT | GAME_BUTTON_RIGHT |
                    GAME_BUTTON_CONFIRM;
    if (maintenance)
        mask |= GAME_BUTTON_START;
    filtered->held &= ~mask;
    filtered->pressed &= ~mask;
    filtered->released &= ~mask;
}

static void wiiu_v2_touch_camera(Game *game,
                                 const InputState *input,
                                 InputState *filtered)
{
    if (!input->touch_pressed || !input->touch_valid) return;

    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const MapPoint point = kTouchCameraPoints[camera];
        if (input->touch_x >= point.x - 42 &&
            input->touch_x < point.x + 42 &&
            input->touch_y >= point.y - 23 &&
            input->touch_y < point.y + 23) {
            if (game->selected_camera != camera) {
                game->selected_camera = camera;
                sOriginalUi.static_flash_frames = 8u;
                restart_camera_static();
                game->dirty = true;
            }
            return;
        }
    }

    if (wiiu_touch_in(input, 38, 423, 230, 44)) {
        filtered->pressed |= GAME_BUTTON_CONFIRM;
        return;
    }
    if (wiiu_touch_in(input, 312, 423, 230, 44)) {
        filtered->pressed |= GAME_BUTTON_MODE;
        return;
    }
    if (wiiu_touch_in(input, 586, 423, 230, 44)) {
        filtered->pressed |= GAME_BUTTON_PANEL;
    }
}

static void wiiu_v2_touch_vent(Game *game,
                               const InputState *input,
                               InputState *filtered)
{
    if (!input->touch_pressed || !input->touch_valid) return;

    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const int y = 82 + vent * 58;
        if (wiiu_touch_in(input, 32, y, 250, 46)) {
            game->selected_vent = (SpringtrapVent)vent;
            game->dirty = true;
            return;
        }
    }

    if (wiiu_touch_in(input, 32, 391, 250, 38)) {
        filtered->pressed |= GAME_BUTTON_CONFIRM;
        return;
    }
    if (wiiu_touch_in(input, 330, 435, 210, 34)) {
        filtered->pressed |= GAME_BUTTON_MODE;
        return;
    }
    if (wiiu_touch_in(input, 610, 435, 210, 34)) {
        filtered->pressed |= GAME_BUTTON_PANEL;
    }
}

static void wiiu_v2_touch_maintenance(Game *game,
                                      const InputState *input,
                                      InputState *filtered)
{
    static const int items[4] = {
        MAINT_AUDIO, MAINT_CAMERA, MAINT_VENTILATION, MAINT_REBOOT_ALL
    };
    if (!input->touch_pressed || !input->touch_valid || game->repairing) return;

    for (int row = 0; row < 4; ++row) {
        const int y = 108 + row * 70;
        if (wiiu_touch_in(input, 125, y, 604, 52)) {
            game->maintenance_selection = items[row];
            filtered->pressed |= GAME_BUTTON_CONFIRM;
            return;
        }
    }

    if (wiiu_touch_in(input, 277, 411, 300, 46))
        filtered->pressed |= GAME_BUTTON_BACK;
}

static void wiiu_v2_filter_touch_mode(Game *game,
                                      const InputState *input,
                                      InputState *filtered)
{
    if (wiiu_controls_mode() != WIIU_CONTROL_TOUCH_GAMEPAD ||
        game->screen != SCREEN_OFFICE) {
        return;
    }

    if (game->maintenance_open) {
        wiiu_v2_clear_panel_buttons(filtered, true);
        wiiu_v2_touch_maintenance(game, input, filtered);
    } else if (game->camera_open) {
        wiiu_v2_clear_panel_buttons(filtered, false);
        if (game->vent_map_open)
            wiiu_v2_touch_vent(game, input, filtered);
        else
            wiiu_v2_touch_camera(game, input, filtered);
    }
}

static void wiiu_control_update_game_v2(Game *game, const InputState *input)
{
    if (sWiiUOptionsOpen) {
        wiiu_options_update(game, input);
        return;
    }

    if (game->screen == SCREEN_TITLE &&
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sWiiUOptionsOpen = true;
        sWiiUOptionsSelection = 0;
        game->dirty = true;
        return;
    }

    InputState filtered = *input;
    wiiu_v2_filter_touch_mode(game, input, &filtered);
    pc_system_fidelity_update_game(game, &filtered);
}
