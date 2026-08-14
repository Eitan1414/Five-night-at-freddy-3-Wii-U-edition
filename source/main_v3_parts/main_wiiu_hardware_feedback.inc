/* Real-Wii-U hardware feedback pass.
 *
 * Keep this layer deliberately small: only fixes that were observed directly
 * on hardware belong here. Touch hit-testing is left to the proven V2-V8 path
 * until the corrected RPX has been validated on the console.
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

    /* The renderer moves the panorama texture, so the historical gameplay
     * signs make LEFT reveal the right side and vice versa. Swap directions
     * only while freely looking around the office. */
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

    /* With the corrected panorama direction, positive office_pan is the left
     * side. X/Y opens System Restart there; elsewhere it opens the monitor. */
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

static void wiiu_control_update_game_v9(Game *game, const InputState *input)
{
    if (game == NULL || input == NULL) {
        wiiu_control_update_game_v8(game, input);
        return;
    }

    InputState corrected = *input;
    wiiu_v9_fix_office_pan(game, &corrected);
    wiiu_v9_contextual_panel(game, &corrected);
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

    graphics_draw_rect(targets, 4, 432, 124, 28, GRAPHICS_RGB(0, 2, 1));
    graphics_draw_frame(targets, 4, 432, 124, 28, 1,
                        left_side ? bright : dim);
    graphics_draw_text(targets, 13, 442, 1,
                       "X: SYSTEM", left_side ? bright : dim);

    graphics_draw_rect(targets, 726, 432, 124, 28, GRAPHICS_RGB(0, 2, 1));
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

    /* TOUCH mode shows the actual PC panel instead of the oversized custom
     * PLAY AUDIO / VENT MAP / CLOSE GamePad interface. Existing V2-V8 touch
     * handling remains active underneath for this first hardware correction. */
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
