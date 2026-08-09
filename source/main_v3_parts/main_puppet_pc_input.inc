/* Puppet blocks panel interaction but follows the player's office view while
 * the player is still free to look left and right. */
static void update_active_game(Game *game, const InputState *input)
{
    if (game != NULL && input != NULL &&
        phantoms_puppet_active(&game->phantoms)) {
        int pan = game->office_pan;
        if (input_is_held(input, GAME_BUTTON_LEFT)) pan -= 5;
        if (input_is_held(input, GAME_BUTTON_RIGHT)) pan += 5;
        if (pan < -140) pan = -140;
        if (pan > 140) pan = 140;
        if (pan != game->office_pan) {
            game->office_pan = pan;
            game->dirty = true;
        }
    }

    fnaf3_legacy_update_active_game(game, input);
}
