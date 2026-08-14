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

static void wiiu_v6_update_pc_victory_flow(Game *game,
                                           const InputState *input)
{
    if (game == NULL || input == NULL || game->screen != SCREEN_VICTORY) {
        wiiu_control_update_game_v5(game, input);
        return;
    }

    /* Retail PC has no interactive `NIGHT COMPLETE / press A` gate after the
     * 5:59 -> 6:00 clock.  Do NOT jump directly into finishing_start_* here:
     * the normal downstream update owns screen/mode transition hooks and the
     * full-audio layer detects those mode changes to start Follow Me/ending
     * ambience.  Instead, on the compatibility frame where the old Wii U code
     * would begin accepting A, feed that existing path one automatic confirm.
     * Nothing is displayed and the player does not have to press anything.
     *
     * 180 frames remains a compatibility duration until the raw Clickteam
     * end-of-clock event timing is decoded from the MFA. */
    InputState forwarded = *input;
    if (game->screen_frames >= 179u)
        forwarded.pressed |= GAME_BUTTON_CONFIRM;

    wiiu_control_update_game_v5(game, &forwarded);
}

static void wiiu_control_update_game_v6(Game *game, const InputState *input)
{
    if (wiiu_v6_try_shadow_cupcake_touch(game, input))
        return;
    wiiu_v6_update_pc_victory_flow(game, input);
}

/* The first Wii U touch layer predates delayed PC vent sealing and labels its
 * action `SEAL / UNSEAL`. V2 already translates that touch into the same
 * GAME_BUTTON_CONFIRM path as a physical controller, so gameplay is correct;
 * this final presentation wrapper only fixes the stale label and exposes the
 * pending state without changing hitboxes or input routing. */
static void wiiu_control_draw_gamepad_v6(const Game *game)
{
    wiiu_control_draw_gamepad(game);

    if (game == NULL ||
        wiiu_controls_mode() != WIIU_CONTROL_TOUCH_GAMEPAD ||
        game->screen != SCREEN_OFFICE || !game->camera_open ||
        !game->vent_map_open) {
        return;
    }

    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       30, 389, 254, 42, COLOUR_BLACK);
    wiiu_draw_touch_button(32, 391, 250, 38,
                           pc_vent_seal_is_active()
                               ? "SEALING..." : "SEAL VENT",
                           pc_vent_seal_is_active(),
                           game->ventilation_failed);
}

#define wiiu_control_draw_gamepad wiiu_control_draw_gamepad_v6

/* Final title routing pass.
 *
 * The PC visual layer already exposes distinct NEW GAME / LOAD GAME /
 * NIGHTMARE / EXTRA rows, while the historical Wii U handler still wrapped
 * three selections. Keep the retail rows intact, add one Wii U CREDITS row,
 * and make the input routing agree with what is actually rendered.
 */
static bool sWiiUTitleCreditsDirect = false;

static int wiiu_v7_title_item_count(void)
{
    return progress_save_is_night_completed(5) ? 5 : 3;
}

static void wiiu_v7_start_title_night(Game *game, int night)
{
    sWiiUTitleCreditsDirect = false;
    sFinishing.mode = FINISHING_NONE;
    finishing_stop_phone();
    content_play_select();
    content_stop_title_music();
    reset_night(game, night);
    audio_play(AUDIO_CUE_START_DAY, 0.82f, false);
}

static void wiiu_v7_open_title_extras(Game *game)
{
    sWiiUTitleCreditsDirect = false;
    finishing_stop_phone();
    sFinishing.mode = FINISHING_EXTRAS;
    sFinishing.extras_page = EXTRAS_PAGE_MAIN;
    sFinishing.extras_selection = 0;
    sFinishing.extras_index = 0;
    game->screen = SCREEN_PLACEHOLDER;
    game->screen_frames = 0u;
    game->dirty = true;
    content_play_select();
}

static void wiiu_v7_open_title_credits(Game *game)
{
    finishing_stop_phone();
    sWiiUTitleCreditsDirect = true;
    sFinishing.mode = FINISHING_EXTRAS;
    sFinishing.extras_page = EXTRAS_PAGE_CREDITS;
    sFinishing.extras_selection = 0;
    sFinishing.extras_index = 0;
    game->screen = SCREEN_PLACEHOLDER;
    game->screen_frames = 0u;
    game->dirty = true;
    content_play_select();
}

static void wiiu_v7_update_title(Game *game, const InputState *input)
{
    ++game->screen_frames;
    const int count = wiiu_v7_title_item_count();
    if (game->title_selection < 0 || game->title_selection >= count)
        game->title_selection = 0;

    const int before = game->title_selection;
    if (input_was_pressed(input, GAME_BUTTON_UP))
        game->title_selection = (game->title_selection + count - 1) % count;
    if (input_was_pressed(input, GAME_BUTTON_DOWN))
        game->title_selection = (game->title_selection + 1) % count;
    if (before != game->title_selection) {
        content_play_select();
        game->dirty = true;
    }

    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        const bool bonus_rows = progress_save_is_night_completed(5);
        if (game->title_selection == 0) {
            wiiu_v7_start_title_night(game, 1);
            return;
        }
        if (game->title_selection == 1) {
            wiiu_v7_start_title_night(game, progress_save_continue_night());
            return;
        }
        if (bonus_rows && game->title_selection == 2) {
            wiiu_v7_start_title_night(game, 6);
            return;
        }
        if (bonus_rows && game->title_selection == 3) {
            wiiu_v7_open_title_extras(game);
            return;
        }
        wiiu_v7_open_title_credits(game);
        return;
    }

    if ((game->screen_frames % 9u) == 0u)
        game->dirty = true;
}

static void wiiu_control_update_game_v7(Game *game, const InputState *input)
{
    if (game == NULL || input == NULL) {
        wiiu_control_update_game_v6(game, input);
        return;
    }

    /* Direct title Credits is intentionally a one-screen branch. B returns
     * straight to the title instead of detouring through Extras. Credits
     * opened from Extras still use the original Extras back path. */
    if (sWiiUTitleCreditsDirect &&
        game->screen == SCREEN_PLACEHOLDER &&
        sFinishing.mode == FINISHING_EXTRAS &&
        sFinishing.extras_page == EXTRAS_PAGE_CREDITS) {
        ++game->screen_frames;
        if (input_was_pressed(input, GAME_BUTTON_BACK)) {
            sWiiUTitleCreditsDirect = false;
            content_play_select();
            finishing_return_to_title(game);
            content_start_title_music();
        }
        return;
    }

    if (game->screen == SCREEN_TITLE) {
        sWiiUTitleCreditsDirect = false;
        wiiu_v7_update_title(game, input);
        return;
    }

    wiiu_control_update_game_v6(game, input);
}

/* main.c names the final update entry through wiiu_control_update_game_v6.
 * Redirect that symbol only after V7 has captured the real V6 function above. */
#define wiiu_control_update_game_v6 wiiu_control_update_game_v7

/* Wii U options V8.
 *
 * V7 made the retail-PC title navigation authoritative, which also meant the
 * older title MODE shortcut no longer reached the Wii U options handler. This
 * final layer restores that platform shortcut and expands the menu with
 * persistent language and subtitle preferences.
 */
static void wiiu_v8_cycle_language(int direction)
{
    int language = (int)wiiu_controls_language();
    language = (language + direction + WIIU_LANGUAGE_COUNT) % WIIU_LANGUAGE_COUNT;
    wiiu_controls_set_language((WiiULanguage)language);
}

static void wiiu_v8_toggle_subtitles(void)
{
    wiiu_controls_set_subtitles_enabled(!wiiu_controls_subtitles_enabled());
}

static void wiiu_v8_options_update(Game *game, const InputState *input)
{
    ++game->screen_frames;

    if (input_was_pressed(input, GAME_BUTTON_BACK) ||
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sWiiUOptionsOpen = false;
        game->dirty = true;
        return;
    }

    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        sWiiUOptionsSelection = (sWiiUOptionsSelection + 3) % 4;
        content_play_select();
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        sWiiUOptionsSelection = (sWiiUOptionsSelection + 1) % 4;
        content_play_select();
        game->dirty = true;
    }

    if (sWiiUOptionsSelection == 0) {
        if (input_was_pressed(input, GAME_BUTTON_LEFT)) {
            wiiu_options_cycle(-1);
            content_play_select();
        }
        if (input_was_pressed(input, GAME_BUTTON_RIGHT) ||
            input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_START)) {
            wiiu_options_cycle(1);
            content_play_select();
        }
    } else if (sWiiUOptionsSelection == 1) {
        if (input_was_pressed(input, GAME_BUTTON_LEFT)) {
            wiiu_v8_cycle_language(-1);
            content_play_select();
        }
        if (input_was_pressed(input, GAME_BUTTON_RIGHT) ||
            input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_START)) {
            wiiu_v8_cycle_language(1);
            content_play_select();
        }
    } else if (sWiiUOptionsSelection == 2) {
        if (input_was_pressed(input, GAME_BUTTON_LEFT) ||
            input_was_pressed(input, GAME_BUTTON_RIGHT) ||
            input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_START)) {
            wiiu_v8_toggle_subtitles();
            content_play_select();
        }
    } else if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
               input_was_pressed(input, GAME_BUTTON_START)) {
        sWiiUOptionsOpen = false;
        content_play_select();
    }

    if (input->touch_pressed && input->touch_valid) {
        if (wiiu_touch_in(input, 75, 126, 220, 72)) {
            wiiu_controls_set_mode(WIIU_CONTROL_TOUCH_GAMEPAD);
            content_play_select();
        } else if (wiiu_touch_in(input, 317, 126, 220, 72)) {
            wiiu_controls_set_mode(WIIU_CONTROL_TV_ONLY);
            content_play_select();
        } else if (wiiu_touch_in(input, 559, 126, 220, 72)) {
            wiiu_controls_set_mode(WIIU_CONTROL_MIRROR);
            content_play_select();
        } else if (wiiu_touch_in(input, 75, 226, 704, 54)) {
            wiiu_v8_cycle_language(1);
            sWiiUOptionsSelection = 1;
            content_play_select();
        } else if (wiiu_touch_in(input, 75, 294, 704, 54)) {
            wiiu_v8_toggle_subtitles();
            sWiiUOptionsSelection = 2;
            content_play_select();
        } else if (wiiu_touch_in(input, 307, 374, 240, 50)) {
            sWiiUOptionsOpen = false;
            content_play_select();
        }
    }

    game->dirty = true;
}

static void wiiu_v8_draw_mode_card(int x, const char *label,
                                   WiiUControlMode mode,
                                   const char *line1,
                                   const char *line2)
{
    const bool active = wiiu_controls_mode() == mode;
    const uint32_t border = active
        ? GRAPHICS_RGB(194, 255, 77)
        : GRAPHICS_RGB(125, 145, 122);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 126, 220, 72,
                       GRAPHICS_RGB(1, 4, 2));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, 126, 220, 72, 3, border);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 14, 140, 2, label, border);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 14, 170, 1, line1,
                       GRAPHICS_RGB(180, 195, 176));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 116, 170, 1, line2,
                       GRAPHICS_RGB(180, 195, 176));
}

static void wiiu_v8_draw_setting_row(int y, const char *label,
                                     const char *value, bool selected)
{
    const uint32_t border = selected
        ? GRAPHICS_RGB(194, 255, 77)
        : GRAPHICS_RGB(105, 125, 103);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 75, y, 704, 54,
                       GRAPHICS_RGB(1, 4, 2));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 75, y, 704, 54, 2, border);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 98, y + 18, 2, label, border);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 590, y + 18, 2, value, border);
}

static void wiiu_draw_options_v8(Game *game)
{
    (void)game;
    const bool french = wiiu_controls_language() == WIIU_LANGUAGE_FRENCH;

    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       french ? 270 : 235, 34, 4,
                       french ? "OPTIONS WII U" : "WII U OPTIONS",
                       COLOUR_WHITE);

    const uint32_t panel_colour = sWiiUOptionsSelection == 0
        ? GRAPHICS_RGB(194, 255, 77)
        : GRAPHICS_RGB(145, 166, 141);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 75, 94, 2,
                       french ? "AFFICHAGE" : "PANEL CONTROL",
                       panel_colour);

    wiiu_v8_draw_mode_card(75, "TOUCH", WIIU_CONTROL_TOUCH_GAMEPAD,
                           "TV: GAME", "DRC: TOUCH");
    wiiu_v8_draw_mode_card(317, french ? "TV SEULE" : "TV ONLY",
                           WIIU_CONTROL_TV_ONLY,
                           "TV: GAME", "DRC: BLACK");
    wiiu_v8_draw_mode_card(559, french ? "IDENTIQUE" : "IDENTICAL",
                           WIIU_CONTROL_MIRROR,
                           "TV + DRC", "SAME");

    wiiu_v8_draw_setting_row(226,
                             french ? "LANGUE" : "LANGUAGE",
                             wiiu_controls_language_name(wiiu_controls_language()),
                             sWiiUOptionsSelection == 1);
    wiiu_v8_draw_setting_row(294,
                             french ? "SOUS-TITRES" : "SUBTITLES",
                             wiiu_controls_subtitles_enabled()
                                 ? (french ? "OUI" : "ON")
                                 : (french ? "NON" : "OFF"),
                             sWiiUOptionsSelection == 2);

    const uint32_t back_colour = sWiiUOptionsSelection == 3
        ? GRAPHICS_RGB(194, 255, 77) : COLOUR_WHITE;
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 307, 374, 240, 50, 2,
                        back_colour);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       french ? 382 : 385, 391, 2,
                       french ? "RETOUR" : "BACK", back_colour);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 169, 448, 1,
                       french
                           ? "HAUT/BAS - GAUCHE/DROITE/A - B/R RETOUR"
                           : "UP/DOWN - LEFT/RIGHT/A - B/R BACK",
                       GRAPHICS_RGB(112, 132, 109));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void wiiu_control_update_game_v8(Game *game, const InputState *input)
{
    if (game == NULL || input == NULL) {
        wiiu_control_update_game_v7(game, input);
        return;
    }

    if (sWiiUOptionsOpen) {
        wiiu_v8_options_update(game, input);
        return;
    }

    if (game->screen == SCREEN_TITLE &&
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sWiiUOptionsOpen = true;
        sWiiUOptionsSelection = 0;
        content_play_select();
        game->dirty = true;
        return;
    }

    wiiu_control_update_game_v7(game, input);
}

/* main.c still names V6 as its final entry point, so retarget that alias once
 * more. original_ui_03.inc is included later and therefore also sees the new
 * options renderer alias below. */
#undef wiiu_control_update_game_v6
#define wiiu_control_update_game_v6 wiiu_control_update_game_v8
#define wiiu_draw_options wiiu_draw_options_v8
