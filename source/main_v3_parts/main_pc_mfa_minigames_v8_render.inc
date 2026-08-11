/* Eighth MFA fidelity pass: render the original PC minigame sprite-sheet
 * states instead of generic reconstruction primitives wherever the original
 * sheets expose the exact art.  V7 remains authoritative for gameplay. */

static const TextureRle *const kPcMfaV8BbBalloons[4] = {
    &gPcSecretBBBalloonRedTexture,
    &gPcSecretBBBalloonPurpleTexture,
    &gPcSecretBBBalloonGreenTexture,
    &gPcSecretBBBalloonOrangeTexture,
};

static const TextureRle *const kPcMfaV8MangleFrames[4] = {
    &gPcSecretManglePlayerTexture,
    &gPcSecretMangleAlt1Texture,
    &gPcSecretMangleAlt2Texture,
    &gPcSecretMangleAlt3Texture,
};

static const TextureRle *const kPcMfaV8MangleKidFrames[4] = {
    &gPcSecretMangleKid1Texture,
    &gPcSecretMangleKid2Texture,
    &gPcSecretMangleKid3Texture,
    &gPcSecretMangleKid4Texture,
};

static const TextureRle *const kPcMfaV8ShadowFrames[5] = {
    &gPcSecretShadowBonniePlayerTexture,
    &gPcSecretShadowAlt1Texture,
    &gPcSecretShadowAlt2Texture,
    &gPcSecretShadowAlt3Texture,
    &gPcSecretShadowAlt4Texture,
};

static const TextureRle *const kPcMfaV8HappiestGray[5] = {
    &gPcSecretHappiestGrayChild1Texture,
    &gPcSecretHappiestGrayChild2Texture,
    &gPcSecretHappiestGrayChild3Texture,
    &gPcSecretHappiestGrayChild4Texture,
    &gPcSecretHappiestGrayChild5Texture,
};

static const TextureRle *const kPcMfaV8HappiestMasks[5] = {
    &gPcSecretHappiestMask1Texture,
    &gPcSecretHappiestMask2Texture,
    &gPcSecretHappiestMask3Texture,
    &gPcSecretHappiestMask4Texture,
    &gPcSecretHappiestMask5Texture,
};

static const TextureRle *const kPcMfaV8HappiestBalloons[4] = {
    &gPcSecretHappiestBalloonPurpleTexture,
    &gPcSecretHappiestBalloonGreenTexture,
    &gPcSecretHappiestBalloonYellowTexture,
    &gPcSecretHappiestBalloonBlueTexture,
};

static void pc_mfa_v8_backdrop(const TextureRle *texture)
{
    pc_finishing_draw_sprite_centered(texture,
        PC_MFA_SCREEN_X + PC_MFA_SCREEN_W / 2,
        PC_MFA_SCREEN_H / 2,
        PC_MFA_SCREEN_W, PC_MFA_SCREEN_H);
}

static void pc_mfa_v8_world_sprite(const TextureRle *texture,
                                   PcMfaPoint point,
                                   int camera_x, int camera_y,
                                   int width, int height)
{
    pc_mfa_draw_sprite(texture, point, camera_x, camera_y, width, height);
}

static void pc_mfa_v8_world_rect_sprite(const TextureRle *texture,
                                        PcMfaRect rect,
                                        int camera_x, int camera_y)
{
    PcMfaPoint centre = {rect.x + rect.w / 2, rect.y + rect.h / 2};
    pc_mfa_draw_sprite(texture, centre, camera_x, camera_y, rect.w, rect.h);
}

static void pc_mfa_v8_draw_exit(PcMfaPoint point, int cx, int cy)
{
    pc_mfa_v8_world_sprite(&gPcSecretHappiestExitTexture,
                           point, cx, cy, 91, 117);
}

static void pc_mfa_v8_draw_cake(PcMfaPoint point, int cx, int cy,
                                bool coloured)
{
    pc_mfa_v8_world_sprite(coloured ? &gPcSecretHappiestCakeColorTexture
                                    : &gPcSecretHappiestCakeGrayTexture,
                           point, cx, cy, 144, 168);
}

static void pc_mfa_v8_draw_bb(void)
{
    int cx, cy;
    pc_mfa_v3_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_mfa_v8_backdrop(&gPcSecretBBBackdropTexture);

    for (int i = 0; i < 7; ++i) {
        if ((sSecret.collected & (uint8_t)(1u << i)) != 0u) continue;
        pc_mfa_v8_world_sprite(kPcMfaV8BbBalloons[i & 3],
                               kPcMfaBbCollect[i], cx, cy, 66, 104);
    }
    if (pc_mfa_v7_bb_unlocked()) {
        for (int i = 0; i < 8; ++i)
            pc_mfa_v8_world_sprite(kPcMfaV8BbBalloons[i & 3],
                                   kPcMfaBbBalloons[i], cx, cy, 66, 104);
    }

    if ((sSecret.collected & 0x7Fu) == 0x7Fu)
        pc_mfa_v8_draw_exit(kPcMfaBbExit, cx, cy);

    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k1()) {
        const bool phase_colour = pc_mfa_v7_k1() &&
            (!sPcMfaV7.cake_sequence || sPcMfaV7.sequence_frames >= 100u);
        pc_mfa_v8_world_sprite(
            sPcMfaV7.cake_sequence && sPcMfaV7.sequence_frames >= 100u
                ? &gPcSecretHappiestMask5Texture
                : &gPcSecretHappiestGrayChild5Texture,
            (PcMfaPoint){2484, 1106}, cx, cy,
            sPcMfaV7.cake_sequence && sPcMfaV7.sequence_frames >= 100u ? 59 : 75,
            sPcMfaV7.cake_sequence && sPcMfaV7.sequence_frames >= 100u ? 56 : 123);
        pc_mfa_v8_draw_cake(kPcMfaBbBigCake, cx, cy,
                            phase_colour || sPcMfaV7.sequence_frames >= 100u);
    }

    pc_mfa_v3_draw_player(&gPcSecretBBPlayerTexture, cx, cy, 109, 116);
}

static void pc_mfa_v8_draw_mangle(void)
{
    int cx, cy;
    pc_mfa_v3_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_mfa_v8_backdrop(&gPcSecretMangleBackdropTexture);

    for (int i = 0; i < 4; ++i) {
        if ((sSecret.collected & (uint8_t)(1u << i)) != 0u) continue;
        pc_mfa_v8_world_sprite(&gPcSecretManglePlayerTexture,
                               kPcMfaMangleParts[i], cx, cy, 92, 96);
    }
    if (pc_mfa_v7_bb_unlocked()) {
        for (int i = 0; i < 12; ++i)
            pc_mfa_v8_world_sprite(kPcMfaV8BbBalloons[i & 3],
                                   kPcMfaMangleBalloonPath[i], cx, cy, 66, 104);
    }

    const TextureRle *kid = kPcMfaV8MangleKidFrames[(sSecret.frames / 8u) & 3u];
    pc_mfa_v8_world_sprite(kid, kPcMfaV2MangleKid, cx, cy, 112, 112);

    if ((sSecret.collected & 0x0Fu) == 0x0Fu) {
        pc_mfa_v8_draw_exit(kPcMfaMangleExit, cx, cy);
        pc_mfa_v8_draw_exit(kPcMfaMangleAltExit, cx, cy);
    }

    /* The two MFA objects named `get this` are trigger objects, not visible
     * cake art.  V7 used a cake placeholder there; V8 deliberately leaves
     * those trigger coordinates invisible like the PC frame. */
    const TextureRle *player =
        kPcMfaV8MangleFrames[(sSecret.frames / 6u) & 3u];
    pc_mfa_v3_draw_player(player, cx, cy, 92, 96);
}

static void pc_mfa_v8_draw_chica(void)
{
    int cx, cy;
    pc_mfa_v3_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_mfa_v8_backdrop(&gPcSecretChicaBackdropTexture);

    for (int i = 0; i < 4; ++i) {
        const uint8_t pickup = (uint8_t)(1u << i);
        const uint8_t fed = (uint8_t)(1u << (i + 4));
        if ((sSecret.collected & pickup) == 0u)
            pc_mfa_v8_world_sprite(&gPcSecretChicaCupcakeSmallTexture,
                                   kPcMfaChicaCupcakes[i], cx, cy, 55, 52);

        pc_mfa_draw_sprite((i & 1) ? &gPcSecretChicaChildGreenTexture
                                   : &gPcSecretChicaChildBlueTexture,
                           kPcMfaChicaChildren[i], cx, cy, 90, 76);
        if ((sSecret.collected & fed) != 0u) {
            PcMfaPoint cake = {kPcMfaChicaChildren[i].x + 48,
                               kPcMfaChicaChildren[i].y};
            pc_mfa_v8_world_sprite(&gPcSecretChicaCupcakeSmallTexture,
                                   cake, cx, cy, 55, 52);
        }
    }

    if (sSecret.carried > 0) {
        PcMfaPoint carried = pc_mfa_v3_player_sprite_point();
        carried.y -= 58;
        pc_mfa_v8_world_sprite(&gPcSecretChicaCupcakeSmallTexture,
                               carried, cx, cy, 55, 52);
    }

    if (pc_mfa_v7_bb_unlocked()) {
        for (int i = 0; i < 2; ++i)
            pc_mfa_v8_world_sprite(kPcMfaV8BbBalloons[i],
                                   kPcMfaChicaBalloons[i], cx, cy, 66, 104);
    }
    if (sSecret.delivered >= 4) {
        pc_mfa_v8_draw_exit(kPcMfaChicaExit, cx, cy);
        pc_mfa_v8_draw_exit(kPcMfaV2ChicaExit2, cx, cy);
    }

    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k2()) {
        const bool second = sPcMfaV7.cake_sequence &&
                            sPcMfaV7.sequence_frames >= 100u;
        pc_mfa_v8_world_sprite(second ? &gPcSecretHappiestMask2Texture
                                      : &gPcSecretHappiestGrayChild2Texture,
                               kPcMfaV7ChicaCakeChild, cx, cy,
                               second ? 55 : 65, second ? 62 : 127);
        pc_mfa_v8_draw_cake(kPcMfaV7ChicaCakeTrigger, cx, cy,
                            second || pc_mfa_v7_k2());
    }

    pc_mfa_v3_draw_player(&gPcSecretChicaPlayerTexture, cx, cy, 90, 140);
}

static void pc_mfa_v8_draw_stage_piece(const PcMfaRect r, int cx, int cy)
{
    if (r.w == 358 && r.h == 92)
        pc_mfa_v8_world_rect_sprite(&gPcSecretStagePlatformTexture, r, cx, cy);
    else if (r.w == 762 && r.h == 30)
        pc_mfa_v8_world_rect_sprite(&gPcSecretStageFloorBarTexture, r, cx, cy);
    else if (r.w == 32 && r.h == 538)
        pc_mfa_v8_world_rect_sprite(&gPcSecretStageVertical538Texture, r, cx, cy);
    else if (r.w == 32 && r.h == 562)
        pc_mfa_v8_world_rect_sprite(&gPcSecretStageVertical562Texture, r, cx, cy);
}

static void pc_mfa_v8_draw_stage01(void)
{
    int cx, cy;
    pc_mfa_v3_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       PC_MFA_SCREEN_X, 0, PC_MFA_SCREEN_W, PC_MFA_SCREEN_H,
                       GRAPHICS_RGB(2, 2, 3));

    for (unsigned i = 0;
         i < sizeof(kPcMfaV5StageSolids) / sizeof(kPcMfaV5StageSolids[0]); ++i)
        pc_mfa_v8_draw_stage_piece(kPcMfaV5StageSolids[i], cx, cy);

    for (int i = 0; i < 23; ++i)
        pc_mfa_v8_world_sprite(&gPcSecretStageChildTexture,
                               kPcMfaV2StageChildren[i], cx, cy, 81, 82);

    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k3()) {
        const bool second = sPcMfaV7.cake_sequence &&
                            sPcMfaV7.sequence_frames >= 100u;
        pc_mfa_v8_world_sprite(second ? &gPcSecretHappiestMask3Texture
                                      : &gPcSecretStageChildTexture,
                               kPcMfaV7StageCakeChild, cx, cy,
                               second ? 64 : 81, second ? 57 : 82);
        pc_mfa_v8_draw_cake(kPcMfaStageCake, cx, cy,
                            second || pc_mfa_v7_k3());
    }
    pc_mfa_v8_draw_exit(kPcMfaStageExit, cx, cy);
    pc_mfa_v3_draw_player(&gPcSecretStagePlayerTexture, cx, cy, 75, 163);
}

static void pc_mfa_v8_draw_shadow(void)
{
    int cx, cy;
    pc_mfa_v4_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_mfa_v8_backdrop(&gPcSecretShadowBackdropTexture);

    for (int i = 0; i < 4; ++i) {
        const PcMfaRect r = kPcMfaShadowBlocks[i];
        pc_mfa_draw_platform(r.x, r.y, r.w, r.h, cx, cy,
                             GRAPHICS_RGB(20, 7, 28));
    }

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

static PcMfaPoint pc_mfa_v8_happiest_child_point(int index)
{
    if (index < 4) return kPcMfaHappiestChildren[index];
    return kPcMfaV7HappiestFinalChild;
}

static const TextureRle *pc_mfa_v8_happiest_colour_child(int index)
{
    if (index < 4) return pc_secret_happiest_child_texture(index);
    return &gPcSecretHappiestChild5Texture;
}

static void pc_mfa_v8_happiest_table(PcMfaPoint cake, int cx, int cy,
                                     bool grey)
{
    PcMfaPoint table = {cake.x, cake.y + 132};
    pc_mfa_v8_world_sprite(grey ? &gPcSecretHappiestTableGrayTexture
                                : &gPcSecretHappiestTableColorTexture,
                           table, cx, cy, 260, 106);
}

static void pc_mfa_v8_draw_happiest(void)
{
    int cx, cy;
    pc_mfa_v3_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       PC_MFA_SCREEN_X, 0, PC_MFA_SCREEN_W, PC_MFA_SCREEN_H,
                       GRAPHICS_RGB(1, 1, 1));

    for (int i = 0; i < 4; ++i) {
        const bool final_cake = i == 3;
        bool grey = final_cake && !sPcMfaV7.final_sequence &&
                    !pc_mfa_v7_progress(6);
        if (sPcMfaV7.final_sequence && final_cake)
            grey = sPcMfaV7.sequence_frames < 100u;
        pc_mfa_v8_happiest_table(kPcMfaHappiestCakes[i], cx, cy, grey);
        pc_mfa_v8_draw_cake(kPcMfaHappiestCakes[i], cx, cy, !grey);
    }

    const bool prereq[4] = {
        pc_mfa_v7_k1(), pc_mfa_v7_k2(), pc_mfa_v7_k3(), pc_mfa_v7_k4()
    };
    const uint32_t phase = sPcMfaV7.final_sequence
        ? sPcMfaV7.sequence_frames : 0u;

    if (!sPcMfaV7.final_sequence) {
        for (int i = 0; i < 4; ++i) {
            if (!prereq[i]) continue;
            pc_mfa_v8_world_sprite(pc_mfa_v8_happiest_colour_child(i),
                                   pc_mfa_v8_happiest_child_point(i),
                                   cx, cy, 94, 146);
        }
        if (pc_mfa_v7_all_happiest_children())
            pc_mfa_v8_world_sprite(&gPcSecretHappiestChild5Texture,
                                   kPcMfaV7HappiestFinalChild,
                                   cx, cy, 86, 130);
    } else if (phase < 100u) {
        for (int i = 0; i < 5; ++i)
            pc_mfa_v8_world_sprite(pc_mfa_v8_happiest_colour_child(i),
                                   pc_mfa_v8_happiest_child_point(i),
                                   cx, cy, i == 4 ? 86 : 94,
                                   i == 4 ? 130 : 146);
    } else if (phase < 200u) {
        for (int i = 0; i < 5; ++i)
            pc_mfa_v8_world_sprite(kPcMfaV8HappiestGray[i],
                                   pc_mfa_v8_happiest_child_point(i),
                                   cx, cy, 88, 124);
    } else if (phase < 300u) {
        for (int i = 0; i < 5; ++i)
            pc_mfa_v8_world_sprite(kPcMfaV8HappiestMasks[i],
                                   pc_mfa_v8_happiest_child_point(i),
                                   cx, cy, 64, 58);
    } else {
        const int rise = (int)((phase - 300u) / 6u) * 10;
        for (int i = 0; i < 6; ++i) {
            PcMfaPoint p = kPcMfaV2HappiestCollect3[i];
            p.y -= rise;
            pc_mfa_v8_world_sprite(kPcMfaV8HappiestBalloons[i & 3],
                                   p, cx, cy, 66, 104);
        }
        for (int i = 0; i < 10; ++i) {
            PcMfaPoint p = kPcMfaV2HappiestCollect4[i];
            p.y -= rise;
            pc_mfa_v8_world_sprite(kPcMfaV8HappiestBalloons[(i + 2) & 3],
                                   p, cx, cy, 66, 104);
        }
    }

    if (!sPcMfaV7.final_sequence)
        pc_mfa_v8_draw_exit(kPcMfaV7HappiestExit, cx, cy);

    const TextureRle *puppet =
        sPcMfaV7.final_sequence && phase >= 100u && phase < 300u
            ? &gPcSecretHappiestPuppetAltTexture
            : &gPcSecretHappiestPlayerTexture;
    pc_mfa_v3_draw_player(puppet, cx, cy, 82, 104);
}

static bool pc_mfa_v8_secret_render_override(Game *game)
{
    if (!sSecret.active || sSecret.good_ending) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_v8_draw_bb(); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_v8_draw_mangle(); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_v8_draw_chica(); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_v8_draw_stage01(); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_v8_draw_shadow(); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_v8_draw_happiest(); break;
        default: return false;
    }

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
