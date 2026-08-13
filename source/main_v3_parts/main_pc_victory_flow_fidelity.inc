/*
 * Retail-PC 6 AM flow.
 *
 * The recovery prototype stopped after the 5:59 -> 6:00 animation and exposed
 * an A-button `NIGHT COMPLETE` continuation.  PC FNaF 3 has no interactive
 * completion gate there: after the clock animation the game proceeds into the
 * corresponding Follow Me scene (Nights 1-5) or the Night 6 ending path.
 *
 * Keep the existing 180-frame compatibility duration until the exact Clickteam
 * end-of-clock event timing is decoded, but remove the platform-only input gate.
 */
static void pc_victory_flow_update_game(Game *game, const InputState *input)
{
    if (game == NULL || input == NULL) return;

    if (game->screen != SCREEN_VICTORY) {
        wiiu_control_update_game_v6(game, input);
        return;
    }

    ++game->screen_frames;
    if ((game->screen_frames % 6u) == 0u)
        game->dirty = true;

    if (game->screen_frames < 180u)
        return;

    if (game->night <= 5)
        finishing_start_minigame(game, game->night, false);
    else
        finishing_start_ending(game, FINISHING_NIGHT6_ENDING);
}
