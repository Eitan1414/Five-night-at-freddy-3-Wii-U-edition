/* Real-Wii-U hardware feedback pass.
 *
 * This layer intentionally sits after the existing Wii U control adapters. It
 * fixes only behaviour observed on hardware: office pan direction, contextual
 * panel opening, and the TOUCH GamePad presentation. The underlying PC/MFA
 * gameplay state is unchanged.
 */

/* Defined later by main_original_ui_03.inc. */
static void original_ui_draw_maintenance(const Game *game);

static void wiiu_v9_swap_bits(uint32_t *bits, uint32_t a, uint32_t b)
{
    const bool has_a = (*bits & a) != 0u;
    const bool has_b = (*bits & b) != 0u;
    *bits &= ~(a | b);
    if (has_a) *bits |= b;
    if (has_b) *bits |= a;
}

static void wiiu_v9_fix_office_pan(Game *game, InputState *input)
{
    if (game == NULL || input == NULL ||
        game->screen != SCREEN_OFFICE || game->camera_open ||
        game->maintenance_open || phantoms_blocks_input(&game->phantoms)) {
        return;
    }

    /* The legacy office renderer moves the panorama texture, not the camera.
     * Consequently its LEFT/RIGHT signs are visually reversed on hardware.
     * Swap them only while freely looking around the office; title/camera maps
     * retain the normal D-pad directions. */
    wiiu_v9_swap_bits(&input->held, GAME_BUTTON_LEFT, GAME_BUTTON_RIGHT);
    wiiu_v9_swap_bits(&input->pressed, GAME_BUTTON_LEFT, GAME_BUTTON_RIGHT);
    wiiu_v9_swap_bits(&input->released, GAME_BUTTON_LEFT, GAME_BUTTON_RIGHT);
}

static void wiiu_v9_contextual_panel(Game *game, InputState *input)
{
    if (game == NULL || input == NULL ||
        game->screen != SCREEN_OFFICE || game->camera_open ||
        game->maintenance_open || phantoms_blocks_input(&game->phantoms)) {
        return;
    }

    /* After the pan correction, positive office_pan is the player's left side.
     * X/Y therefore opens System Restart while looking left, and the monitor
     * while looking centre/right. Minus remains a direct maintenance shortcut. */
    if (game->office_pan < 24) return;

    const bool held = (input->held & GAME_BUTTON_PANEL) != 0u;
    const bool pressed = (input->pressed & GAME_BUTTON_PANEL) != 0u;
    const bool released = (input->released & GAME_BUTTON_PANEL) != 0u;
    input->held &= ~GAME_BUTTON_PANEL;
    input->pressed &= ~GAME_BUTTON_PANEL;
    input->released &= ~GAME_BUTTON_PANEL;
    if (held) input->held |= GAME_BUTTON_SELECT;
    if (pressed) input->pressed |= GAME_BUTTON_SELECT;
    if (released) input->released |= GAME_BUTTON_SELECT;
}

static bool wiiu_v9_touch_rect(const InputState *input,
                               int x, int y, int w, int h)
{
    return input != NULL && input->touch_valid && input->touch_pressed &&
           input->touch_x >= x && input->touch_x < x + w &&
           input->touch_y >= y && input->touch_y < y + h;
}

static void wiiu_v9_remap_authentic_touch(Game *game, InputState *input)
{
    if (game == NULL || input == NULL ||
        wiiu_controls_mode() != WIIU_CONTROL_TOUCH_GAMEPAD ||
        game->screen != SCREEN_OFFICE ||
        !input->touch_valid || !input->touch_pressed) {
        return;
    }

    if (game->maintenance_open) {
        /* Match the five visible PC System Restart rows. V2 consumes the first
         * four via its old large touch rows and EXIT via its separate hitbox. */
        for (int row = 0; row < 5; ++row) {
            const int y = 127 + row * 50;
            if (!wiiu_v9_touch_rect(input, 238, y - 4, 548, 42)) continue;
            if (row < 4) {
                input->touch_x = 200;
                input->touch_y = 108 + row * 70 + 24;
            } else {
                input->touch_x = 400;
                input->touch_y = 430;
            }
            return;
        }
        return;
    }

    if (!game->camera_open) return;

    if (game->vent_map_open) {
        for (int vent = 0; vent < VENT_COUNT; ++vent) {
            const PcVentMapUiNode *node = &kPcVentMapUiNodes[vent];
            if (!wiiu_v9_touch_rect(input,
                                     node->button_x - 5,
                                     node->button_y - 5,
                                     node->button_w + 10,
                                     node->button_h + 10)) {
                continue;
            }
            input->touch_x = 120;
            input->touch_y = 82 + vent * 58 + 22;
            return;
        }

        /* Authentic Map Toggle button. */
        if (wiiu_v9_touch_rect(input, 349, 382, 140, 70)) {
            input->touch_x = 400;
            input->touch_y = 450;
        }
        return;
    }

    /* Camera buttons: use the exact PC map positions rendered on the DRC, then
     * translate to the historical V2 centres so the existing gameplay path is
     * still the one that performs the camera change. */
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const MapPoint point = kOriginalCameraUiPoints[camera];
        if (input->touch_x < point.x - 28 || input->touch_x > point.x + 28 ||
            input->touch_y < point.y - 18 || input->touch_y > point.y + 18) {
            continue;
        }
        input->touch_x = kTouchCameraPoints[camera].x;
        input->touch_y = kTouchCameraPoints[camera].y;
        return;
    }

    /* Authentic PC Play Audio and Map Toggle sprites. */
    if (wiiu_v9_touch_rect(input, 360, 312, 118, 64)) {
        input->touch_x = 100;
        input->touch_y = 440;
        return;
    }
    if (wiiu_v9_touch_rect(input, 349, 382, 140, 70)) {
        input->touch_x = 400;
        input->touch_y = 440;
    }
}

static void wiiu_control_update_game_v9(Game *game, const InputState *input)
{
    if (game == NULL || input == NULL) {
        wiiu_control_update_game_v8(game, input);
        return;
    }

    InputState corrected = *input;
    wiiu_v9_fix_office_pan(game, &corrected);
    wiiu_v9_contextual_panel(game, &corrected);
    wiiu_v9_remap_authentic_touch(game, &corrected);
    wiiu_control_update_game_v8(game, &corrected);
}

static void wiiu_v9_draw_office_panel_hints(const Game *game,
                                             uint32_t targets)
{
    if (game == NULL || game->screen != SCREEN_OFFICE ||
        game->camera_open || game->maintenance_open ||
        phantoms_blocks_input(&game->phantoms)) {
        return;
    }

    const uint32_t dim = GRAPHICS_RGB(92, 117, 91);
    const uint32_t bright = GRAPHICS_RGB(194, 255, 77);
    const bool left_side = game->office_pan >= 24;
    const bool right_side = game->office_pan <= -24;

    /* Small edge tabs provide the missing console affordance without covering
     * the PC office art. X/Y follows whichever side is currently in view. */
    graphics_draw_rect(targets, 4, 432, 124, 28,
                       GRAPHICS_RGB(0, 2, 1));
    graphics_draw_frame(targets, 4, 432, 124, 28, 1,
                        left_side ? bright : dim);
    graphics_draw_text(targets, 13, 442, 1,
                       "X: SYSTEM", left_side ? bright : dim);

    graphics_draw_rect(targets, 726, 432, 124, 28,
                       GRAPHICS_RGB(0, 2, 1));
    graphics_draw_frame(targets, 726, 432, 124, 28, 1,
                        right_side ? bright : dim);
    graphics_draw_text(targets, 737, 442, 1,
                       "X: MONITOR", right_side ? bright : dim);
}

static void wiiu_control_draw_gamepad_v9(const Game *game)
{
    if (game == NULL) return;

    const WiiUControlMode mode = wiiu_controls_mode();
    if (mode == WIIU_CONTROL_MIRROR) return;

    if (mode == WIIU_CONTROL_TV_ONLY || game->screen != SCREEN_OFFICE) {
        graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
        return;
    }

    /* TOUCH mode now shows the actual PC panel instead of the oversized custom
     * Wii U map/PLAY AUDIO/VENT MAP/CLOSE interface. */
    if (game->maintenance_open || game->camera_open) {
        const uint32_t previous_targets = sOriginalUiPanelTargets;
        sOriginalUiPanelTargets = GRAPHICS_TARGET_GAMEPAD;

        if (game->maintenance_open) {
            original_ui_draw_maintenance(game);
        } else if (game->vent_map_open) {
            original_ui_draw_vent_map(game);
            monitor_v2_draw_vent_controls();
        } else {
            original_ui_draw_camera_feed(game);
        }

        if (sMonitorV2.transition_active)
            monitor_v2_draw_transition();

        sOriginalUiPanelTargets = previous_targets;
        return;
    }

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
}

#undef wiiu_control_update_game_v6
#define wiiu_control_update_game_v6 wiiu_control_update_game_v9
#undef wiiu_control_draw_gamepad
#define wiiu_control_draw_gamepad wiiu_control_draw_gamepad_v9
