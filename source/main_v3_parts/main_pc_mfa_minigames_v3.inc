/* Third MFA fidelity pass: reproduce the event-sheet coordinate model.
 *
 * The original project does not smoothly follow the player.  A hidden
 * "watch screen" object starts at 512,384 and moves by exactly 1024 or 768
 * pixels when the hitbox crosses a page edge.  The visible character is also
 * not the authoritative position: every frame it is positioned relative to
 * the hidden hitbox.  Those rules and the movement/zip/block deltas below are
 * taken directly from fivenights3-94.mfa event records. */

typedef struct PcMfaV3State {
    bool active;
    SecretMinigameKind kind;
    uint32_t last_special_frame;
} PcMfaV3State;

static PcMfaV3State sPcMfaV3;

/* Authoritative hidden hitbox starts, not the decorative Active sprite. */
static const PcMfaPoint kPcMfaV3HitboxStart[SECRET_MINIGAME_COUNT] = {
    {309, 589}, /* BB */
    {445, 595}, /* Mangle */
    {309, 589}, /* Toy Chica */
    {255, 505}, /* Stage 01 / GFreddy */
    {207, 501}, /* RWQFSFASXC */
    {309, 589}, /* Happiest Day / Marion */
};

/* The Active character is attached to the hitbox by these exact MFA offsets. */
static const PcMfaPoint kPcMfaV3SpriteOffset[SECRET_MINIGAME_COUNT] = {
    {0, -10}, {0, -45}, {0, -20}, {0, -35}, {0, -35}, {0, -10},
};

static int pc_mfa_v3_floor_div(int value, int divisor)
{
    if (value >= 0) return value / divisor;
    return -(((-value) + divisor - 1) / divisor);
}

static void pc_mfa_v3_camera(int *camera_x, int *camera_y)
{
    int page_x = pc_mfa_v3_floor_div(sSecret.x, PC_MFA_CAMERA_W);
    int page_y = pc_mfa_v3_floor_div(sSecret.y, PC_MFA_CAMERA_H);

    /* Clickteam cannot scroll the 3072x2304 frame past its three real pages. */
    page_x = pc_mfa_clamp(page_x, 0, 2);
    page_y = pc_mfa_clamp(page_y, 0, 2);
    *camera_x = page_x * PC_MFA_CAMERA_W;
    *camera_y = page_y * PC_MFA_CAMERA_H;
}

static void pc_mfa_v3_begin_scene(int *camera_x, int *camera_y)
{
    pc_mfa_v3_camera(camera_x, camera_y);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       PC_MFA_SCREEN_X, 0, PC_MFA_SCREEN_W, PC_MFA_SCREEN_H,
                       GRAPHICS_RGB(2, 2, 3));
}

static PcMfaPoint pc_mfa_v3_player_sprite_point(void)
{
    const PcMfaPoint offset = kPcMfaV3SpriteOffset[sSecret.kind];
    PcMfaPoint point = {sSecret.x + offset.x, sSecret.y + offset.y};
    return point;
}

static void pc_mfa_v3_draw_player(const TextureRle *texture,
                                  int camera_x, int camera_y,
                                  int width, int height)
{
    pc_mfa_draw_sprite(texture, pc_mfa_v3_player_sprite_point(),
                       camera_x, camera_y, width, height);
}

static void pc_mfa_v3_draw_bb(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(0, 640, 1024, 32, cx, cy, GRAPHICS_RGB(34, 143, 38));
    pc_mfa_draw_platform(1024, 1130, 760, 28, cx, cy, GRAPHICS_RGB(36, 151, 41));
    pc_mfa_draw_platform(2048, 1098, 690, 30, cx, cy, GRAPHICS_RGB(36, 151, 41));
    static const uint32_t colours[8] = {
        GRAPHICS_RGB(229, 50, 61), GRAPHICS_RGB(244, 203, 46),
        GRAPHICS_RGB(76, 190, 83), GRAPHICS_RGB(60, 126, 221),
        GRAPHICS_RGB(192, 73, 203), GRAPHICS_RGB(229, 50, 61),
        GRAPHICS_RGB(244, 203, 46), GRAPHICS_RGB(76, 190, 83),
    };
    for (int i = 0; i < 8; ++i)
        if ((sSecret.collected & (uint8_t)(1u << i)) == 0u)
            pc_mfa_draw_balloon(kPcMfaBbBalloons[i], cx, cy, colours[i]);
    pc_mfa_draw_cake(kPcMfaBbBigCake, cx, cy, true);
    pc_mfa_draw_world_marker(kPcMfaBbExit, cx, cy, COLOUR_WHITE);
    pc_mfa_v3_draw_player(&gPcSecretBBPlayerTexture, cx, cy, 92, 116);
}

static void pc_mfa_v3_draw_mangle(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(0, 690, 3072, 24, cx, cy, GRAPHICS_RGB(172, 26, 38));
    pc_mfa_draw_platform(300, 1280, 1500, 24, cx, cy, GRAPHICS_RGB(172, 26, 38));
    pc_mfa_draw_platform(400, 2040, 1120, 24, cx, cy, GRAPHICS_RGB(172, 26, 38));
    for (int i = 0; i < 4; ++i) {
        if ((sSecret.collected & (uint8_t)(1u << i)) != 0u) continue;
        pc_mfa_draw_sprite(&gPcSecretManglePlayerTexture,
                           kPcMfaMangleParts[i], cx, cy, 58, 62);
    }
    for (int i = 0; i < 12; ++i)
        pc_mfa_draw_balloon(kPcMfaMangleBalloonPath[i], cx, cy,
                            GRAPHICS_RGB(194, 49, 66));
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV2MangleKid, cx, cy, 78, 88);
    pc_mfa_draw_world_marker(kPcMfaMangleExit, cx, cy, COLOUR_WHITE);
    pc_mfa_draw_world_marker(kPcMfaMangleAltExit, cx, cy,
                             GRAPHICS_RGB(150, 150, 150));
    pc_mfa_v3_draw_player(&gPcSecretManglePlayerTexture, cx, cy, 92, 96);
}

static void pc_mfa_v3_draw_chica(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(0, 650, 1024, 30, cx, cy, GRAPHICS_RGB(40, 151, 44));
    pc_mfa_draw_platform(1024, 1320, 1024, 30, cx, cy, GRAPHICS_RGB(40, 151, 44));
    pc_mfa_draw_platform(2048, 720, 1024, 30, cx, cy, GRAPHICS_RGB(40, 151, 44));
    for (int i = 0; i < 4; ++i) {
        const uint8_t pickup = (uint8_t)(1u << i);
        const uint8_t delivered = (uint8_t)(1u << (i + 4));
        if ((sSecret.collected & pickup) == 0u) {
            const PcMfaPoint p = kPcMfaChicaCupcakes[i];
            if (pc_mfa_visible(p.x, p.y, cx, cy, 80))
                content_draw_cupcake(pc_mfa_screen_x(p.x, cx),
                                     pc_mfa_screen_y(p.y, cy));
        }
        pc_mfa_draw_sprite((i & 1) ? &gPcSecretChicaChildGreenTexture
                                    : &gPcSecretChicaChildBlueTexture,
                           kPcMfaChicaChildren[i], cx, cy, 84, 92);
        if ((sSecret.collected & delivered) != 0u) {
            PcMfaPoint p = {kPcMfaChicaChildren[i].x + 48,
                            kPcMfaChicaChildren[i].y};
            if (pc_mfa_visible(p.x, p.y, cx, cy, 80))
                content_draw_cupcake(pc_mfa_screen_x(p.x, cx),
                                     pc_mfa_screen_y(p.y, cy));
        }
    }
    for (int i = 0; i < 2; ++i)
        pc_mfa_draw_balloon(kPcMfaChicaBalloons[i], cx, cy,
                            GRAPHICS_RGB(227, 55 + i * 90, 79));
    pc_mfa_draw_world_marker(kPcMfaChicaExit, cx, cy, COLOUR_WHITE);
    pc_mfa_draw_world_marker(kPcMfaV2ChicaExit2, cx, cy,
                             GRAPHICS_RGB(150, 150, 150));
    pc_mfa_v3_draw_player(&gPcSecretChicaPlayerTexture, cx, cy, 88, 112);
}

static void pc_mfa_v3_draw_stage01(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    for (int i = 0; i < 8; ++i) {
        const PcMfaPoint p = kPcMfaStageNodes[i];
        pc_mfa_draw_platform(p.x - 180, p.y + 96, 360, 34, cx, cy,
                             GRAPHICS_RGB(107, 47, 119));
    }
    for (int i = 0; i < 23; ++i)
        pc_mfa_draw_sprite(&gPcSecretStageChildTexture,
                           kPcMfaV2StageChildren[i], cx, cy, 82, 90);
    pc_mfa_draw_cake(kPcMfaStageCake, cx, cy, true);
    pc_mfa_v3_draw_player(&gPcSecretStagePlayerTexture, cx, cy, 90, 116);
}

static void pc_mfa_v3_draw_shadow(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
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

static void pc_mfa_v3_draw_happiest(void)
{
    int cx, cy;
    pc_mfa_v3_begin_scene(&cx, &cy);
    pc_mfa_draw_platform(400, 650, 2350, 32, cx, cy, GRAPHICS_RGB(42, 42, 42));
    for (int i = 0; i < 5; ++i)
        pc_mfa_draw_world_marker(kPcMfaHappiestGuests[i], cx, cy,
                                 GRAPHICS_RGB(94, 94, 94));
    for (int i = 0; i < 4; ++i) {
        const TextureRle *child = pc_secret_happiest_child_texture(i);
        pc_mfa_draw_sprite(child, kPcMfaHappiestChildren[i], cx, cy, 100, 126);
        if ((sSecret.collected & (uint8_t)(1u << i)) != 0u) {
            PcMfaPoint balloon = {kPcMfaHappiestChildren[i].x,
                                  kPcMfaHappiestChildren[i].y - 160};
            pc_mfa_draw_balloon(balloon, cx, cy,
                                GRAPHICS_RGB(80 + i * 35, 150, 90 + i * 28));
        }
    }
    for (int i = 0; i < 4; ++i)
        pc_mfa_draw_cake(kPcMfaHappiestCakes[i], cx, cy,
                         i == 3 && sSecret.delivered >= 4);
    pc_mfa_v3_draw_player(&gPcSecretHappiestPlayerTexture, cx, cy, 82, 104);
}

static bool pc_mfa_v3_secret_render_override(Game *game)
{
    if (!sSecret.active || sSecret.good_ending) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_v3_draw_bb(); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_v3_draw_mangle(); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_v3_draw_chica(); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_v3_draw_stage01(); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_v3_draw_shadow(); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_v3_draw_happiest(); break;
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

static bool pc_mfa_v3_near(PcMfaPoint point, int radius)
{
    return secret_near(sSecret.x, sSecret.y, point.x, point.y, radius);
}

static bool pc_mfa_v3_inside_rect(PcMfaRect rect, int margin)
{
    return sSecret.x >= rect.x - margin &&
           sSecret.x <= rect.x + rect.w + margin &&
           sSecret.y >= rect.y - margin &&
           sSecret.y <= rect.y + rect.h + margin;
}

static void pc_mfa_v3_move_player(const InputState *input)
{
    /* Event sheet: A/D move 15 px per event, except Marion which uses 20. */
    const int horizontal =
        sSecret.kind == SECRET_MINIGAME_HAPPIEST_DAY ? 20 : 15;
    if (input_is_held(input, GAME_BUTTON_LEFT)) sSecret.x -= horizontal;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) sSecret.x += horizontal;

    /* Vertical collision masks are the last part still being decoded.  Keep
     * the Wii U vertical traversal at the event-sheet gravity unit (10 px)
     * so every MFA trigger remains reachable while page/warp parity is tested. */
    if (input_is_held(input, GAME_BUTTON_UP)) sSecret.y -= 10;
    if (input_is_held(input, GAME_BUTTON_DOWN)) sSecret.y += 10;

    sSecret.x = pc_mfa_clamp(sSecret.x, -128, 3400);
    sSecret.y = pc_mfa_clamp(sSecret.y, -220, 2340);
}

static void pc_mfa_v3_apply_zip_rules(void)
{
    if ((sSecret.frames % 3u) != 0u) return; /* MFA Every 50 ms. */

    if (sSecret.kind == SECRET_MINIGAME_STAGE01) {
        for (int i = 0; i < 8; ++i) {
            if (pc_mfa_v3_near(kPcMfaStageZipUp[i], 34)) {
                sSecret.y -= 10; /* exact event delta */
                return;
            }
        }
        for (int i = 0; i < 2; ++i) {
            if (pc_mfa_v3_near(kPcMfaStageZipRight[i], 34)) {
                sSecret.x += 20; /* exact event delta */
                return;
            }
        }
    } else if (sSecret.kind == SECRET_MINIGAME_SHADOW_BONNIE) {
        /* RWQ has one zip-up and one zip-right instance in the MFA. */
        static const PcMfaPoint rwq_zip_up = {1166, -184};
        static const PcMfaPoint rwq_zip_right = {3164, 466};
        if (pc_mfa_v3_near(rwq_zip_up, 34)) sSecret.y -= 10;
        if (pc_mfa_v3_near(rwq_zip_right, 34)) sSecret.x += 20;
    }
}

static void pc_mfa_v3_apply_rwq_blocks(void)
{
    if (sSecret.kind != SECRET_MINIGAME_SHADOW_BONNIE) return;
    if (sSecret.frames == sPcMfaV3.last_special_frame) return;

    /* Exact responses from MFA event groups 48-51. */
    if (pc_mfa_v3_inside_rect(kPcMfaShadowBlocks[0], 10)) {
        sSecret.x -= 30;
    } else if (pc_mfa_v3_inside_rect(kPcMfaShadowBlocks[1], 10)) {
        sSecret.x += 30;
    } else if (pc_mfa_v3_inside_rect(kPcMfaShadowBlocks[2], 10)) {
        sSecret.y -= 30;
    } else if (pc_mfa_v3_inside_rect(kPcMfaShadowBlocks[3], 10)) {
        sSecret.y += 30;
    } else {
        return;
    }
    sPcMfaV3.last_special_frame = sSecret.frames;
    pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.42f, false);
}

static void pc_mfa_v3_update_bb(Game *game, const InputState *input)
{
    pc_mfa_v3_move_player(input);
    pc_mfa_collect(game, kPcMfaBbCollect, 8, 34);
    if (sSecret.collected == 0xFFu && pc_mfa_v3_near(kPcMfaBbExit, 48))
        secret_complete(game);
}

static void pc_mfa_v3_update_mangle(Game *game, const InputState *input)
{
    pc_mfa_v3_move_player(input);
    pc_mfa_collect(game, kPcMfaMangleParts, 4, 42);
    if ((sSecret.collected & 0x0Fu) == 0x0Fu &&
        (pc_mfa_v3_near(kPcMfaMangleExit, 56) ||
         pc_mfa_v3_near(kPcMfaMangleAltExit, 56)))
        secret_complete(game);
}

static void pc_mfa_v3_update_chica(Game *game, const InputState *input)
{
    pc_mfa_v3_move_player(input);
    for (int i = 0; i < 4; ++i) {
        const uint8_t pickup = (uint8_t)(1u << i);
        if ((sSecret.collected & pickup) == 0u &&
            pc_mfa_v3_near(kPcMfaChicaCupcakes[i], 42)) {
            sSecret.collected |= pickup;
            ++sSecret.carried;
            content_play_select();
        }
        const uint8_t delivered = (uint8_t)(1u << (i + 4));
        if (sSecret.carried > 0 && (sSecret.collected & delivered) == 0u &&
            pc_mfa_v3_near(kPcMfaChicaChildren[i], 52)) {
            sSecret.collected |= delivered;
            --sSecret.carried;
            ++sSecret.delivered;
            audio_play(AUDIO_CUE_END, 0.42f, false);
        }
    }
    if (sSecret.delivered >= 4 &&
        (pc_mfa_v3_near(kPcMfaChicaExit, 58) ||
         pc_mfa_v3_near(kPcMfaV2ChicaExit2, 58)))
        secret_complete(game);
}

static void pc_mfa_v3_update_stage01(Game *game, const InputState *input)
{
    pc_mfa_v3_move_player(input);
    pc_mfa_v3_apply_zip_rules();
    pc_mfa_collect(game, kPcMfaStageGoalNodes, 3, 52);
    if ((sSecret.collected & 0x07u) == 0x07u &&
        pc_mfa_v3_near(kPcMfaStageCake, 60))
        secret_complete(game);
}

static void pc_mfa_v3_update_shadow(Game *game, const InputState *input)
{
    pc_mfa_v3_move_player(input);
    pc_mfa_v3_apply_zip_rules();
    pc_mfa_v3_apply_rwq_blocks();

    /* Keep MODE as a Wii U mapping for the original phase navigation while
     * the collision-mask extractor is completed; positions are real anchors. */
    if (input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) % 4;
        sSecret.x = kPcMfaShadowAnchors[sSecret.room].x;
        sSecret.y = kPcMfaShadowAnchors[sSecret.room].y;
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.60f, false);
    }
    const uint8_t bit = (uint8_t)(1u << sSecret.room);
    if ((sSecret.collected & bit) == 0u &&
        pc_mfa_v3_near(kPcMfaShadowAnchors[sSecret.room], 60)) {
        sSecret.collected |= bit;
        content_play_select();
    }
    if ((sSecret.collected & 0x0Fu) == 0x0Fu &&
        (pc_mfa_v3_near(kPcMfaShadowKid, 64) ||
         pc_mfa_v3_near(kPcMfaShadowCake, 64)))
        secret_complete(game);
}

static void pc_mfa_v3_update_happiest(Game *game, const InputState *input)
{
    pc_mfa_v3_move_player(input);
    for (int i = 0; i < 4; ++i) {
        const uint8_t bit = (uint8_t)(1u << i);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!pc_mfa_v3_near(kPcMfaHappiestChildren[i], 54)) continue;
        sSecret.collected |= bit;
        ++sSecret.delivered;
        audio_play(AUDIO_CUE_END, 0.42f, false);
    }
    if (sSecret.delivered >= 4 &&
        (pc_mfa_v3_near(kPcMfaHappiestTrigger, 64) ||
         pc_mfa_v3_near(kPcMfaHappiestCakes[3], 64)))
        secret_complete(game);
}

static void pc_mfa_v3_update_active(Game *game, const InputState *input)
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

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_v3_update_bb(game, input); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_v3_update_mangle(game, input); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_v3_update_chica(game, input); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_v3_update_stage01(game, input); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_v3_update_shadow(game, input); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_v3_update_happiest(game, input); break;
        default: break;
    }
    game->dirty = true;
}

static void pc_mfa_v3_exact_update_game(Game *game, const InputState *input)
{
    const bool before_active = sSecret.active;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const int before_room = sSecret.room;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;

    if (sSecret.active && !sSecret.good_ending) {
        pc_mfa_v3_update_active(game, input);
    } else {
        sPcMfaV3.active = false;
        pc_mfa_v2_exact_update_game(game, input);
    }

    pc_mfa_v2_audio_after_update(input, before_active, before_collected,
                                 before_delivered, before_room,
                                 before_follow_event);
}
