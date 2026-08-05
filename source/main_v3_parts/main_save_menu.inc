#undef handle_title_input

static void handle_title_input(Game *game, const InputState *input)
{
    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        game->title_selection =
            (game->title_selection + TITLE_ITEM_COUNT - 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        game->title_selection =
            (game->title_selection + 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (!input_was_pressed(input, GAME_BUTTON_CONFIRM) &&
        !input_was_pressed(input, GAME_BUTTON_START)) {
        return;
    }

    if (game->title_selection == 0) {
        reset_night(game, 1);
    } else if (game->title_selection == 1) {
        reset_night(game, g_highest_unlocked_night);
    } else {
        game->screen = SCREEN_PLACEHOLDER;
        game->screen_frames = 0u;
        game->dirty = true;
    }
}
