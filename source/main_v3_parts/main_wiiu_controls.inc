/* Wii U-specific display/control layer.
 *
 * TOUCH GAMEPAD: PC monitor/maintenance stays on TV while the DRC is black and
 * exposes only contextual touch controls (camera map / restart buttons).
 * TV ONLY: PC presentation stays entirely on TV and the DRC remains black.
 * IDENTICAL: the full PC presentation is rendered to TV and DRC together.
 */

static bool sWiiUOptionsOpen = false;
static int sWiiUOptionsSelection = 0;

static const MapPoint kTouchCameraPoints[CAMERA_COUNT] = {
    {304, 330}, /* CAM 01 */
    {500, 309}, /* CAM 02 */
    {629, 251}, /* CAM 03 */
    {629, 169}, /* CAM 04 */
    {478, 222}, /* CAM 05 */
    {298, 224}, /* CAM 06 */
    {298, 143}, /* CAM 07 */
    {425, 125}, /* CAM 08 */
    {504,  59}, /* CAM 09 */
    {629,  86}, /* CAM 10 */
};

static bool wiiu_touch_in(const InputState *input,
                          int x, int y, int w, int h)
{
    return input != NULL && input->touch_valid && input->touch_pressed &&
           input->touch_x >= x && input->touch_x < x + w &&
           input->touch_y >= y && input->touch_y < y + h;
}

static uint32_t wiiu_control_panel_targets(void)
{
    return wiiu_controls_mode() == WIIU_CONTROL_MIRROR
        ? GRAPHICS_TARGET_BOTH : GRAPHICS_TARGET_TV;
}

static void wiiu_draw_touch_button(int x, int y, int w, int h,
                                   const char *label, bool active,
                                   bool error)
{
    const uint32_t border = error ? COLOUR_RED
        : active ? GRAPHICS_RGB(194, 255, 77)
                 : GRAPHICS_RGB(210, 220, 210);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, x, y, w, h,
                       GRAPHICS_RGB(1, 3, 2));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, x, y, w, h, 2, border);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       x + 12, y + h / 2 - 5, 1, label, border);
}

static void wiiu_draw_touch_camera_map(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);

    /* Same extracted PC map used by the TV monitor, enlarged to the DRC. The
     * node positions follow the supplied FNaF 3 map reference. */
    const OriginalUiTextureId map_id =
        ((game->night_frames / 12u) & 1u)
            ? ORIGINAL_UI_CAMERA_MAP_ALT
            : ORIGINAL_UI_CAMERA_MAP_BASE;
    original_ui_draw_texture(map_id, GRAPHICS_TARGET_GAMEPAD,
                             172, 20, 510, 380);

    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const MapPoint point = kTouchCameraPoints[camera];
        const bool selected = camera == game->selected_camera;
        const uint32_t colour = selected
            ? GRAPHICS_RGB(194, 255, 77)
            : GRAPHICS_RGB(230, 230, 225);
        char label[12];
        snprintf(label, sizeof(label), "CAM %02d", camera + 1);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           point.x - 36, point.y - 17, 72, 34,
                           GRAPHICS_RGB(2, 3, 3));
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            point.x - 36, point.y - 17, 72, 34, 2,
                            colour);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           point.x - 28, point.y - 4, 1,
                           label, colour);
    }

    wiiu_draw_touch_button(38, 423, 230, 44, "PLAY AUDIO", false,
                           game->audio_failed);
    wiiu_draw_touch_button(312, 423, 230, 44, "VENT MAP", false, false);
    wiiu_draw_touch_button(586, 423, 230, 44, "CLOSE", false, false);
}

static void wiiu_draw_touch_vent_map(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);

    const OriginalUiTextureId map_id =
        ((game->night_frames / 10u) & 1u)
            ? ORIGINAL_UI_VENT_MAP_ALT
            : ORIGINAL_UI_VENT_MAP_BASE;
    original_ui_draw_texture(map_id, GRAPHICS_TARGET_GAMEPAD,
                             330, 27, 490, 400);

    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       32, 32, 2, "VENT CAMERAS", COLOUR_WHITE);
    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const int y = 82 + vent * 58;
        const bool selected = vent == (int)game->selected_vent;
        const bool sealed = vent == (int)game->sealed_vent;
        char label[16];
        snprintf(label, sizeof(label), "CAM %02d", 11 + vent);
        wiiu_draw_touch_button(32, y, 250, 46, label,
                               selected || sealed, false);
        if (sealed)
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               205, y + 17, 1, "SEALED",
                               GRAPHICS_RGB(194, 255, 77));
    }

    wiiu_draw_touch_button(32, 391, 250, 38, "SEAL / UNSEAL", false,
                           game->ventilation_failed);
    wiiu_draw_touch_button(330, 435, 210, 34, "CAMERA MAP", false, false);
    wiiu_draw_touch_button(610, 435, 210, 34, "CLOSE", false, false);
}

static OriginalUiTextureId wiiu_maintenance_texture(const Game *game,
                                                     int item)
{
    switch (item) {
        case MAINT_AUDIO:
            return game->audio_failed ? ORIGINAL_UI_MAINT_AUDIO_ERROR
                                      : ORIGINAL_UI_MAINT_AUDIO;
        case MAINT_CAMERA:
            return game->camera_failed ? ORIGINAL_UI_MAINT_CAMERA_ERROR
                                       : ORIGINAL_UI_MAINT_CAMERA;
        case MAINT_VENTILATION:
            return game->ventilation_failed ? ORIGINAL_UI_MAINT_VENT_ERROR
                                            : ORIGINAL_UI_MAINT_VENT;
        case MAINT_REBOOT_ALL:
        default:
            return ORIGINAL_UI_MAINT_REBOOT_ALL;
    }
}

static void wiiu_draw_touch_maintenance(const Game *game)
{
    static const int items[4] = {
        MAINT_AUDIO, MAINT_CAMERA, MAINT_VENTILATION, MAINT_REBOOT_ALL
    };

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
    original_ui_draw_texture(ORIGINAL_UI_MAINT_MENU,
                             GRAPHICS_TARGET_GAMEPAD,
                             184, 28, 486, 58);

    for (int row = 0; row < 4; ++row) {
        const int item = items[row];
        const int y = 108 + row * 70;
        const bool selected = item == game->maintenance_selection;
        const bool error = (item == MAINT_AUDIO && game->audio_failed) ||
                           (item == MAINT_CAMERA && game->camera_failed) ||
                           (item == MAINT_VENTILATION && game->ventilation_failed);
        const uint32_t border = error ? COLOUR_RED
            : selected ? GRAPHICS_RGB(194, 255, 77)
                       : GRAPHICS_RGB(105, 125, 103);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           125, y, 604, 52, GRAPHICS_RGB(1, 3, 2));
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            125, y, 604, 52, 2, border);
        original_ui_draw_texture(wiiu_maintenance_texture(game, item),
                                 GRAPHICS_TARGET_GAMEPAD,
                                 158, y + 8, 535, 34);
    }

    if (game->repairing) {
        original_ui_draw_texture(ORIGINAL_UI_MAINT_RESTART,
                                 GRAPHICS_TARGET_GAMEPAD,
                                 264, 398, 330, 31);
        const int dots = 1 + (int)((game->repair_frames / 20u) % 3u);
        for (int dot = 0; dot < dots; ++dot)
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               582 + dot * 14, 415, 7, 7, COLOUR_WHITE);
    } else {
        wiiu_draw_touch_button(277, 411, 300, 46, "EXIT", false, false);
    }
}

static void wiiu_control_draw_gamepad(const Game *game)
{
    const WiiUControlMode mode = wiiu_controls_mode();
    if (mode == WIIU_CONTROL_MIRROR) return;

    if (mode == WIIU_CONTROL_TV_ONLY || game->screen != SCREEN_OFFICE) {
        graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
        return;
    }

    if (game->maintenance_open) {
        wiiu_draw_touch_maintenance(game);
    } else if (game->camera_open) {
        if (game->vent_map_open)
            wiiu_draw_touch_vent_map(game);
        else
            wiiu_draw_touch_camera_map(game);
    } else {
        graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
    }
}

static void wiiu_touch_camera(Game *game, const InputState *input)
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
        trigger_audio_lure(game);
        return;
    }
    if (wiiu_touch_in(input, 312, 423, 230, 44)) {
        game->vent_map_open = true;
        sOriginalUi.static_flash_frames = 8u;
        restart_camera_static();
        game->dirty = true;
        return;
    }
    if (wiiu_touch_in(input, 586, 423, 230, 44)) {
        close_cameras(game);
    }
}

static void wiiu_touch_vent(Game *game, const InputState *input)
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
        if (!game->ventilation_failed) {
            game->sealed_vent = game->sealed_vent == game->selected_vent
                ? SPRINGTRAP_VENT_NONE : game->selected_vent;
            audio_play(AUDIO_CUE_WAIT, 0.6f, false);
            set_message(game, MESSAGE_VENT_SEALED);
        }
        game->dirty = true;
        return;
    }
    if (wiiu_touch_in(input, 330, 435, 210, 34)) {
        game->vent_map_open = false;
        sOriginalUi.static_flash_frames = 8u;
        restart_camera_static();
        game->dirty = true;
        return;
    }
    if (wiiu_touch_in(input, 610, 435, 210, 34)) {
        close_cameras(game);
    }
}

static void wiiu_touch_maintenance(Game *game, const InputState *input)
{
    static const int items[4] = {
        MAINT_AUDIO, MAINT_CAMERA, MAINT_VENTILATION, MAINT_REBOOT_ALL
    };
    if (!input->touch_pressed || !input->touch_valid || game->repairing) return;

    for (int row = 0; row < 4; ++row) {
        const int y = 108 + row * 70;
        if (wiiu_touch_in(input, 125, y, 604, 52)) {
            game->maintenance_selection = items[row];
            start_repair(game);
            return;
        }
    }

    if (wiiu_touch_in(input, 277, 411, 300, 46)) {
        game->maintenance_open = false;
        audio_stop(AUDIO_CUE_WAIT);
        game->dirty = true;
    }
}

static void wiiu_controls_filter_touch_mode(Game *game,
                                            const InputState *input,
                                            InputState *filtered)
{
    if (wiiu_controls_mode() != WIIU_CONTROL_TOUCH_GAMEPAD ||
        game->screen != SCREEN_OFFICE) {
        return;
    }

    if (game->maintenance_open) {
        wiiu_touch_maintenance(game, input);
        /* Selection/reboot is intentionally touch-only in this mode. */
        filtered->held &= ~(GAME_BUTTON_UP | GAME_BUTTON_DOWN |
                            GAME_BUTTON_LEFT | GAME_BUTTON_RIGHT |
                            GAME_BUTTON_CONFIRM | GAME_BUTTON_START);
        filtered->pressed &= ~(GAME_BUTTON_UP | GAME_BUTTON_DOWN |
                               GAME_BUTTON_LEFT | GAME_BUTTON_RIGHT |
                               GAME_BUTTON_CONFIRM | GAME_BUTTON_START);
    } else if (game->camera_open) {
        if (game->vent_map_open)
            wiiu_touch_vent(game, input);
        else
            wiiu_touch_camera(game, input);
        /* Camera/vent selection and Play Audio are touch-only in this mode;
         * physical B/X/Y/R remain available as harmless shortcuts. */
        filtered->held &= ~(GAME_BUTTON_UP | GAME_BUTTON_DOWN |
                            GAME_BUTTON_LEFT | GAME_BUTTON_RIGHT |
                            GAME_BUTTON_CONFIRM);
        filtered->pressed &= ~(GAME_BUTTON_UP | GAME_BUTTON_DOWN |
                               GAME_BUTTON_LEFT | GAME_BUTTON_RIGHT |
                               GAME_BUTTON_CONFIRM);
    }
}

static void wiiu_options_cycle(int direction)
{
    int mode = (int)wiiu_controls_mode();
    mode = (mode + direction + WIIU_CONTROL_MODE_COUNT) % WIIU_CONTROL_MODE_COUNT;
    wiiu_controls_set_mode((WiiUControlMode)mode);
}

static void wiiu_options_update(Game *game, const InputState *input)
{
    ++game->screen_frames;

    if (input_was_pressed(input, GAME_BUTTON_BACK) ||
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sWiiUOptionsOpen = false;
        game->dirty = true;
        return;
    }

    if (input_was_pressed(input, GAME_BUTTON_UP) ||
        input_was_pressed(input, GAME_BUTTON_DOWN)) {
        sWiiUOptionsSelection ^= 1;
        game->dirty = true;
    }

    if (sWiiUOptionsSelection == 0) {
        if (input_was_pressed(input, GAME_BUTTON_LEFT)) wiiu_options_cycle(-1);
        if (input_was_pressed(input, GAME_BUTTON_RIGHT) ||
            input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_START)) wiiu_options_cycle(1);
    } else if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
               input_was_pressed(input, GAME_BUTTON_START)) {
        sWiiUOptionsOpen = false;
    }

    if (input->touch_pressed && input->touch_valid) {
        if (wiiu_touch_in(input, 75, 198, 220, 92))
            wiiu_controls_set_mode(WIIU_CONTROL_TOUCH_GAMEPAD);
        else if (wiiu_touch_in(input, 317, 198, 220, 92))
            wiiu_controls_set_mode(WIIU_CONTROL_TV_ONLY);
        else if (wiiu_touch_in(input, 559, 198, 220, 92))
            wiiu_controls_set_mode(WIIU_CONTROL_MIRROR);
        else if (wiiu_touch_in(input, 307, 370, 240, 54))
            sWiiUOptionsOpen = false;
    }
    game->dirty = true;
}

static void wiiu_control_update_game(Game *game, const InputState *input)
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
    wiiu_controls_filter_touch_mode(game, input, &filtered);
    pc_system_fidelity_update_game(game, &filtered);
}

static void wiiu_draw_option_card(int x, const char *label,
                                  WiiUControlMode mode,
                                  const char *line1,
                                  const char *line2)
{
    const bool active = wiiu_controls_mode() == mode;
    const uint32_t border = active
        ? GRAPHICS_RGB(194, 255, 77)
        : GRAPHICS_RGB(125, 145, 122);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 198, 220, 92,
                       GRAPHICS_RGB(1, 4, 2));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, 198, 220, 92, 3, border);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 14, 216, 2, label, border);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 14, 252, 1, line1,
                       GRAPHICS_RGB(180, 195, 176));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 14, 270, 1, line2,
                       GRAPHICS_RGB(180, 195, 176));
}

static void wiiu_draw_options(Game *game)
{
    (void)game;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 235, 72, 4,
                       "WII U OPTIONS", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 287, 132, 2,
                       "PANEL CONTROL", GRAPHICS_RGB(194, 255, 77));

    wiiu_draw_option_card(75, "TOUCH", WIIU_CONTROL_TOUCH_GAMEPAD,
                          "TV: GAME", "DRC: TOUCH UI");
    wiiu_draw_option_card(317, "TV ONLY", WIIU_CONTROL_TV_ONLY,
                          "TV: GAME", "DRC: BLACK");
    wiiu_draw_option_card(559, "IDENTICAL", WIIU_CONTROL_MIRROR,
                          "TV + DRC", "SAME IMAGE");

    const uint32_t back_colour = sWiiUOptionsSelection == 1
        ? GRAPHICS_RGB(194, 255, 77) : COLOUR_WHITE;
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 307, 370, 240, 54, 2,
                        back_colour);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 385, 390, 2,
                       "BACK", back_colour);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 209, 448, 1,
                       "TOUCH A MODE OR USE LEFT/RIGHT - B/R BACK",
                       GRAPHICS_RGB(112, 132, 109));
    graphics_present(GRAPHICS_TARGET_BOTH);
}
