/* V8 update shim: V7 intentionally freezes traversal during a cake sequence,
 * but once its 200-count event has set win/completed the sequence latch must
 * be released so the normal completion timer/input path can run on the next
 * frame.  Happiest Day keeps its separate 1300-count good-ending route. */
static void pc_mfa_v8_exact_update_game(Game *game, const InputState *input)
{
    pc_mfa_v7_exact_update_game(game, input);

    if (sSecret.completed && sPcMfaV7.cake_sequence &&
        !sPcMfaV7.final_sequence && sPcMfaV7.sequence_frames >= 200u) {
        sPcMfaV7.cake_sequence = false;
        game->dirty = true;
    }
}
