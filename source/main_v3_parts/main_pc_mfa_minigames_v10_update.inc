/* Tenth MFA minigame update pass.
 *
 * V7 incorrectly stopped running the decoded minigame state machine as soon as
 * Marion wrote goodend, even though the original MFA keeps the Happiest Day
 * frame alive through its 100/200/300/1300-count finale.  Keep the exact V7
 * update alive for every active secret frame, and retain V8's release of normal
 * 200-count cake sequences once win/completed has been set.
 *
 * Hidden minigames launched from a night also return to the office
 * automatically after the existing completion hold.  The former mandatory
 * A/B/+ prompt was a Wii U control artefact, not a PC interaction.  The custom
 * Extras minigame launcher still keeps its manual return behaviour.
 */
static void pc_mfa_v10_exact_update_game(Game *game,
                                         const InputState *input)
{
    const bool before_active = sSecret.active;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;

    if (sSecret.active) {
        pc_mfa_v7_update_active(game, input);
    } else {
        sPcMfaV7.active = false;
        sPcMfaV3.active = false;
        sPcMfaV5.active = false;
        pc_mfa_v2_exact_update_game(game, input);
    }

    pc_mfa_v5_audio_after_update(before_active, before_collected,
                                 before_delivered, before_follow_event);

    /* V8 behaviour: normal cake sequences finish at 200 and then allow the
     * regular completed-frame path to advance. Happiest Day owns a separate
     * final_sequence and therefore remains latched until its 1300-count event. */
    if (sSecret.completed && sPcMfaV7.cake_sequence &&
        !sPcMfaV7.final_sequence && sPcMfaV7.sequence_frames >= 200u) {
        sPcMfaV7.cake_sequence = false;
        game->dirty = true;
    }

    /* Secret triggers entered from the night should disappear back into the
     * office without a Wii U-only confirmation prompt. Keep the existing 150
     * frame visual hold until a more precise source transition is decoded. */
    if (sSecret.active && sSecretTriggers.return_to_office &&
        sSecret.completed && sSecret.completed_frames >= 150u) {
        sPcMfaV7.active = false;
        sPcMfaV3.active = false;
        sPcMfaV5.active = false;
        complete_return_from_hidden_minigame(game);
    }
}
