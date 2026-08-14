/*
 * Retail-PC vent sealing compatibility layer.
 *
 * The legacy Wii U prototype wrote selected_vent into sealed_vent in the same
 * input frame.  PC FNaF 3 instead enters a visible `sealing vent` state before
 * the closure becomes effective; community frame/video observations put that
 * operation at about three seconds.  Until the raw MFA event counter is
 * recovered again, keep that duration explicitly labelled as PC-observed
 * timing rather than claiming an exact Clickteam counter.
 *
 * This wrapper sits outside the AI: Springtrap/Phantoms continue updating while
 * the closure is in progress, preserving the vulnerability window.
 */
#define PC_VENT_SEAL_OBSERVED_FRAMES (3u * 60u)

typedef struct PcVentSealState {
    bool active;
    SpringtrapVent target;
    uint32_t frames;
} PcVentSealState;

static PcVentSealState sPcVentSeal = {
    false, SPRINGTRAP_VENT_NONE, 0u
};

static bool pc_vent_seal_is_active(void)
{
    return sPcVentSeal.active;
}

static SpringtrapVent pc_vent_seal_target(void)
{
    return sPcVentSeal.active ? sPcVentSeal.target : SPRINGTRAP_VENT_NONE;
}

static uint32_t pc_vent_seal_progress_frames(void)
{
    return sPcVentSeal.active ? sPcVentSeal.frames : 0u;
}

static uint32_t pc_vent_seal_duration_frames(void)
{
    return PC_VENT_SEAL_OBSERVED_FRAMES;
}

static void pc_vent_seal_cancel(Game *game)
{
    if (!sPcVentSeal.active) return;
    sPcVentSeal.active = false;
    sPcVentSeal.target = SPRINGTRAP_VENT_NONE;
    sPcVentSeal.frames = 0u;
    if (game != NULL) game->dirty = true;
}

static bool pc_vent_seal_context_valid(const Game *game)
{
    return game != NULL && game->screen == SCREEN_OFFICE &&
           game->camera_open && game->vent_map_open &&
           !game->ventilation_failed &&
           !phantoms_blocks_input(&game->phantoms);
}

static void pc_vent_seal_finish(Game *game)
{
    if (game == NULL || !sPcVentSeal.active) return;

    /* Only one PC vent may be closed at once. Replacing sealed_vent here keeps
     * the old closure effective throughout the sealing animation, then swaps
     * atomically when the new closure completes. */
    game->sealed_vent = sPcVentSeal.target;
    sPcVentSeal.active = false;
    sPcVentSeal.target = SPRINGTRAP_VENT_NONE;
    sPcVentSeal.frames = 0u;

    /* The retail seal-complete sound is shared with the system-restart sound;
     * AUDIO_CUE_WAIT is the existing PC-derived restart cue in this port. */
    audio_play(AUDIO_CUE_WAIT, 0.6f, false);
    set_message(game, MESSAGE_VENT_SEALED);
    game->dirty = true;
}

static void pc_vent_seal_update_game(Game *game, const InputState *input)
{
    if (game == NULL || input == NULL) return;

    InputState filtered = *input;
    const bool seal_pressed =
        input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_SEAL);
    const bool seal_context = pc_vent_seal_context_valid(game);

    /* Consume the legacy instant-seal action whenever A/Seal is used on the
     * vent map. Other button paths remain byte-for-byte the same downstream. */
    if (seal_context && seal_pressed) {
        const uint32_t mask = GAME_BUTTON_CONFIRM | GAME_BUTTON_SEAL;
        filtered.pressed &= ~mask;
        filtered.held &= ~mask;
        filtered.released &= ~mask;

        if (!sPcVentSeal.active) {
            sPcVentSeal.active = true;
            sPcVentSeal.target = game->selected_vent;
            sPcVentSeal.frames = 0u;
            game->dirty = true;
        }
    }

    pc_system_fidelity_update_game(game, &filtered);

    if (!sPcVentSeal.active) return;

    /* PC feedback is tied to the vent camera being sealed. Moving away from
     * that target, leaving the vent map/monitor, or losing ventilation aborts
     * the pending closure instead of granting an invisible instant seal. */
    if (!pc_vent_seal_context_valid(game) ||
        game->selected_vent != sPcVentSeal.target) {
        pc_vent_seal_cancel(game);
        return;
    }

    if (sPcVentSeal.frames < PC_VENT_SEAL_OBSERVED_FRAMES)
        ++sPcVentSeal.frames;

    game->dirty = true;
    if (sPcVentSeal.frames >= PC_VENT_SEAL_OBSERVED_FRAMES)
        pc_vent_seal_finish(game);
}
