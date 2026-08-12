/*
 * Original PC maintenance-system timing pass.
 *
 * System health is reconstructed directly from fivenights3-94.mfa rather than
 * from fixed failure times. Audio and camera health lose AI points at their
 * source events, ventilation loses one point on its own night-dependent timer
 * plus the independent no-screen pressure event, and all three fail at -10.
 */

typedef struct PcSystemFidelityState {
    int night;
    int audio_health;
    int camera_health;
    uint32_t camera_seconds;
    int ventilation_health;
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

static uint32_t pc_ventilation_health_interval(int night)
{
    /* MFA groups 330-334. AI 6 has a 6000 ms branch, but the retail six-night
     * configuration jumps from AI 5 on Night 5 to AI 7 on Nightmare, so Night
     * 6 has no natural timer drain. Phantom/no-screen damage still applies. */
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
    sPcSystems.hallucination_rng =
        0xA341316Cu ^ ((uint32_t)night * 0x9E3779B9u);
    sPcHallucinationCameraMask = 0u;
    sPcHallucinationFrames = 0u;
}

static void pc_systems_reset_camera(bool reset_subcounter)
{
    /* MFA group 312 repairs camera health only. Reboot All (group 314) also
     * clears camera alterable value F, the twelve-second subcounter. */
    sPcSystems.camera_health = 0;
    if (reset_subcounter)
        sPcSystems.camera_seconds = 0u;
}

static void pc_systems_reset_audio(void)
{
    sPcSystems.audio_health = 0;
}

static void pc_systems_reset_ventilation(void)
{
    sPcSystems.ventilation_health = 0;
    /* Clickteam Every timers are global event timers. A repair restores health
     * but does not restart their phase, so there is no local vent timer reset. */
}

static void pc_systems_apply_completed_repair(const Game *game,
                                              int repaired_target)
{
    (void) game;
    if (repaired_target == MAINT_CAMERA)
        pc_systems_reset_camera(false);
    else if (repaired_target == MAINT_REBOOT_ALL)
        pc_systems_reset_camera(true);

    if (repaired_target == MAINT_AUDIO ||
        repaired_target == MAINT_REBOOT_ALL)
        pc_systems_reset_audio();

    if (repaired_target == MAINT_VENTILATION ||
        repaired_target == MAINT_REBOOT_ALL)
        pc_systems_reset_ventilation();
}

static void pc_systems_update_camera(Game *game)
{
    if (game->camera_failed || !game->camera_open) return;

    /* MFA group 637: every 1000 ms while the camera screen is viewed, camera
     * alterable F gains one. Group 638: at F >= 12, health -= AI and F = 0. */
    if ((game->night_frames % 60u) != 0u) return;

    if (sPcSystems.camera_seconds < 12u)
        ++sPcSystems.camera_seconds;
    if (sPcSystems.camera_seconds < 12u) return;
    sPcSystems.camera_seconds = 0u;

    const int ai = pc_systems_ai_level(game->night);
    if (ai <= 0) return;

    sPcSystems.camera_health -= ai;
    if (sPcSystems.camera_health <= -10) {
        sPcSystems.camera_health = -10;
        game->camera_failed = true;
        game->dirty = true;
    }
}

static void pc_systems_note_audio_use(Game *game)
{
    if (game->audio_failed) return;

    /* MFA group 225: every accepted Play Audio press subtracts AI directly
     * from audio health. Groups 282/288 expose/clamp the error at <= -10.
     * This yields exact use counts N2..N6 = 5,4,3,2,2. */
    const int ai = pc_systems_ai_level(game->night);
    if (ai <= 0) return;

    sPcSystems.audio_health -= ai;
    if (sPcSystems.audio_health <= -10) {
        sPcSystems.audio_health = -10;
        /* The lure created by group 225 is still allowed to resolve in the same
         * Clickteam event loop; mark the system failed without cancelling it. */
        game->audio_failed = true;
        set_message(game, MESSAGE_AUDIO_FAILED);
        game->dirty = true;
    }
}

static void pc_systems_damage_ventilation(Game *game)
{
    if (sPcSystems.ventilation_health > -10)
        --sPcSystems.ventilation_health;

    if (sPcSystems.ventilation_health <= -10) {
        sPcSystems.ventilation_health = -10;
        set_ventilation_failure(game);
    }
}

static void pc_systems_update_ventilation_health(Game *game)
{
    if (game->ventilation_failed) return;

    /* MFA groups 330-334 use global Every timers, so night_frames supplies the
     * phase and a repair does not grant a fresh full interval. */
    const uint32_t interval = pc_ventilation_health_interval(game->night);
    if (interval != UINT32_MAX &&
        game->night_frames != 0u &&
        (game->night_frames % interval) == 0u) {
        pc_systems_damage_ventilation(game);
        if (game->ventilation_failed) return;
    }

    /* MFA groups 746-749 are a second, independent drain path. The same exact
     * no-screen counter is maintained inside Springtrap AI: it becomes >10 on
     * the eleventh one-second tick, then ventilation health loses one each
     * second. Night 1 is explicitly excluded by group 748. */
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

    const uint32_t lure_cooldown_before = game->audio_lure_cooldown;
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

    /* A successful Play Audio press moves cooldown from zero to a positive
     * value. This detects the use even when the MFA lure is resolved and
     * destroyed in the same gameplay frame. */
    if (lure_cooldown_before == 0u && game->audio_lure_cooldown > 0u)
        pc_systems_note_audio_use(game);

    pc_systems_update_camera(game);
    pc_systems_update_ventilation_health(game);
    pc_systems_update_hallucination(game);
    pc_systems_enforce_ventilation_error(game);
}