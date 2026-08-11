/*
 * Original PC maintenance-system timing pass.
 *
 * The first Wii U gameplay layer used deterministic night-frame failures and
 * a ten-second idle -> ventilation-error shortcut.  Those were scaffolding,
 * not Clickteam mechanics.  Keep the old fields only for ABI/source stability
 * and drive the actual system failures here from the retail PC counters.
 */

typedef struct PcSystemFidelityState {
    int night;
    uint32_t camera_usage_frames;
    uint32_t audio_uses;
    bool audio_failure_pending;
    int ventilation_health;
    uint32_t ventilation_health_frames;
} PcSystemFidelityState;

static PcSystemFidelityState sPcSystems = {
    .night = -1,
};

static uint32_t pc_camera_limit_frames(int night)
{
    /* Retail PC camera-use limits: N2 60s, N3 48s, N4 36s,
     * N5 24s, N6 17s. Night 1 cannot naturally fail. */
    switch (night) {
        case 2: return 60u * 60u;
        case 3: return 48u * 60u;
        case 4: return 36u * 60u;
        case 5: return 24u * 60u;
        case 6: return 17u * 60u;
        default: return UINT32_MAX;
    }
}

static uint32_t pc_audio_use_limit(int night)
{
    /* Observed retail-PC limits used by the speedrun community. */
    switch (night) {
        case 2: return 10u;
        case 3: return 4u;
        case 4: return 3u;
        case 5:
        case 6: return 2u;
        default: return UINT32_MAX;
    }
}

static uint32_t pc_ventilation_health_interval(int night)
{
    /* fivenights3-94.mfa ventilation-health drain. The value starts at 0 and
     * loses one point until -10. The retail Night-6 AI is 7, while the source
     * drain branch only covers the lower AI values, so N6 has no natural
     * ventilation failure (Phantoms can still cause one). */
    switch (night) {
        case 2: return 12u * 60u;
        case 3: return 10u * 60u;
        case 4: return 9u * 60u;
        case 5: return 8u * 60u;
        default: return UINT32_MAX;
    }
}

static void pc_systems_reset_for_night(int night)
{
    memset(&sPcSystems, 0, sizeof(sPcSystems));
    sPcSystems.night = night;
    sPcSystems.ventilation_health = 0;
}

static void pc_systems_reset_camera(void)
{
    sPcSystems.camera_usage_frames = 0u;
}

static void pc_systems_reset_audio(void)
{
    sPcSystems.audio_uses = 0u;
    sPcSystems.audio_failure_pending = false;
}

static void pc_systems_reset_ventilation(void)
{
    sPcSystems.ventilation_health = 0;
    sPcSystems.ventilation_health_frames = 0u;
}

static void pc_systems_apply_completed_repair(const Game *game,
                                              int repaired_target)
{
    (void) game;
    if (repaired_target == MAINT_CAMERA ||
        repaired_target == MAINT_REBOOT_ALL) {
        pc_systems_reset_camera();
    }
    if (repaired_target == MAINT_AUDIO ||
        repaired_target == MAINT_REBOOT_ALL) {
        pc_systems_reset_audio();
    }
    if (repaired_target == MAINT_VENTILATION ||
        repaired_target == MAINT_REBOOT_ALL) {
        pc_systems_reset_ventilation();
    }
}

static void pc_systems_update_camera(Game *game)
{
    if (game->camera_failed || !game->camera_open) return;

    const uint32_t limit = pc_camera_limit_frames(game->night);
    if (limit == UINT32_MAX) return;

    if (sPcSystems.camera_usage_frames < limit)
        ++sPcSystems.camera_usage_frames;

    if (sPcSystems.camera_usage_frames >= limit) {
        game->camera_failed = true;
        game->dirty = true;
    }
}

static void pc_systems_note_audio_use(Game *game)
{
    if (game->audio_failed) return;

    const uint32_t limit = pc_audio_use_limit(game->night);
    if (limit == UINT32_MAX) return;

    if (sPcSystems.audio_uses < limit)
        ++sPcSystems.audio_uses;

    /* The final allowed lure is still played. The error is raised when that
     * playback resolves, matching the PC behaviour instead of cancelling the
     * last usable lure at button-press time. */
    if (sPcSystems.audio_uses >= limit)
        sPcSystems.audio_failure_pending = true;
}

static void pc_systems_finish_pending_audio_failure(Game *game)
{
    if (!sPcSystems.audio_failure_pending || game->audio_failed) return;

    sPcSystems.audio_failure_pending = false;
    game->audio_failed = true;
    stop_lure_audio();
    set_message(game, MESSAGE_AUDIO_FAILED);
    game->dirty = true;
}

static void pc_systems_update_ventilation_health(Game *game)
{
    if (game->ventilation_failed) return;

    const uint32_t interval = pc_ventilation_health_interval(game->night);
    if (interval == UINT32_MAX) return;

    if (++sPcSystems.ventilation_health_frames < interval) return;
    sPcSystems.ventilation_health_frames = 0u;

    if (sPcSystems.ventilation_health > -10)
        --sPcSystems.ventilation_health;

    if (sPcSystems.ventilation_health <= -10)
        set_ventilation_failure(game);
}

static void pc_system_fidelity_update_game(Game *game,
                                           const InputState *input)
{
    const bool lure_was_active = game->audio_lure_active;
    const bool repair_was_active = game->repairing;
    const int repair_target = game->repair_target;

    /* Disable the old deterministic frame-based system failures. The legacy
     * update still owns the normal gameplay ordering, but these three flags
     * prevent its scaffolding thresholds from firing. */
    game->camera_failure_triggered = true;
    game->audio_failure_triggered = true;
    game->ventilation_failure_triggered = true;

    pc_mfa_v8_exact_update_game(game, input);

    if (sPcSystems.night != game->night)
        pc_systems_reset_for_night(game->night);

    if (game->screen != SCREEN_OFFICE) return;

    /* The Extras no-errors cheat must remain absolute. Reset counters too so
     * disabling the cheat does not instantly resurrect a previously hidden
     * failure. */
    if (sCompleteCheats.no_errors) {
        pc_systems_reset_for_night(game->night);
        return;
    }

    if (repair_was_active && !game->repairing)
        pc_systems_apply_completed_repair(game, repair_target);

    if (!lure_was_active && game->audio_lure_active)
        pc_systems_note_audio_use(game);

    if (lure_was_active && !game->audio_lure_active)
        pc_systems_finish_pending_audio_failure(game);

    if (game->audio_failed)
        sPcSystems.audio_failure_pending = false;

    pc_systems_update_camera(game);
    pc_systems_update_ventilation_health(game);
}
