/*
 * Original PC maintenance-system timing pass.
 *
 * The first Wii U gameplay layer used deterministic night-frame failures and
 * a ten-second idle -> ventilation-error shortcut. Those were scaffolding,
 * not Clickteam mechanics. Keep the old fields only for ABI/source stability
 * and drive the actual system failures here from the retail PC counters.
 */

typedef struct PcSystemFidelityState {
    int night;
    uint32_t camera_usage_frames;
    uint32_t audio_uses;
    bool audio_failure_pending;
    int ventilation_health;
    uint32_t ventilation_health_frames;
    uint32_t hallucination_rng;
} PcSystemFidelityState;

static PcSystemFidelityState sPcSystems = {
    .night = -1,
};

static uint32_t pc_systems_random(void)
{
    sPcSystems.hallucination_rng =
        sPcSystems.hallucination_rng * 1664525u + 1013904223u;
    return sPcSystems.hallucination_rng;
}

static int pc_systems_ai_level(int night)
{
    if (night <= 1) return 0;
    if (night == 2) return 2;
    if (night == 3) return 3;
    if (night == 4) return 4;
    if (night == 5) return 5;
    return 7;
}

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
    sPcSystems.hallucination_rng =
        0xA341316Cu ^ ((uint32_t)night * 0x9E3779B9u);
    sPcHallucinationCameraMask = 0u;
    sPcHallucinationFrames = 0u;
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
    /* The PC hallucination counter continues after the ventilation repair.
     * Do not clear sPcHallucinationFrames here. */
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

static void pc_systems_damage_ventilation(Game *game)
{
    if (sPcSystems.ventilation_health > -10)
        --sPcSystems.ventilation_health;

    if (sPcSystems.ventilation_health <= -10)
        set_ventilation_failure(game);
}

static void pc_systems_update_ventilation_health(Game *game)
{
    if (game->ventilation_failed) return;

    /* MFA groups 330-334: independent natural health drain by AI level. */
    const uint32_t interval = pc_ventilation_health_interval(game->night);
    if (interval != UINT32_MAX) {
        if (++sPcSystems.ventilation_health_frames >= interval) {
            sPcSystems.ventilation_health_frames = 0u;
            pc_systems_damage_ventilation(game);
            if (game->ventilation_failed) return;
        }
    }

    /* MFA groups 746-749 are a second, independent drain path. The no-screen
     * counter gains one every 1000 ms, is reset by either system screen, and
     * begins subtracting one ventilation-health point per second only when the
     * counter is strictly greater than 10. springtrap.office_idle_frames is now
     * the exact same source counter, so the first damage tick is 11 seconds.
     * Night 1 is explicitly excluded by group 748. */
    const uint32_t no_screen_frames = game->springtrap.office_idle_frames;
    if (game->night > 1 &&
        no_screen_frames >= 11u * 60u &&
        (no_screen_frames % 60u) == 0u) {
        pc_systems_damage_ventilation(game);
    }
}

static void pc_systems_begin_hallucination(Game *game)
{
    const int ai = pc_systems_ai_level(game->night);
    if (ai <= 0) return;

    /* MFA: C = (AI * 200) + Random(200), then C is decremented every frame.
     * The exact probabilities are preserved here; the Wii U port deliberately
     * uses its own deterministic RNG stream rather than claiming Clickteam's
     * internal RNG sequence. */
    sPcHallucinationFrames =
        (uint32_t)(ai * 200) + (pc_systems_random() % 200u);

    const int real_camera = springtrap_ai_camera(&game->springtrap);
    uint16_t mask = 0u;
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        if (camera == real_camera) continue;
        /* PC Hallucinationtrap: each eligible camera receives Random(3), with
         * one of the three values selecting the fake Springtrap composite. */
        if ((pc_systems_random() % 3u) == 1u)
            mask |= (uint16_t)(1u << (uint32_t)camera);
    }
    sPcHallucinationCameraMask = mask;
    game->dirty = true;
}

static void pc_systems_update_hallucination(Game *game)
{
    if (sPcHallucinationFrames > 0u) {
        --sPcHallucinationFrames;
        if (sPcHallucinationFrames == 0u) {
            sPcHallucinationCameraMask = 0u;
            game->dirty = true;
        }
        return;
    }

    if (!game->ventilation_failed) return;

    const int ai = pc_systems_ai_level(game->night);
    if (ai <= 0) return;

    /* MFA hallucination trigger: ventilation-error counter B must exceed
     * 1000 - (AI * 100). B is represented by ventilation_error_frames in the
     * Wii U integration and continues increasing while the system is failed. */
    const uint32_t threshold = (uint32_t)(1000 - ai * 100);
    if (game->ventilation_error_frames > threshold)
        pc_systems_begin_hallucination(game);
}

static void pc_systems_enforce_ventilation_error(Game *game)
{
    if (!game->ventilation_failed) return;

    /* Retail PC ventilation errors release the current vent seal and prevent
     * a new one from remaining active until ventilation has been rebooted. The
     * legacy input path may momentarily write selected_vent into sealed_vent;
     * clearing it in the same update frame preserves the PC gameplay result. */
    if (game->sealed_vent != SPRINGTRAP_VENT_NONE) {
        game->sealed_vent = SPRINGTRAP_VENT_NONE;
        if (game->message == MESSAGE_VENT_SEALED) {
            game->message = MESSAGE_NONE;
            game->message_frames = 0u;
        }
        game->dirty = true;
    }
}

static int pc_title_item_count(void)
{
    /* Completing Night 5 reveals Nightmare and Extra as two independent rows. */
    return progress_save_extras_unlocked() ? 4 : 2;
}

static void pc_systems_update_title(Game *game, const InputState *input)
{
    ++game->screen_frames;
    if (sFinishing.title_notice_frames > 0u)
        --sFinishing.title_notice_frames;

    const int item_count = pc_title_item_count();
    if (game->title_selection < 0 || game->title_selection >= item_count)
        game->title_selection = 0;

    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        game->title_selection =
            (game->title_selection + item_count - 1) % item_count;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        game->title_selection = (game->title_selection + 1) % item_count;
        game->dirty = true;
    }

    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        switch (game->title_selection) {
            case 0:
                /* New Game resets only the five-night Continue chain. PC bonus
                 * unlocks, stars, Nightmare and Extras remain unlocked. */
                progress_save_start_new_game();
                sFinishing.mode = FINISHING_NONE;
                finishing_stop_phone();
                reset_night(game, 1);
                return;

            case 1:
                sFinishing.mode = FINISHING_NONE;
                finishing_stop_phone();
                reset_night(game, progress_save_continue_night());
                return;

            case 2:
                if (progress_save_extras_unlocked()) {
                    sFinishing.mode = FINISHING_NONE;
                    finishing_stop_phone();
                    reset_night(game, 6);
                    return;
                }
                break;

            case 3:
                if (progress_save_extras_unlocked()) {
                    finishing_open_extras(game);
                    return;
                }
                break;

            default:
                break;
        }

        sFinishing.title_notice_frames = 180u;
        game->dirty = true;
    }

    if ((game->screen_frames % 9u) == 0u ||
        sFinishing.title_notice_frames > 0u) {
        game->dirty = true;
    }
}

static void pc_system_fidelity_update_game(Game *game,
                                           const InputState *input)
{
    /* Final title routing lives here so the old three-item compatibility menu
     * cannot swallow Nightmare into Extra. */
    if (game->screen == SCREEN_TITLE) {
        pc_systems_update_title(game, input);
        return;
    }

    const bool lure_was_active = game->audio_lure_active;
    const bool repair_was_active = game->repairing;
    const int repair_target = game->repair_target;
    const bool ventilation_was_failed = game->ventilation_failed;
    const PhantomId phantom_before =
        phantoms_active_jumpscare(&game->phantoms);

    /* Disable the old deterministic frame-based system failures. The legacy
     * update still owns the normal gameplay ordering, but these three flags
     * prevent its scaffolding thresholds from firing. */
    game->camera_failure_triggered = true;
    game->audio_failure_triggered = true;
    game->ventilation_failure_triggered = true;

    pc_mfa_v8_exact_update_game(game, input);

    if (sPcSystems.night != game->night)
        pc_systems_reset_for_night(game->night);

    /* PC Phantom jumpscares enter the ventilation fade immediately. Natural
     * health depletion begins at B=0 and therefore keeps the short alarm/red
     * light lead-in. The Phantom event ends its jumpscare and raises the vent
     * error in the same legacy update, which gives us an unambiguous source. */
    if (!ventilation_was_failed && game->ventilation_failed &&
        phantom_before != PHANTOM_NONE &&
        phantoms_active_jumpscare(&game->phantoms) == PHANTOM_NONE) {
        game->ventilation_error_frames = PC_VENT_FADE_DELAY_FRAMES + 1u;
        game->player_blinded =
            pc_ventilation_is_blind(game->ventilation_error_frames);
        game->dirty = true;
    }

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
    pc_systems_update_hallucination(game);
    pc_systems_enforce_ventilation_error(game);
}