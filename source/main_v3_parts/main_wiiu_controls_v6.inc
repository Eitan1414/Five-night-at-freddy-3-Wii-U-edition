/* Exact Shadow Cupcake touch collection for IDENTICAL mode.
 *
 * The four layouts and source rectangles come from the MFA Create actions and
 * image-bank hotspot metadata in main_pc_camera_fidelity.inc. This layer only
 * adapts the PC mouse click to the Wii U resistive touchscreen; it does not
 * invent alternate locations or bypass the existing secret-minigame state.
 */

static bool wiiu_v6_shadow_cupcake_touch_allowed(const Game *game)
{
    return wiiu_controls_mode() == WIIU_CONTROL_MIRROR &&
           game->screen == SCREEN_OFFICE && game->night == 3 &&
           game->camera_open && !game->vent_map_open &&
           !game->camera_failed && !sSecret.active &&
           !phantoms_blocks_input(&game->phantoms);
}

static bool wiiu_v6_try_shadow_cupcake_touch(Game *game,
                                             const InputState *input)
{
    if (!wiiu_v6_shadow_cupcake_touch_allowed(game) ||
        input == NULL || !input->touch_valid || !input->touch_pressed) {
        return false;
    }

    const PcShadowCupcakeLayout *cupcake =
        pc_shadow_cupcake_for_camera(game->selected_camera);
    if (!pc_shadow_cupcake_is_visible(game, cupcake))
        return false;

    int x = 0, y = 0, w = 0, h = 0;
    pc_shadow_cupcake_drc_rect(cupcake, &x, &y, &w, &h);

    /* The PC condition is a mouse click directly on the Active. The source
     * sprites become only ~16-32 px wide on the 854x480 DRC because the camera
     * feed itself is reduced. Expand by 8 DRC pixels around the exact sprite
     * rectangle solely as a resistive-touch accessibility adaptation. */
    const int pad = 8;
    if (input->touch_x < x - pad || input->touch_x >= x + w + pad ||
        input->touch_y < y - pad || input->touch_y >= y + h + pad) {
        return false;
    }

    sSecretTriggers.cupcake_mask |= cupcake->collected_bit;
    content_play_select();
    game->dirty = true;

    /* The MFA destroys the clicked cupcake. Once all four independent camera
     * objects have been collected, enter Chica's Party through the same hidden
     * minigame route used by the controller fallback. */
    if ((sSecretTriggers.cupcake_mask & 0x0Fu) == 0x0Fu) {
        sSecretTriggers.cupcake_mask = 0u;
        complete_start_hidden_minigame(game, SECRET_MINIGAME_CHICA);
    }
    return true;
}

static bool wiiu_v6_update_pc_victory_flow(Game *game)
{
    if (game == NULL || game->screen != SCREEN_VICTORY)
        return false;

    /* Retail PC has no `NIGHT COMPLETE / press A` gate after the 5:59 -> 6:00
     * clock. Keep the existing 180-frame compatibility duration until the raw
     * Clickteam end-of-clock event is decoded, then enter the same inter-night
     * route already used by the finishing layer. */
    ++game->screen_frames;
    if ((game->screen_frames % 6u) == 0u)
        game->dirty = true;

    if (game->screen_frames >= 180u) {
        if (game->night <= 5)
            finishing_start_minigame(game, game->night, false);
        else
            finishing_start_ending(game, FINISHING_NIGHT6_ENDING);
    }
    return true;
}

static void wiiu_control_update_game_v6(Game *game, const InputState *input)
{
    if (wiiu_v6_update_pc_victory_flow(game))
        return;
    if (wiiu_v6_try_shadow_cupcake_touch(game, input))
        return;
    wiiu_control_update_game_v5(game, input);
}
