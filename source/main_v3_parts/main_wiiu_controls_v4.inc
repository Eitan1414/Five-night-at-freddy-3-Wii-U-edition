/* Authentic hidden-office touch triggers for IDENTICAL mode.
 *
 * These are not guessed screen zones. Stage01 key instances and Shadow
 * Bonnie's hidden double-click object were recovered directly from
 * fivenights3-94.mfa in the original 2000x768 office panorama. The Wii U
 * office uses the same panorama at 1120x430, so the source points can be
 * transformed without inventing a new layout. Small touch tolerances and the
 * double-tap time window are Wii U adaptations for the resistive GamePad.
 */

typedef struct WiiUHiddenTouchState {
    uint8_t stage_step;
    bool shadow_tap_armed;
    uint32_t shadow_tap_frame;
} WiiUHiddenTouchState;

static WiiUHiddenTouchState sWiiUHiddenTouch;

static const MapPoint kMfaStage01KeyPoints[9] = {
    {875, 475}, {939, 476}, {1003, 478},
    {875, 542}, {939, 545}, {1003, 545},
    {875, 606}, {939, 611}, {1003, 614},
};

/* MFA object "double click" (handle 191), placed over the Shadow Bonnie
 * figurine on the far-right office shelf. */
static const MapPoint kMfaShadowBonnieDoubleClick = {1526, 333};

static MapPoint wiiu_v4_office_source_to_drc(const Game *game,
                                             MapPoint source)
{
    MapPoint point;
    const int texture_x = -133 + game->office_pan;
    point.x = texture_x + (source.x * 1120) / 2000;
    point.y = 24 + (source.y * 430) / 768;
    return point;
}

static bool wiiu_v4_touch_near_source(const Game *game,
                                      const InputState *input,
                                      MapPoint source,
                                      int tolerance_x,
                                      int tolerance_y)
{
    if (input == NULL || !input->touch_valid || !input->touch_pressed)
        return false;
    const MapPoint point = wiiu_v4_office_source_to_drc(game, source);
    return input->touch_x >= point.x - tolerance_x &&
           input->touch_x <= point.x + tolerance_x &&
           input->touch_y >= point.y - tolerance_y &&
           input->touch_y <= point.y + tolerance_y;
}

static int wiiu_v4_stage01_touched_key(const Game *game,
                                       const InputState *input)
{
    /* 18 logical DRC pixels ~= 32 pixels in the original panorama. The source
     * Active digits themselves are tiny mouse targets; this tolerance keeps the
     * same centres while making them usable with a fingertip. */
    for (int index = 0; index < 9; ++index) {
        if (wiiu_v4_touch_near_source(game, input,
                                     kMfaStage01KeyPoints[index], 18, 18))
            return index + 1;
    }
    return 0;
}

static bool wiiu_v4_hidden_touch_allowed(const Game *game)
{
    return wiiu_controls_mode() == WIIU_CONTROL_MIRROR &&
           game->screen == SCREEN_OFFICE &&
           !game->camera_open && !game->maintenance_open &&
           !sSecret.active && !phantoms_blocks_input(&game->phantoms);
}

static bool wiiu_v4_try_shadow_touch(Game *game, const InputState *input)
{
    if (!wiiu_v4_hidden_touch_allowed(game) || game->night < 5) {
        sWiiUHiddenTouch.shadow_tap_armed = false;
        return false;
    }

    /* Expire a lone first tap after 30 source frames (~500 ms at 60 Hz). The
     * PC object is a mouse double-click condition; only the timing window is a
     * platform adaptation, while the hotspot centre is the MFA instance. */
    if (sWiiUHiddenTouch.shadow_tap_armed &&
        game->night_frames - sWiiUHiddenTouch.shadow_tap_frame > 30u) {
        sWiiUHiddenTouch.shadow_tap_armed = false;
    }

    if (!wiiu_v4_touch_near_source(game, input,
                                   kMfaShadowBonnieDoubleClick, 24, 24))
        return false;

    if (sWiiUHiddenTouch.shadow_tap_armed &&
        game->night_frames - sWiiUHiddenTouch.shadow_tap_frame <= 30u) {
        sWiiUHiddenTouch.shadow_tap_armed = false;
        complete_start_hidden_minigame(game, SECRET_MINIGAME_SHADOW_BONNIE);
        return true;
    }

    sWiiUHiddenTouch.shadow_tap_armed = true;
    sWiiUHiddenTouch.shadow_tap_frame = game->night_frames;
    content_play_select();
    game->dirty = true;
    return false;
}

static bool wiiu_v4_try_stage01_touch(Game *game, const InputState *input)
{
    if (!wiiu_v4_hidden_touch_allowed(game) || game->night < 4)
        return false;

    const int key = wiiu_v4_stage01_touched_key(game, input);
    if (key == 0) return false;

    static const uint8_t code[6] = {3u, 9u, 5u, 2u, 4u, 8u};
    complete_reset_trigger_sequences(game->night);

    if ((uint8_t)key == code[sWiiUHiddenTouch.stage_step]) {
        ++sWiiUHiddenTouch.stage_step;
        sSecretTriggers.stage_step = sWiiUHiddenTouch.stage_step;
        content_play_select();
        if (sWiiUHiddenTouch.stage_step >= 6u) {
            sWiiUHiddenTouch.stage_step = 0u;
            sSecretTriggers.stage_step = 0u;
            complete_start_hidden_minigame(game, SECRET_MINIGAME_STAGE01);
            return true;
        }
    } else {
        sWiiUHiddenTouch.stage_step = 0u;
        sSecretTriggers.stage_step = 0u;
    }
    game->dirty = true;
    return false;
}

static void wiiu_control_update_game_v4(Game *game, const InputState *input)
{
    if (wiiu_v4_try_shadow_touch(game, input))
        return;
    if (wiiu_v4_try_stage01_touch(game, input))
        return;
    wiiu_control_update_game_v2(game, input);
}
