/* Fourth MFA fidelity pass.
 *
 * RWQFSFASXC is special: unlike the other minigame frames, crossing a page
 * edge does not move the Clickteam "watch screen" object.  The original S key
 * cycles a five-value `view` counter and explicitly places the watch screen at
 * five camera centres.  GAME_BUTTON_MODE is the Wii U mapping for that S key.
 * This layer also adds the Active 11 left-moving glitch strip present in both
 * Stage 01 and RWQFSFASXC (Every 50 ms -> hitbox X - 10). */

static const PcMfaPoint kPcMfaV4StageLeftZip = {319, 634};
static const PcMfaPoint kPcMfaV4ShadowLeftZip = {207, -136};

/* Exact watch-screen centres from RWQ event groups 40-44. */
static const PcMfaPoint kPcMfaV4ShadowViews[5] = {
    {512, 384}, {1536, 384}, {2560, 384}, {512, 1152}, {1536, 1152},
};

static void pc_mfa_v4_camera(int *camera_x, int *camera_y)
{
    if (sSecret.kind != SECRET_MINIGAME_SHADOW_BONNIE) {
        pc_mfa_v3_camera(camera_x, camera_y);
        return;
    }

    int view = sSecret.room;
    if (view < 0 || view >= 5) view = 0;
    *camera_x = kPcMfaV4ShadowViews[view].x - PC_MFA_CAMERA_W / 2;
    *camera_y = kPcMfaV4ShadowViews[view].y - PC_MFA_CAMERA_H / 2;
}

static void pc_mfa_v4_draw_shadow(void)
{
    int cx, cy;
    pc_mfa_v4_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       PC_MFA_SCREEN_X, 0, PC_MFA_SCREEN_W, PC_MFA_SCREEN_H,
                       GRAPHICS_RGB(2, 2, 3));

    static const uint32_t anchor_colours[4] = {
        GRAPHICS_RGB(57, 18, 70), GRAPHICS_RGB(89, 28, 105),
        GRAPHICS_RGB(38, 10, 53), GRAPHICS_RGB(108, 37, 126),
    };
    for (int i = 0; i < 4; ++i) {
        pc_mfa_draw_platform(kPcMfaShadowAnchors[i].x - 170,
                             kPcMfaShadowAnchors[i].y + 88,
                             340, 30, cx, cy, anchor_colours[i]);
    }
    for (int i = 0; i < 4; ++i) {
        const PcMfaRect r = kPcMfaShadowBlocks[i];
        pc_mfa_draw_platform(r.x, r.y, r.w, r.h, cx, cy,
                             GRAPHICS_RGB(20, 7, 28));
    }

    pc_mfa_draw_cake(kPcMfaShadowCake, cx, cy, false);
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV2ShadowKid, cx, cy, 78, 88);
    pc_mfa_draw_world_marker(kPcMfaShadowExit, cx, cy, COLOUR_WHITE);
    pc_mfa_v3_draw_player(&gPcSecretShadowBonniePlayerTexture,
                          cx, cy, 92, 120);
}

static bool pc_mfa_v4_secret_render_override(Game *game)
{
    if (!sSecret.active || sSecret.good_ending) return false;
    if (sSecret.kind != SECRET_MINIGAME_SHADOW_BONNIE)
        return pc_mfa_v3_secret_render_override(game);
    if (!game->dirty) return true;
    game->dirty = false;

    pc_mfa_v4_draw_shadow();
    if (sSecret.completed) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 220, 188, 414, 104,
                           GRAPHICS_RGB(3, 3, 3));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 220, 188, 414, 104, 3,
                            COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 282, 216, 3,
                           "MINIGAME COMPLETE", COLOUR_WHITE);
    }
    achievement_draw_overlay();
    graphics_present(GRAPHICS_TARGET_BOTH);
    return true;
}

static void pc_mfa_v4_apply_stage_left_zip(void)
{
    if (sSecret.kind != SECRET_MINIGAME_STAGE01 ||
        (sSecret.frames % 3u) != 0u) return;
    if (pc_mfa_v3_near(kPcMfaV4StageLeftZip, 34)) {
        sSecret.x -= 10;
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.34f, false);
    }
}

static void pc_mfa_v4_update_shadow(Game *game, const InputState *input)
{
    pc_mfa_v3_move_player(input);

    /* Exact Every-50-ms glitch strips decoded from the RWQ event sheet. */
    if ((sSecret.frames % 3u) == 0u) {
        if (pc_mfa_v3_near(kPcMfaV4ShadowLeftZip, 34))
            sSecret.x -= 10;
        static const PcMfaPoint zip_up = {1166, -184};
        static const PcMfaPoint zip_right = {3164, 466};
        if (pc_mfa_v3_near(zip_up, 34)) sSecret.y -= 10;
        if (pc_mfa_v3_near(zip_right, 34)) sSecret.x += 20;
    }
    pc_mfa_v3_apply_rwq_blocks();

    /* PC key S: view = (view % 5) + 1, followed by the five exact watch-screen
     * coordinates above.  On Wii U MODE is the direct equivalent control. */
    if (input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) % 5;
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.60f, false);
        game->dirty = true;
    }

    /* The final cake/child event chain still depends on the backdrop collision
     * masks being extracted.  Keep the portable completion trigger at the real
     * child/cake coordinates instead of the old artificial four-anchor hunt. */
    if (pc_mfa_v3_near(kPcMfaShadowKid, 64) ||
        pc_mfa_v3_near(kPcMfaShadowCake, 64)) {
        secret_complete(game);
    }
}

static void pc_mfa_v4_update_active(Game *game, const InputState *input)
{
    if (!sPcMfaV3.active || sPcMfaV3.kind != sSecret.kind) {
        memset(&sPcMfaV3, 0, sizeof(sPcMfaV3));
        sPcMfaV3.active = true;
        sPcMfaV3.kind = sSecret.kind;
        sSecret.x = kPcMfaV3HitboxStart[sSecret.kind].x;
        sSecret.y = kPcMfaV3HitboxStart[sSecret.kind].y;
        sSecret.room = 0;
        sSecret.frames = 0u;
        game->dirty = true;
    }

    ++sSecret.frames;
    ++game->screen_frames;
    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending) {
        secret_return_to_menu(game);
        sPcMfaV3.active = false;
        return;
    }
    if (sSecret.completed) {
        ++sSecret.completed_frames;
        game->dirty = true;
        if (sSecret.completed_frames >= 150u &&
            (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
             input_was_pressed(input, GAME_BUTTON_START) ||
             input_was_pressed(input, GAME_BUTTON_BACK))) {
            secret_return_to_menu(game);
            sPcMfaV3.active = false;
        }
        return;
    }

    if (sSecret.kind == SECRET_MINIGAME_SHADOW_BONNIE) {
        pc_mfa_v4_update_shadow(game, input);
    } else {
        switch (sSecret.kind) {
            case SECRET_MINIGAME_BB: pc_mfa_v3_update_bb(game, input); break;
            case SECRET_MINIGAME_MANGLE: pc_mfa_v3_update_mangle(game, input); break;
            case SECRET_MINIGAME_CHICA: pc_mfa_v3_update_chica(game, input); break;
            case SECRET_MINIGAME_STAGE01:
                pc_mfa_v3_update_stage01(game, input);
                pc_mfa_v4_apply_stage_left_zip();
                break;
            case SECRET_MINIGAME_HAPPIEST_DAY:
                pc_mfa_v3_update_happiest(game, input);
                break;
            default: break;
        }
    }
    game->dirty = true;
}

static void pc_mfa_v4_exact_update_game(Game *game, const InputState *input)
{
    const bool before_active = sSecret.active;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const int before_room = sSecret.room;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;

    if (sSecret.active && !sSecret.good_ending) {
        pc_mfa_v4_update_active(game, input);
    } else {
        sPcMfaV3.active = false;
        pc_mfa_v2_exact_update_game(game, input);
    }

    pc_mfa_v2_audio_after_update(input, before_active, before_collected,
                                 before_delivered, before_room,
                                 before_follow_event);
}
