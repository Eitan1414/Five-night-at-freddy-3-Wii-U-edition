/* Ninth MFA fidelity pass.
 *
 * V8 already uses the extracted PC sprite sheets for the six secret games,
 * but two compatibility artefacts were still visible:
 *  - Happiest Day stopped using V8 as soon as goodend was written, even though
 *    the MFA keeps the frame alive through the 1300-count finale.
 *  - Shadow Bonnie drew generic purple rectangles on top of the exact backdrop.
 *
 * Keep V8's decoded gameplay/art and only replace those presentation leaks.
 */

static void pc_mfa_v9_draw_shadow(void)
{
    int cx, cy;
    pc_mfa_v4_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_mfa_v8_backdrop(&gPcSecretShadowBackdropTexture);

    /* kPcMfaShadowBlocks remain authoritative for collision in V5. They are
     * Backdrop/collision geometry, not a license to paint replacement blocks
     * over the extracted PC sheet. */
    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k4()) {
        const bool second = sPcMfaV7.cake_sequence &&
                            sPcMfaV7.sequence_frames >= 100u;
        pc_mfa_v8_world_sprite(second ? &gPcSecretShadowChild2Texture
                                      : &gPcSecretShadowChild1Texture,
                               kPcMfaV7ShadowCakeChild, cx, cy, 94, 98);
        pc_mfa_v8_draw_cake(kPcMfaShadowCake, cx, cy,
                            second || pc_mfa_v7_k4());
    } else {
        pc_mfa_v8_world_sprite(&gPcSecretShadowChild1Texture,
                               kPcMfaV7ShadowCakeChild, cx, cy, 94, 98);
    }

    pc_mfa_v8_draw_exit(kPcMfaShadowExit, cx, cy);
    const TextureRle *player =
        kPcMfaV8ShadowFrames[(sSecret.frames / 7u) % 5u];
    pc_mfa_v3_draw_player(player, cx, cy, 92, 120);
}

static bool pc_mfa_v9_secret_render_override(Game *game)
{
    if (!sSecret.active) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB:
            pc_mfa_v8_draw_bb();
            break;
        case SECRET_MINIGAME_MANGLE:
            pc_mfa_v8_draw_mangle();
            break;
        case SECRET_MINIGAME_CHICA:
            pc_mfa_v8_draw_chica();
            break;
        case SECRET_MINIGAME_STAGE01:
            pc_mfa_v8_draw_stage01();
            break;
        case SECRET_MINIGAME_SHADOW_BONNIE:
            pc_mfa_v9_draw_shadow();
            break;
        case SECRET_MINIGAME_HAPPIEST_DAY:
            /* Do not gate this on sSecret.good_ending. MFA Marion writes
             * goodend at the start of the finale, then continues the same
             * frame through 100/200/300/1300-count visual stages. */
            pc_mfa_v8_draw_happiest();
            break;
        default:
            return false;
    }

    /* The old Wii U renderer added a centred "MINIGAME COMPLETE" dialog.
     * The PC frames complete through their own win/cake/exit events, so leave
     * the extracted scene unobstructed while the completion timer resolves. */
    achievement_draw_overlay();
    graphics_present(GRAPHICS_TARGET_BOTH);
    return true;
}
