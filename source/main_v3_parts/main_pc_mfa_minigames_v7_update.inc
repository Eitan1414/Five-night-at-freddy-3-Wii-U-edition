static void pc_mfa_v7_update_bb(Game *game, const InputState *input)
{
    (void)pc_mfa_v7_apply_physics(input);
    pc_mfa_v7_collect_bb(game);

    const bool exit_visible = (sSecret.collected & 0x7Fu) == 0x7Fu;
    if (exit_visible && pc_mfa_v3_near(kPcMfaBbExit, 50)) {
        pc_mfa_v7_normal_finish(game);
        return;
    }

    /* The hidden BB pickup writes INI "bb"=1 and wins immediately. */
    if (pc_mfa_v3_near(kPcMfaBbHidden, 54)) {
        pc_mfa_v7_unlock_bb();
        pc_mfa_v7_normal_finish(game);
        return;
    }

    /* k1: hidden cake trigger + global cake flag + child in its idle state. */
    if (!pc_mfa_v7_k1() && pc_mfa_v7_cake_owned() &&
        pc_mfa_v3_near(kPcMfaBbBigCake, 62))
        pc_mfa_v7_begin_cake_sequence(game, false);

    pc_mfa_v7_tick_secret_sequence(game);
}

static void pc_mfa_v7_update_mangle(Game *game, const InputState *input)
{
    (void)pc_mfa_v7_apply_physics(input);
    pc_mfa_collect(game, kPcMfaMangleParts, 4, 42);

    const bool all_parts = (sSecret.collected & 0x0Fu) == 0x0Fu;
    if (all_parts &&
        (pc_mfa_v3_near(kPcMfaMangleExit, 56) ||
         pc_mfa_v3_near(kPcMfaMangleAltExit, 56))) {
        pc_mfa_v7_normal_finish(game);
        return;
    }
    if (pc_mfa_v3_near(kPcMfaV2MangleKid, 60)) {
        pc_mfa_v7_normal_finish(game);
        return;
    }

    /* Mangle Event group 22: touching either selected "get this" instance
     * sets INI cake=1, destroys the pickup and sets win=1. */
    if (!pc_mfa_v7_cake_owned()) {
        for (int i = 0; i < 2; ++i) {
            if (!pc_mfa_v3_near(kPcMfaV7MangleGetThis[i], 58)) continue;
            /* Mangle group 22 writes cake=1 and win=1 in the same event. */
            progress_save_complete_secret_minigame(2);
            pc_mfa_v7_normal_finish(game);
            return;
        }
    }
}

static void pc_mfa_v7_update_chica(Game *game, const InputState *input)
{
    (void)pc_mfa_v7_apply_physics(input);

    /* Four collect objects; any carried cupcake may feed any still-unfed
     * Active 8 child, matching the selected-object Fusion event. */
    for (int i = 0; i < 4; ++i) {
        const uint8_t pickup = (uint8_t)(1u << i);
        if ((sSecret.collected & pickup) == 0u &&
            pc_mfa_v3_near(kPcMfaChicaCupcakes[i], 42)) {
            sSecret.collected |= pickup;
            ++sSecret.carried;
        }
    }
    for (int i = 0; i < 4 && sSecret.carried > 0; ++i) {
        const uint8_t fed = (uint8_t)(1u << (i + 4));
        if ((sSecret.collected & fed) != 0u) continue;
        if (!pc_mfa_v3_near(kPcMfaChicaChildren[i], 54)) continue;
        sSecret.collected |= fed;
        --sSecret.carried;
        ++sSecret.delivered;
    }

    if (sSecret.delivered >= 4 &&
        (pc_mfa_v3_near(kPcMfaChicaExit, 58) ||
         pc_mfa_v3_near(kPcMfaV2ChicaExit2, 58))) {
        pc_mfa_v7_normal_finish(game);
        return;
    }

    if (pc_mfa_v3_near(kPcMfaV7ChicaGetThis, 54)) {
        pc_mfa_v7_unlock_bb();
        pc_mfa_v7_normal_finish(game);
        return;
    }

    /* Chica Event group 39 writes k2 and reveals get-this-2; its freeze
     * counter reaches 100 then 200 before win. */
    if (!pc_mfa_v7_k2() && pc_mfa_v7_cake_owned() &&
        pc_mfa_v3_near(kPcMfaV7ChicaCakeTrigger, 58))
        pc_mfa_v7_begin_cake_sequence(game, false);

    pc_mfa_v7_tick_secret_sequence(game);
}

static void pc_mfa_v7_update_stage01(Game *game, const InputState *input)
{
    (void)pc_mfa_v7_apply_physics(input);
    pc_mfa_v3_apply_zip_rules();
    pc_mfa_v4_apply_stage_left_zip();

    /* Stage 01's Counter is already at the value that reveals its ordinary
     * exit; the previous three-anchor hunt was a Wii U approximation. */
    if (pc_mfa_v3_near(kPcMfaStageExit, 58)) {
        pc_mfa_v7_normal_finish(game);
        return;
    }
    if (pc_mfa_v3_near(kPcMfaV7StageGetThis, 54)) {
        pc_mfa_v7_unlock_bb();
        pc_mfa_v7_normal_finish(game);
        return;
    }

    /* Event groups 30-33: cake + child -> show big cake, write k3, animate at
     * 100 and set win at 200. */
    if (!pc_mfa_v7_k3() && pc_mfa_v7_cake_owned() &&
        pc_mfa_v3_near(kPcMfaStageCake, 62))
        pc_mfa_v7_begin_cake_sequence(game, false);

    pc_mfa_v7_tick_secret_sequence(game);
}

static void pc_mfa_v7_update_shadow(Game *game, const InputState *input)
{
    (void)pc_mfa_v7_apply_physics(input);

    if ((sSecret.frames % 3u) == 0u) {
        if (pc_mfa_v3_near(kPcMfaV4ShadowLeftZip, 34)) sSecret.x -= 10;
        static const PcMfaPoint zip_up = {1166, -184};
        static const PcMfaPoint zip_right = {3164, 466};
        if (pc_mfa_v3_near(zip_up, 34)) sSecret.y -= 10;
        if (pc_mfa_v3_near(zip_right, 34)) sSecret.x += 20;
    }
    pc_mfa_v3_apply_rwq_blocks();

    if (input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) % 5;
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.60f, false);
    }

    if (pc_mfa_v3_near(kPcMfaShadowExit, 58)) {
        pc_mfa_v7_normal_finish(game);
        return;
    }
    if (pc_mfa_v3_near(kPcMfaV7ShadowGetThis, 54)) {
        pc_mfa_v7_unlock_bb();
        pc_mfa_v7_normal_finish(game);
        return;
    }

    /* RWQ Event groups 26-29: the child/Active 6 trigger writes k4, then the
     * big cake counts 100/200 before win. */
    if (!pc_mfa_v7_k4() && pc_mfa_v7_cake_owned() &&
        pc_mfa_v3_near(kPcMfaV7ShadowCakeChild, 64))
        pc_mfa_v7_begin_cake_sequence(game, false);

    pc_mfa_v7_tick_secret_sequence(game);
}

static void pc_mfa_v7_update_happiest(Game *game, const InputState *input)
{
    (void)pc_mfa_v7_apply_physics(input);

    /* Marion has the same ordinary exit world coordinate as BB.  It exits
     * without touching k1..k4/goodend. */
    if (pc_mfa_v3_near(kPcMfaV7HappiestExit, 54)) {
        pc_mfa_v7_normal_finish(game);
        return;
    }
    if (pc_mfa_v3_near(kPcMfaV7HappiestGetThis, 54)) {
        pc_mfa_v7_unlock_bb();
        pc_mfa_v7_normal_finish(game);
        return;
    }

    /* Marion group 25 requires the four k-children to exist before trigger.
     * It writes goodend immediately, but the frame does not leave until the
     * final big-cake timer reaches 1300 (group 49). */
    if (!pc_mfa_v7_progress(6) && pc_mfa_v7_all_happiest_children() &&
        pc_mfa_v3_near(kPcMfaHappiestTrigger, 64))
        pc_mfa_v7_begin_cake_sequence(game, true);

    pc_mfa_v7_tick_secret_sequence(game);
}

static void pc_mfa_v7_reset_scene(Game *game)
{
    memset(&sPcMfaV7, 0, sizeof(sPcMfaV7));
    sPcMfaV7.active = true;
    sPcMfaV7.kind = sSecret.kind;
    pc_mfa_v7_infer_bb_flag();

    memset(&sPcMfaV3, 0, sizeof(sPcMfaV3));
    sPcMfaV3.active = true;
    sPcMfaV3.kind = sSecret.kind;
    sSecret.x = kPcMfaV3HitboxStart[sSecret.kind].x;
    sSecret.y = kPcMfaV3HitboxStart[sSecret.kind].y;
    sSecret.room = 0;
    sSecret.carried = 0;
    sSecret.delivered = 0;
    sSecret.collected = 0u;
    sSecret.frames = 0u;
    pc_mfa_v7_reset_physics();
    game->dirty = true;
}

static void pc_mfa_v7_update_active(Game *game, const InputState *input)
{
    if (!sPcMfaV7.active || sPcMfaV7.kind != sSecret.kind)
        pc_mfa_v7_reset_scene(game);

    ++sSecret.frames;
    ++game->screen_frames;

    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending &&
        !sPcMfaV7.cake_sequence && !sPcMfaV7.final_sequence) {
        secret_return_to_menu(game);
        sPcMfaV7.active = false;
        sPcMfaV3.active = false;
        sPcMfaV5.active = false;
        if (sPcMfaRunSound) {
            pc_minigame_sfx_stop(PC_MINIGAME_SFX_RUN);
            sPcMfaRunSound = false;
        }
        return;
    }

    if (sPcMfaV7.cake_sequence || sPcMfaV7.final_sequence) {
        /* The PC cake/final event chain freezes traversal while the visible
         * cake counter advances to its 100/200/300/1300 thresholds. */
        sPcMfaV5.moved_horizontal = false;
        pc_mfa_v5_update_run_sound();
        pc_mfa_v7_tick_secret_sequence(game);
        game->dirty = true;
        return;
    }

    if (sSecret.completed) {
        sPcMfaV5.moved_horizontal = false;
        pc_mfa_v5_update_run_sound();
        ++sSecret.completed_frames;
        game->dirty = true;
        if (sSecret.completed_frames >= 150u &&
            (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
             input_was_pressed(input, GAME_BUTTON_START) ||
             input_was_pressed(input, GAME_BUTTON_BACK))) {
            secret_return_to_menu(game);
            sPcMfaV7.active = false;
            sPcMfaV3.active = false;
            sPcMfaV5.active = false;
        }
        return;
    }

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_v7_update_bb(game, input); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_v7_update_mangle(game, input); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_v7_update_chica(game, input); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_v7_update_stage01(game, input); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_v7_update_shadow(game, input); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_v7_update_happiest(game, input); break;
        default: break;
    }
    pc_mfa_v5_update_run_sound();
    game->dirty = true;
}
