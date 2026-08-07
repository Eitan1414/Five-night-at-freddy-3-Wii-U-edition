/* Wii U GamePad touch layer for the original PC hidden-minigame triggers.
 *
 * The existing controller combinations remain available as fallbacks. This
 * layer only handles a fresh touch press, using the monitor's native 854x480
 * logical coordinate space supplied by input_wiiu.c.
 */

typedef struct CompleteTouchRect {
    int x;
    int y;
    int width;
    int height;
} CompleteTouchRect;

static bool complete_touch_pressed_rect(const InputState *input,
                                        CompleteTouchRect rect)
{
    return input != NULL && input->touch_pressed &&
           input->touch_x >= rect.x &&
           input->touch_x < rect.x + rect.width &&
           input->touch_y >= rect.y &&
           input->touch_y < rect.y + rect.height;
}

static bool complete_touch_in_camera_feed(const InputState *input)
{
    const CompleteTouchRect feed = {28, 65, 532, 295};
    return complete_touch_pressed_rect(input, feed);
}

static bool complete_touch_try_mangle(Game *game, const InputState *input)
{
    if (game->selected_camera != 6 || game->night < 2 ||
        !complete_touch_in_camera_feed(input)) {
        return false;
    }

    /* The PC arcade presents four buttons. Treat the camera feed as the same
     * 2x2 ordering so the original UL, DL, UR, DR sequence is natural on a
     * resistive touchscreen while the controller sequence remains intact. */
    const int split_x = 294;
    const int split_y = 213;
    const bool left = input->touch_x < split_x;
    const bool top = input->touch_y < split_y;
    int quadrant = 0;
    if (left && top) quadrant = 0;          /* upper-left */
    else if (left && !top) quadrant = 1;   /* lower-left */
    else if (!left && top) quadrant = 2;   /* upper-right */
    else quadrant = 3;                     /* lower-right */

    if (quadrant == (int) sSecretTriggers.mangle_step) {
        ++sSecretTriggers.mangle_step;
        content_play_select();
        if (sSecretTriggers.mangle_step >= 4u) {
            sSecretTriggers.mangle_step = 0u;
            complete_start_hidden_minigame(game, SECRET_MINIGAME_MANGLE);
        }
    } else {
        sSecretTriggers.mangle_step = 0u;
    }
    return true;
}

static bool complete_touch_try_chica(Game *game, const InputState *input)
{
    if (game->night < 3 || !input->touch_pressed) return false;

    static const int cameras[4] = {1, 2, 3, 5};
    /* The dark cupcakes live in the camera image rather than the map/toolbar.
     * These rectangles follow their respective floor/wall regions in the
     * 532x295 camera viewport and deliberately stay clear of CAM03's Puppet
     * drawing hotspot. */
    static const CompleteTouchRect cupcakes[4] = {
        {230, 245, 170, 110}, /* CAM02 */
        {205, 270, 180, 85},  /* CAM03 */
        {55, 245, 180, 110},  /* CAM04 */
        {355, 240, 190, 115}, /* CAM06 */
    };

    for (int index = 0; index < 4; ++index) {
        if (game->selected_camera != cameras[index] ||
            !complete_touch_pressed_rect(input, cupcakes[index])) {
            continue;
        }
        const uint8_t bit = (uint8_t) (1u << index);
        if ((sSecretTriggers.cupcake_mask & bit) == 0u) {
            sSecretTriggers.cupcake_mask |= bit;
            content_play_select();
        }
        if ((sSecretTriggers.cupcake_mask & 0x0Fu) == 0x0Fu) {
            sSecretTriggers.cupcake_mask = 0u;
            complete_start_hidden_minigame(game, SECRET_MINIGAME_CHICA);
        }
        return true;
    }
    return false;
}

static bool complete_try_touch_camera_trigger(Game *game,
                                              const InputState *input)
{
    if (!game->camera_open || game->camera_failed || game->vent_map_open ||
        input == NULL || !input->touch_pressed) {
        return false;
    }

    /* CAM08: Balloon Boy drawing is on the upper-left wall. */
    if (game->selected_camera == 7 &&
        complete_touch_pressed_rect(input, (CompleteTouchRect){40, 75, 185, 150})) {
        complete_start_hidden_minigame(game, SECRET_MINIGAME_BB);
        return true;
    }

    /* CAM03: the Puppet drawing occupies the right wall. Keep this before the
     * cupcake check because both secrets share CAM03 on Night 3. */
    if (game->selected_camera == 2 &&
        complete_touch_pressed_rect(input, (CompleteTouchRect){385, 80, 170, 165})) {
        complete_start_hidden_minigame(game, SECRET_MINIGAME_HAPPIEST_DAY);
        return true;
    }

    if (complete_touch_try_chica(game, input)) return true;
    if (complete_touch_try_mangle(game, input)) return true;
    return false;
}

static void fnaf3_touch_update_game(Game *game, const InputState *input)
{
    if (game != NULL && input != NULL &&
        game->screen == SCREEN_OFFICE && !sSecret.active &&
        !phantoms_blocks_input(&game->phantoms)) {
        complete_reset_trigger_sequences(game->night);
        if (complete_try_touch_camera_trigger(game, input) && sSecret.active)
            return;
    }

    fnaf3_complete_update_game(game, input);
}
