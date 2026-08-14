/* Eleventh PC/MFA minigame fidelity pass.
 *
 * The retail PC Extras screen exposes five replayable secret minigames:
 * BB's Air Adventure, Mangle's Quest, Chica's Party, Stage 01 and
 * RWQFSFASXC/Shadow Bonnie. Happiest Day remains a valid sixth internal scene
 * because the CAM03 Puppet drawing can launch it from a night, but it is not a
 * replay row in Extras.
 *
 * Keep SECRET_MINIGAME_COUNT at six so save/progress IDs and hidden scene
 * routing stay stable. This layer narrows only the Extras menu presentation and
 * navigation.
 */

#define PC_EXTRAS_SECRET_MINIGAME_COUNT 5

static bool pc_mfa_v11_is_extras_minigame_menu(const Game *game)
{
    return game != NULL && !sSecret.active &&
           game->screen == SCREEN_PLACEHOLDER &&
           sFinishing.mode == FINISHING_EXTRAS &&
           sFinishing.extras_page == EXTRAS_PAGE_MINIGAMES;
}

static void pc_mfa_v11_clamp_extras_selection(void)
{
    if (sSecret.selection < 0 ||
        sSecret.selection >= PC_EXTRAS_SECRET_MINIGAME_COUNT)
        sSecret.selection = 0;
}

static int pc_mfa_v11_move_vertical(int selection, int direction)
{
    /* The visual grid has three rows in the left column and two in the right.
     * Controller navigation is a Wii U adaptation, so wrap inside the current
     * visible column rather than ever landing on hidden Happiest Day. */
    if (selection < 3) {
        if (direction < 0)
            return selection == 0 ? 2 : selection - 1;
        return selection == 2 ? 0 : selection + 1;
    }

    if (direction < 0)
        return selection == 3 ? 4 : 3;
    return selection == 4 ? 3 : 4;
}

static void pc_mfa_v11_update_extras_minigame_menu(Game *game,
                                                    const InputState *input)
{
    ++game->screen_frames;
    pc_mfa_v11_clamp_extras_selection();
    const int old = sSecret.selection;

    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        sSecret.selection = pc_mfa_v11_move_vertical(sSecret.selection, -1);
    } else if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        sSecret.selection = pc_mfa_v11_move_vertical(sSecret.selection, 1);
    } else if (input_was_pressed(input, GAME_BUTTON_LEFT)) {
        if (sSecret.selection >= 3)
            sSecret.selection -= 3;
    } else if (input_was_pressed(input, GAME_BUTTON_RIGHT)) {
        if (sSecret.selection <= 1)
            sSecret.selection += 3;
        /* Row three has no right-column PC Extras item. */
    }

    if (old != sSecret.selection)
        content_play_select();

    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        content_play_select();
        secret_start(game, sSecret.selection);
        return;
    }

    if (input_was_pressed(input, GAME_BUTTON_BACK)) {
        content_play_select();
        finishing_change_extras_page(game, EXTRAS_PAGE_MAIN);
        return;
    }

    game->dirty = true;
}

static void pc_mfa_v11_exact_update_game(Game *game,
                                         const InputState *input)
{
    if (pc_mfa_v11_is_extras_minigame_menu(game)) {
        pc_mfa_v11_update_extras_minigame_menu(game, input);
        return;
    }
    pc_mfa_v10_exact_update_game(game, input);
}

static void pc_mfa_v11_draw_extras_minigame_menu(void)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(2, 8, 5));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 58,
                       GRAPHICS_RGB(8, 25, 12));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 28, 19, 3,
                       "EXTRAS / SECRET MINIGAMES", COLOUR_GREEN);

    pc_mfa_v11_clamp_extras_selection();
    for (int index = 0; index < PC_EXTRAS_SECRET_MINIGAME_COUNT; ++index) {
        const int column = index / 3;
        const int row = index % 3;
        const int x = 56 + column * 402;
        const int y = 92 + row * 103;

        if (index == sSecret.selection) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 342, 72,
                               GRAPHICS_RGB(30, 70, 35));
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, y, 342, 72, 3,
                                GRAPHICS_RGB(128, 202, 122));
        } else {
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, y, 342, 72, 2,
                                GRAPHICS_RGB(52, 90, 55));
        }

        graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 17, y + 19, 2,
                           kSecretMinigameNames[index],
                           index == sSecret.selection
                               ? COLOUR_WHITE
                               : GRAPHICS_RGB(123, 151, 122));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 292, y + 45, 1,
                           progress_save_is_secret_minigame_completed(index + 1)
                               ? "DONE" : "NEW",
                           progress_save_is_secret_minigame_completed(index + 1)
                               ? COLOUR_GREEN
                               : GRAPHICS_RGB(184, 137, 72));
    }

    if (progress_save_good_ending_unlocked())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 257, 409, 2,
                           "GOOD ENDING UNLOCKED", COLOUR_WHITE);

    graphics_draw_text(GRAPHICS_TARGET_BOTH, 260, 450, 1,
                       "D-PAD: SELECT   A: PLAY   B: BACK",
                       GRAPHICS_RGB(121, 161, 117));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static bool pc_mfa_v11_extras_render_override(Game *game)
{
    if (!pc_mfa_v11_is_extras_minigame_menu(game))
        return false;
    if (!game->dirty)
        return true;

    game->dirty = false;
    pc_mfa_v11_draw_extras_minigame_menu();
    return true;
}
