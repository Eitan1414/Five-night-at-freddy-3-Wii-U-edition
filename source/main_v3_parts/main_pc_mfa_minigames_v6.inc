/* Sixth MFA fidelity pass: final update routing.
 *
 * V5 owns the new platform SFX.  When no secret minigame is active, fall
 * straight back to the base MFA wrapper instead of V2 so the V2 audio hook
 * cannot replay Follow Me/entry sounds a second time. */

static void pc_mfa_v6_exact_update_game(Game *game, const InputState *input)
{
    const bool before_active = sSecret.active;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;

    if (sSecret.active && !sSecret.good_ending) {
        pc_mfa_v5_update_active(game, input);
    } else {
        sPcMfaV3.active = false;
        sPcMfaV5.active = false;
        pc_mfa_exact_update_game(game, input);
    }

    pc_mfa_v5_audio_after_update(before_active, before_collected,
                                 before_delivered, before_follow_event);
}
