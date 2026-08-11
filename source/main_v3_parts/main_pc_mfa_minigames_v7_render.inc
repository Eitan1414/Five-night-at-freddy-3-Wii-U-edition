static void pc_mfa_v7_draw_bb(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(0, 640, 1024, 32, cx, cy, GRAPHICS_RGB(34, 143, 38));
    pc_mfa_draw_platform(1024, 1130, 760, 28, cx, cy, GRAPHICS_RGB(36, 151, 41));
    pc_mfa_draw_platform(2048, 1098, 690, 30, cx, cy, GRAPHICS_RGB(36, 151, 41));

    static const uint32_t collect_colours[7] = {
        GRAPHICS_RGB(229,50,61), GRAPHICS_RGB(244,203,46),
        GRAPHICS_RGB(76,190,83), GRAPHICS_RGB(60,126,221),
        GRAPHICS_RGB(192,73,203), GRAPHICS_RGB(229,50,61),
        GRAPHICS_RGB(244,203,46),
    };
    for (int i = 0; i < 7; ++i)
        if ((sSecret.collected & (uint8_t)(1u << i)) == 0u)
            pc_mfa_draw_balloon(kPcMfaBbCollect[i], cx, cy, collect_colours[i]);

    if (pc_mfa_v7_bb_unlocked()) {
        for (int i = 0; i < 8; ++i)
            pc_mfa_draw_balloon(kPcMfaBbBalloons[i], cx, cy,
                                GRAPHICS_RGB(78 + i * 17, 115 + i * 9, 205 - i * 11));
    }

    if ((sSecret.collected & 0x7Fu) == 0x7Fu)
        pc_mfa_draw_world_marker(kPcMfaBbExit, cx, cy, COLOUR_WHITE);
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       (PcMfaPoint){2484,1106}, cx, cy, 78, 88);
    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k1())
        pc_mfa_draw_cake(kPcMfaBbBigCake, cx, cy, true);
    pc_mfa_v3_draw_player(&gPcSecretBBPlayerTexture, cx, cy, 92, 116);
}

static void pc_mfa_v7_draw_mangle(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(0, 690, 3072, 24, cx, cy, GRAPHICS_RGB(172,26,38));
    pc_mfa_draw_platform(300, 1280, 1500, 24, cx, cy, GRAPHICS_RGB(172,26,38));
    pc_mfa_draw_platform(400, 2040, 1120, 24, cx, cy, GRAPHICS_RGB(172,26,38));
    for (int i = 0; i < 4; ++i) {
        if ((sSecret.collected & (uint8_t)(1u << i)) != 0u) continue;
        pc_mfa_draw_sprite(&gPcSecretManglePlayerTexture,
                           kPcMfaMangleParts[i], cx, cy, 58, 62);
    }
    if (pc_mfa_v7_bb_unlocked())
        for (int i = 0; i < 12; ++i)
            pc_mfa_draw_balloon(kPcMfaMangleBalloonPath[i], cx, cy,
                                GRAPHICS_RGB(194,49,66));
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV2MangleKid, cx, cy, 78, 88);
    if ((sSecret.collected & 0x0Fu) == 0x0Fu) {
        pc_mfa_draw_world_marker(kPcMfaMangleExit, cx, cy, COLOUR_WHITE);
        pc_mfa_draw_world_marker(kPcMfaMangleAltExit, cx, cy,
                                 GRAPHICS_RGB(150,150,150));
    }
    if (!pc_mfa_v7_cake_owned())
        for (int i = 0; i < 2; ++i)
            pc_mfa_draw_cake(kPcMfaV7MangleGetThis[i], cx, cy, true);
    pc_mfa_v3_draw_player(&gPcSecretManglePlayerTexture, cx, cy, 92, 96);
}

static void pc_mfa_v7_draw_chica(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(0, 650, 1024, 30, cx, cy, GRAPHICS_RGB(40,151,44));
    pc_mfa_draw_platform(1024, 1320, 1024, 30, cx, cy, GRAPHICS_RGB(40,151,44));
    pc_mfa_draw_platform(2048, 720, 1024, 30, cx, cy, GRAPHICS_RGB(40,151,44));
    for (int i = 0; i < 4; ++i) {
        const uint8_t pickup = (uint8_t)(1u << i);
        const uint8_t fed = (uint8_t)(1u << (i + 4));
        if ((sSecret.collected & pickup) == 0u &&
            pc_mfa_visible(kPcMfaChicaCupcakes[i].x, kPcMfaChicaCupcakes[i].y,
                           cx, cy, 80))
            content_draw_cupcake(pc_mfa_screen_x(kPcMfaChicaCupcakes[i].x, cx),
                                 pc_mfa_screen_y(kPcMfaChicaCupcakes[i].y, cy));
        pc_mfa_draw_sprite((i & 1) ? &gPcSecretChicaChildGreenTexture
                                    : &gPcSecretChicaChildBlueTexture,
                           kPcMfaChicaChildren[i], cx, cy, 84, 92);
        if ((sSecret.collected & fed) != 0u) {
            PcMfaPoint p = {kPcMfaChicaChildren[i].x + 48,
                            kPcMfaChicaChildren[i].y};
            if (pc_mfa_visible(p.x, p.y, cx, cy, 80))
                content_draw_cupcake(pc_mfa_screen_x(p.x, cx),
                                     pc_mfa_screen_y(p.y, cy));
        }
    }
    if (sSecret.carried > 0) {
        PcMfaPoint p = pc_mfa_v3_player_sprite_point();
        p.y -= 56;
        if (pc_mfa_visible(p.x,p.y,cx,cy,60))
            content_draw_cupcake(pc_mfa_screen_x(p.x,cx), pc_mfa_screen_y(p.y,cy));
    }
    if (pc_mfa_v7_bb_unlocked())
        for (int i = 0; i < 2; ++i)
            pc_mfa_draw_balloon(kPcMfaChicaBalloons[i], cx, cy,
                                GRAPHICS_RGB(227,55 + i * 90,79));
    if (sSecret.delivered >= 4) {
        pc_mfa_draw_world_marker(kPcMfaChicaExit, cx, cy, COLOUR_WHITE);
        pc_mfa_draw_world_marker(kPcMfaV2ChicaExit2, cx, cy,
                                 GRAPHICS_RGB(150,150,150));
    }
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV7ChicaCakeChild, cx, cy, 78, 88);
    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k2())
        pc_mfa_draw_cake(kPcMfaV7ChicaCakeTrigger, cx, cy, true);
    pc_mfa_v3_draw_player(&gPcSecretChicaPlayerTexture, cx, cy, 88, 112);
}

static void pc_mfa_v7_draw_stage01(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    for (int i = 0; i < 8; ++i) {
        const PcMfaPoint p = kPcMfaStageNodes[i];
        pc_mfa_draw_platform(p.x - 180, p.y + 96, 360, 34, cx, cy,
                             GRAPHICS_RGB(107,47,119));
    }
    for (int i = 0; i < 23; ++i)
        pc_mfa_draw_sprite(&gPcSecretStageChildTexture,
                           kPcMfaV2StageChildren[i], cx, cy, 82, 90);
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV7StageCakeChild, cx, cy, 78, 88);
    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k3())
        pc_mfa_draw_cake(kPcMfaStageCake, cx, cy, true);
    pc_mfa_draw_world_marker(kPcMfaStageExit, cx, cy, COLOUR_WHITE);
    pc_mfa_v3_draw_player(&gPcSecretStagePlayerTexture, cx, cy, 90, 116);
}

static void pc_mfa_v7_draw_shadow(void)
{
    int cx, cy;
    pc_mfa_v4_camera(&cx, &cy);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       PC_MFA_SCREEN_X, 0, PC_MFA_SCREEN_W, PC_MFA_SCREEN_H,
                       GRAPHICS_RGB(2,2,3));
    static const uint32_t colours[4] = {
        GRAPHICS_RGB(57,18,70), GRAPHICS_RGB(89,28,105),
        GRAPHICS_RGB(38,10,53), GRAPHICS_RGB(108,37,126),
    };
    for (int i = 0; i < 4; ++i)
        pc_mfa_draw_platform(kPcMfaShadowAnchors[i].x - 170,
                             kPcMfaShadowAnchors[i].y + 88,
                             340,30,cx,cy,colours[i]);
    for (int i = 0; i < 4; ++i) {
        const PcMfaRect r = kPcMfaShadowBlocks[i];
        pc_mfa_draw_platform(r.x,r.y,r.w,r.h,cx,cy,GRAPHICS_RGB(20,7,28));
    }
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV7ShadowCakeChild, cx, cy, 78, 88);
    if (sPcMfaV7.cake_sequence || pc_mfa_v7_k4())
        pc_mfa_draw_cake(kPcMfaShadowCake, cx, cy, true);
    pc_mfa_draw_world_marker(kPcMfaShadowExit, cx, cy, COLOUR_WHITE);
    pc_mfa_v3_draw_player(&gPcSecretShadowBonniePlayerTexture,cx,cy,92,120);
}

static void pc_mfa_v7_draw_happiest(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(400,650,2350,32,cx,cy,GRAPHICS_RGB(42,42,42));

    for (int i = 0; i < 5; ++i)
        pc_mfa_draw_world_marker(kPcMfaHappiestGuests[i],cx,cy,
                                 GRAPHICS_RGB(94,94,94));

    const bool flags[4] = {
        pc_mfa_v7_k1(), pc_mfa_v7_k2(), pc_mfa_v7_k3(), pc_mfa_v7_k4()
    };
    const bool celebration = sPcMfaV7.final_sequence &&
                             sPcMfaV7.sequence_frames >= 300u;
    for (int i = 0; i < 4; ++i) {
        if (!flags[i]) continue;
        pc_mfa_draw_sprite(pc_secret_happiest_child_texture(i),
                           kPcMfaHappiestChildren[i],cx,cy,100,126);
    }

    /* Three pre-existing party cakes plus the final good-ending cake. */
    for (int i = 0; i < 3; ++i)
        pc_mfa_draw_cake(kPcMfaHappiestCakes[i],cx,cy,false);
    if (sPcMfaV7.final_sequence || pc_mfa_v7_progress(6))
        pc_mfa_draw_cake(kPcMfaHappiestCakes[3],cx,cy,true);

    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV7HappiestFinalChild,cx,cy,78,88);

    /* Marion groups 45-48 start moving the collect-3/collect-4 balloon rows
     * after the 300-count celebration state.  Preserve every original start
     * coordinate and move them upward at the event-sheet cadence. */
    if (celebration) {
        const int rise = (int)((sPcMfaV7.sequence_frames - 300u) / 6u) * 10;
        for (int i = 0; i < 6; ++i) {
            PcMfaPoint p = kPcMfaV2HappiestCollect3[i];
            p.y -= rise;
            pc_mfa_draw_balloon(p,cx,cy,GRAPHICS_RGB(91,164,230));
        }
        for (int i = 0; i < 10; ++i) {
            PcMfaPoint p = kPcMfaV2HappiestCollect4[i];
            p.y -= rise;
            pc_mfa_draw_balloon(p,cx,cy,GRAPHICS_RGB(231,92,118));
        }
    }

    pc_mfa_v3_draw_player(&gPcSecretHappiestPlayerTexture,cx,cy,82,104);
}

static bool pc_mfa_v7_secret_render_override(Game *game)
{
    if (!sSecret.active || sSecret.good_ending) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_v7_draw_bb(); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_v7_draw_mangle(); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_v7_draw_chica(); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_v7_draw_stage01(); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_v7_draw_shadow(); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_v7_draw_happiest(); break;
        default: return false;
    }

    if (sSecret.completed) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,220,188,414,104,
                           GRAPHICS_RGB(3,3,3));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH,220,188,414,104,3,COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH,282,216,3,
                           "MINIGAME COMPLETE",COLOUR_WHITE);
    }
    achievement_draw_overlay();
    graphics_present(GRAPHICS_TARGET_BOTH);
    return true;
}

static void pc_mfa_v7_exact_update_game(Game *game, const InputState *input)
{
    const bool before_active = sSecret.active;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;

    if (sSecret.active && !sSecret.good_ending) {
        pc_mfa_v7_update_active(game,input);
    } else {
        sPcMfaV7.active = false;
        sPcMfaV3.active = false;
        sPcMfaV5.active = false;
        pc_mfa_v2_exact_update_game(game,input);
    }

    pc_mfa_v5_audio_after_update(before_active,before_collected,
                                 before_delivered,before_follow_event);
}
