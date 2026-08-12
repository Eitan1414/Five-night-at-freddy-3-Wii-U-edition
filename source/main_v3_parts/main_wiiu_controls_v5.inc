/* Authentic hidden-camera touch triggers for IDENTICAL mode.
 *
 * The main MFA camera workspace starts at X=1024. Hidden trigger instances
 * therefore convert to the original 825x650 camera image by subtracting 1024
 * from X and retaining Y. The stable Wii U feed stretches that source image to
 * PC_CAMERA_FEED_W x PC_CAMERA_FEED_H, so these centres can be transformed into
 * DRC coordinates without inventing touch locations.
 */

typedef struct WiiUCameraSecretTouchState {
    uint8_t mangle_step;
    bool bb_tap_armed;
    uint32_t bb_tap_frame;
    bool puppet_tap_armed;
    uint32_t puppet_tap_frame;
} WiiUCameraSecretTouchState;

static WiiUCameraSecretTouchState sWiiUCameraSecretTouch;

#define MFA_CAMERA_WORKSPACE_X 1024
#define MFA_CAMERA_SOURCE_W 825
#define MFA_CAMERA_SOURCE_H 650
#define WIIU_DOUBLE_TAP_FRAMES 30u

/* Recovered MFA instance centres. */
static const MapPoint kMfaMangleArcadeButtons[4] = {
    {1534, 345}, /* arcade button   : top-left */
    {1530, 364}, /* arcade button 3 : bottom-left */
    {1557, 339}, /* arcade button 2 : top-right */
    {1554, 359}, /* arcade button 4 : bottom-right */
};
static const MapPoint kMfaBbDouble = {1217, 324};
static const MapPoint kMfaPuppetDouble = {1809, 241};

static MapPoint wiiu_v5_camera_source_to_drc(MapPoint source)
{
    MapPoint point;
    int camera_x = source.x - MFA_CAMERA_WORKSPACE_X;
    if (camera_x < 0) camera_x = 0;
    if (camera_x > MFA_CAMERA_SOURCE_W) camera_x = MFA_CAMERA_SOURCE_W;

    int camera_y = source.y;
    if (camera_y < 0) camera_y = 0;
    if (camera_y > MFA_CAMERA_SOURCE_H) camera_y = MFA_CAMERA_SOURCE_H;

    point.x = PC_CAMERA_FEED_X +
        (camera_x * PC_CAMERA_FEED_W) / MFA_CAMERA_SOURCE_W;
    point.y = PC_CAMERA_FEED_Y +
        (camera_y * PC_CAMERA_FEED_H) / MFA_CAMERA_SOURCE_H;
    return point;
}

static bool wiiu_v5_touch_near_camera_source(const InputState *input,
                                              MapPoint source,
                                              int tolerance_x,
                                              int tolerance_y)
{
    if (input == NULL || !input->touch_valid || !input->touch_pressed)
        return false;
    const MapPoint point = wiiu_v5_camera_source_to_drc(source);
    return input->touch_x >= point.x - tolerance_x &&
           input->touch_x <= point.x + tolerance_x &&
           input->touch_y >= point.y - tolerance_y &&
           input->touch_y <= point.y + tolerance_y;
}

static bool wiiu_v5_camera_secret_allowed(const Game *game)
{
    return wiiu_controls_mode() == WIIU_CONTROL_MIRROR &&
           game->screen == SCREEN_OFFICE && game->camera_open &&
           !game->vent_map_open && !game->camera_failed &&
           !sSecret.active && !phantoms_blocks_input(&game->phantoms);
}

static bool wiiu_v5_double_tap(Game *game,
                               const InputState *input,
                               MapPoint source,
                               bool *armed,
                               uint32_t *first_frame)
{
    if (*armed && game->night_frames - *first_frame > WIIU_DOUBLE_TAP_FRAMES)
        *armed = false;

    if (!wiiu_v5_touch_near_camera_source(input, source, 18, 18))
        return false;

    if (*armed && game->night_frames - *first_frame <= WIIU_DOUBLE_TAP_FRAMES) {
        *armed = false;
        return true;
    }

    *armed = true;
    *first_frame = game->night_frames;
    content_play_select();
    game->dirty = true;
    return false;
}

static int wiiu_v5_mangle_touched_button(const InputState *input)
{
    if (input == NULL || !input->touch_valid || !input->touch_pressed)
        return -1;

    /* The original four mouse targets are only a few DRC pixels apart after
     * the 825x650 feed is reduced to 432x240. Use the closest exact MFA centre
     * inside one fingertip-sized cluster instead of overlapping four boxes. */
    int best = -1;
    int best_distance = 0x7FFFFFFF;
    for (int index = 0; index < 4; ++index) {
        const MapPoint point =
            wiiu_v5_camera_source_to_drc(kMfaMangleArcadeButtons[index]);
        const int dx = input->touch_x - point.x;
        const int dy = input->touch_y - point.y;
        if (dx < -18 || dx > 18 || dy < -18 || dy > 18)
            continue;
        const int distance = dx * dx + dy * dy;
        if (distance < best_distance) {
            best_distance = distance;
            best = index;
        }
    }
    return best;
}

static bool wiiu_v5_try_mangle_touch(Game *game, const InputState *input)
{
    if (!wiiu_v5_camera_secret_allowed(game) ||
        game->night < 2 || game->selected_camera != 6) {
        sWiiUCameraSecretTouch.mangle_step = 0u;
        return false;
    }

    const int touched = wiiu_v5_mangle_touched_button(input);
    if (touched < 0) return false;

    /* Array order is already the PC sequence TL -> BL -> TR -> BR. */
    if ((uint8_t)touched == sWiiUCameraSecretTouch.mangle_step) {
        ++sWiiUCameraSecretTouch.mangle_step;
        sSecretTriggers.mangle_step = sWiiUCameraSecretTouch.mangle_step;
        content_play_select();
        if (sWiiUCameraSecretTouch.mangle_step >= 4u) {
            sWiiUCameraSecretTouch.mangle_step = 0u;
            sSecretTriggers.mangle_step = 0u;
            complete_start_hidden_minigame(game, SECRET_MINIGAME_MANGLE);
            return true;
        }
    } else {
        sWiiUCameraSecretTouch.mangle_step = 0u;
        sSecretTriggers.mangle_step = 0u;
    }
    game->dirty = true;
    return false;
}

static bool wiiu_v5_try_bb_touch(Game *game, const InputState *input)
{
    if (!wiiu_v5_camera_secret_allowed(game) || game->selected_camera != 7) {
        sWiiUCameraSecretTouch.bb_tap_armed = false;
        return false;
    }

    if (wiiu_v5_double_tap(game, input, kMfaBbDouble,
                           &sWiiUCameraSecretTouch.bb_tap_armed,
                           &sWiiUCameraSecretTouch.bb_tap_frame)) {
        complete_start_hidden_minigame(game, SECRET_MINIGAME_BB);
        return true;
    }
    return false;
}

static bool wiiu_v5_try_puppet_touch(Game *game, const InputState *input)
{
    if (!wiiu_v5_camera_secret_allowed(game) || game->selected_camera != 2) {
        sWiiUCameraSecretTouch.puppet_tap_armed = false;
        return false;
    }

    if (wiiu_v5_double_tap(game, input, kMfaPuppetDouble,
                           &sWiiUCameraSecretTouch.puppet_tap_armed,
                           &sWiiUCameraSecretTouch.puppet_tap_frame)) {
        complete_start_hidden_minigame(game, SECRET_MINIGAME_HAPPIEST_DAY);
        return true;
    }
    return false;
}

static void wiiu_control_update_game_v5(Game *game, const InputState *input)
{
    if (wiiu_v5_try_mangle_touch(game, input)) return;
    if (wiiu_v5_try_bb_touch(game, input)) return;
    if (wiiu_v5_try_puppet_touch(game, input)) return;
    wiiu_control_update_game_v4(game, input);
}
