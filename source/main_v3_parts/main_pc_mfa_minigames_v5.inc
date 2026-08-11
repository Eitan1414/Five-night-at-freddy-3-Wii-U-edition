/* Fifth MFA fidelity pass: collision/platform physics from fivenights3-94.mfa.
 *
 * The collision rectangles below are the original obstacle Backdrop instances
 * (ObstacleType == 1) with their real image-bank dimensions.  The six hidden
 * feeler objects are positioned every frame relative to the authoritative
 * player hitbox at the exact offsets used by the Clickteam event sheet:
 *
 *   feel right   +39,-3     feel left   -33,-3
 *   feel bottom   +3,+41    feel bottom2 +3,+60
 *   feel top      +5,-31    feel bottom3 +3,+30
 *
 * The original events move horizontally by 15 px (20 for Marion), start a
 * seven-step jump, move upward 20 px every 60 ms, apply gravity +10 px every
 * 100 ms, and resolve floor penetration upward by 5 px.  Wii U runs at 60 Hz,
 * so the timer events are represented by 4 and 6 frame ticks respectively. */

typedef struct PcMfaV5PhysicsState {
    bool active;
    SecretMinigameKind kind;
    int jump_ticks;
    bool jump_latch;
    bool grounded;
    bool moved_horizontal;
    uint32_t jump_clock;
    uint32_t gravity_clock;
} PcMfaV5PhysicsState;

static PcMfaV5PhysicsState sPcMfaV5;

static const PcMfaRect kPcMfaV5BbSolids[] = {
    {-2,-10,3054,30}, {0,-6,32,2316}, {3044,-2,32,2316},
    {106,132,762,30}, {839,134,32,562}, {700,294,164,30},
    {434,320,164,30}, {106,342,32,328}, {134,400,164,30},
    {346,488,164,30}, {610,554,164,30}, {106,668,762,30},
    {1828,1160,762,30}, {2180,1652,762,30}, {2910,1652,32,572},
    {2180,1662,32,328}, {2466,1828,164,30}, {2758,1904,164,30},
    {2536,1992,164,30}, {2334,2096,164,30}, {2180,2198,762,30},
    {0,2272,3054,30},
};

static const PcMfaRect kPcMfaV5MangleSolids[] = {
    {-2,-10,3054,30}, {0,-6,32,2316}, {3044,-2,32,2316},
    {106,132,2736,30}, {2838,340,32,358}, {106,342,32,328},
    {1392,342,164,30}, {1694,374,164,30}, {2386,398,164,30},
    {2678,412,164,30}, {1106,422,164,30}, {2096,456,164,30},
    {2346,550,164,30}, {1334,552,164,30}, {610,554,164,30},
    {106,668,2736,30}, {0,2272,3054,30},
};

static const PcMfaRect kPcMfaV5ChicaSolids[] = {
    {-2,-10,3054,30}, {0,-6,32,2316}, {3044,-2,32,2316},
    {106,132,2830,30}, {107,132,32,562}, {2905,132,32,562},
    {1882,280,164,30}, {1476,412,164,30}, {1632,412,164,30},
    {1206,560,164,30}, {106,670,1322,30}, {1660,670,1276,30},
    {1522,752,164,30}, {174,768,32,328}, {1070,792,32,322},
    {1685,818,1260,30}, {2913,824,32,562}, {1328,852,164,30},
    {1074,948,164,30}, {1300,1062,164,30}, {1070,1063,32,322},
    {176,1066,164,30}, {330,1066,164,30}, {332,1066,32,322},
    {1598,1156,164,30}, {1334,1256,164,30}, {2580,1276,164,30},
    {2250,1278,164,30}, {1669,1357,1276,30}, {904,1358,1276,30},
    {2014,1534,32,772}, {2180,1652,2830,30}, {2910,1652,32,572},
    {2180,1662,32,328}, {2466,1828,164,30}, {2758,1904,164,30},
    {2536,1992,164,30}, {2334,2096,164,30}, {2180,2198,2830,30},
    {0,2274,3054,30},
};

static const PcMfaRect kPcMfaV5StageSolids[] = {
    {1008,-748,32,2316}, {-2,-10,3054,30}, {0,-6,32,2316},
    {3044,-2,32,2316}, {2194,114,32,538}, {2194,114,762,30},
    {2927,116,32,562}, {1146,118,32,538}, {1146,118,762,30},
    {1879,120,32,562}, {106,132,32,538}, {106,132,762,30},
    {839,134,32,562}, {1152,564,358,92}, {134,580,358,92},
    {2194,650,762,30}, {1146,654,762,30}, {106,668,762,30},
    {1154,882,32,538}, {1154,882,762,30}, {2196,882,32,538},
    {2196,882,762,30}, {108,884,32,538}, {108,884,762,30},
    {1887,884,32,562}, {2929,884,32,562}, {841,886,32,562},
    {1160,1328,358,92}, {2202,1328,358,92}, {114,1330,358,92},
    {1154,1418,762,30}, {2196,1418,762,30}, {108,1420,762,30},
    {110,1600,32,538}, {110,1600,762,30}, {843,1602,32,562},
    {1150,1604,32,538}, {1150,1604,762,30}, {1883,1606,32,562},
    {2168,1609,32,538}, {2168,1609,762,30}, {2901,1611,32,562},
    {116,2046,358,92}, {1156,2050,358,92}, {2174,2055,358,92},
    {110,2136,762,30}, {1150,2140,762,30}, {2168,2145,762,30},
    {0,2272,3054,30},
};

static const PcMfaRect kPcMfaV5ShadowSolids[] = {
    {1132,112,766,32}, {2194,112,762,30}, {1866,114,32,562},
    {2927,114,32,562}, {1132,118,32,562}, {106,132,32,538},
    {106,132,762,30}, {839,134,32,562}, {2788,274,164,30},
    {2522,300,164,30}, {2194,322,32,328}, {2222,380,164,30},
    {1484,452,164,30}, {2434,468,164,30}, {1706,508,164,30},
    {2698,534,164,30}, {134,580,358,92}, {1132,648,766,32},
    {2194,648,762,30}, {106,668,762,30}, {1145,876,802,32},
    {103,878,802,32}, {1915,888,32,546}, {873,890,32,546},
    {1144,894,32,546}, {102,896,32,546}, {422,1136,164,30},
    {714,1150,164,30}, {132,1194,164,30}, {382,1288,164,30},
    {1145,1408,802,32}, {103,1410,802,32}, {0,2272,3054,30},
};

static const PcMfaRect kPcMfaV5HappiestSolids[] = {
    {-2,-10,3054,30}, {0,-6,32,2316}, {3044,-2,32,2316},
    {106,132,32,538}, {106,132,2846,30}, {2921,134,32,562},
    {106,668,2846,30}, {2180,1452,32,538}, {2180,1652,2846,30},
    {2910,1652,32,572}, {2180,2198,2846,30}, {0,2272,3054,30},
};

static const PcMfaRect *pc_mfa_v5_solids(int *count)
{
    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB:
            *count = (int)(sizeof(kPcMfaV5BbSolids) / sizeof(kPcMfaV5BbSolids[0]));
            return kPcMfaV5BbSolids;
        case SECRET_MINIGAME_MANGLE:
            *count = (int)(sizeof(kPcMfaV5MangleSolids) / sizeof(kPcMfaV5MangleSolids[0]));
            return kPcMfaV5MangleSolids;
        case SECRET_MINIGAME_CHICA:
            *count = (int)(sizeof(kPcMfaV5ChicaSolids) / sizeof(kPcMfaV5ChicaSolids[0]));
            return kPcMfaV5ChicaSolids;
        case SECRET_MINIGAME_STAGE01:
            *count = (int)(sizeof(kPcMfaV5StageSolids) / sizeof(kPcMfaV5StageSolids[0]));
            return kPcMfaV5StageSolids;
        case SECRET_MINIGAME_SHADOW_BONNIE:
            *count = (int)(sizeof(kPcMfaV5ShadowSolids) / sizeof(kPcMfaV5ShadowSolids[0]));
            return kPcMfaV5ShadowSolids;
        case SECRET_MINIGAME_HAPPIEST_DAY:
            *count = (int)(sizeof(kPcMfaV5HappiestSolids) / sizeof(kPcMfaV5HappiestSolids[0]));
            return kPcMfaV5HappiestSolids;
        default:
            *count = 0;
            return NULL;
    }
}

static bool pc_mfa_v5_point_in_solid(int x, int y)
{
    int count = 0;
    const PcMfaRect *solids = pc_mfa_v5_solids(&count);
    for (int i = 0; i < count; ++i) {
        const PcMfaRect r = solids[i];
        if (x >= r.x && x < r.x + r.w &&
            y >= r.y && y < r.y + r.h)
            return true;
    }
    return false;
}

static bool pc_mfa_v5_point_on_balloon(int x, int y)
{
    const PcMfaPoint *points = NULL;
    int count = 0;
    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB:
        case SECRET_MINIGAME_HAPPIEST_DAY:
            points = kPcMfaBbBalloons;
            count = 8;
            break;
        case SECRET_MINIGAME_MANGLE:
            points = kPcMfaMangleBalloonPath;
            count = 12;
            break;
        case SECRET_MINIGAME_CHICA:
            points = kPcMfaChicaBalloons;
            count = 2;
            break;
        default:
            break;
    }
    for (int i = 0; i < count; ++i) {
        const int dx = x - points[i].x;
        const int dy = y - points[i].y;
        if (dx >= -28 && dx <= 28 && dy >= -34 && dy <= 34)
            return true;
    }
    return false;
}

static bool pc_mfa_v5_sensor_solid(int ox, int oy)
{
    return pc_mfa_v5_point_in_solid(sSecret.x + ox, sSecret.y + oy);
}

static bool pc_mfa_v5_sensor_floor(int ox, int oy)
{
    const int x = sSecret.x + ox;
    const int y = sSecret.y + oy;
    return pc_mfa_v5_point_in_solid(x, y) || pc_mfa_v5_point_on_balloon(x, y);
}

static bool pc_mfa_v5_on_zip_up(void)
{
    if (sSecret.kind == SECRET_MINIGAME_STAGE01) {
        for (int i = 0; i < 8; ++i)
            if (pc_mfa_v3_near(kPcMfaStageZipUp[i], 34)) return true;
    } else if (sSecret.kind == SECRET_MINIGAME_SHADOW_BONNIE) {
        static const PcMfaPoint rwq_zip_up = {1166, -184};
        if (pc_mfa_v3_near(rwq_zip_up, 34)) return true;
    }
    return false;
}

static void pc_mfa_v5_reset_physics(void)
{
    memset(&sPcMfaV5, 0, sizeof(sPcMfaV5));
    sPcMfaV5.active = true;
    sPcMfaV5.kind = sSecret.kind;
    sPcMfaV5.grounded = pc_mfa_v5_sensor_floor(3, 60);
}

static bool pc_mfa_v5_apply_physics(const InputState *input)
{
    const bool was_grounded = sPcMfaV5.grounded;
    const int old_x = sSecret.x;
    const int horizontal =
        sSecret.kind == SECRET_MINIGAME_HAPPIEST_DAY ? 20 : 15;

    /* Exact A/D event moves guarded by feel-left / feel-right. */
    if (input_is_held(input, GAME_BUTTON_LEFT) &&
        !pc_mfa_v5_sensor_solid(-33, -3))
        sSecret.x -= horizontal;
    if (input_is_held(input, GAME_BUTTON_RIGHT) &&
        !pc_mfa_v5_sensor_solid(39, -3))
        sSecret.x += horizontal;
    sPcMfaV5.moved_horizontal = sSecret.x != old_x;

    /* W released -> clear the one-jump latch. */
    if (!input_is_held(input, GAME_BUTTON_UP))
        sPcMfaV5.jump_latch = false;

    /* W + floor/balloon + jump counter 0 -> seven-step jump. */
    bool jump_started = false;
    if (input_is_held(input, GAME_BUTTON_UP) &&
        !sPcMfaV5.jump_latch && sPcMfaV5.jump_ticks == 0 &&
        pc_mfa_v5_sensor_floor(3, 60)) {
        sPcMfaV5.jump_ticks = 7;
        sPcMfaV5.jump_latch = true;
        sPcMfaV5.jump_clock = 0u;
        jump_started = true;
    }

    /* Hitting the ceiling cancels the remaining jump impulses. */
    if (pc_mfa_v5_sensor_solid(5, -31)) {
        sPcMfaV5.jump_ticks = 0;
        sPcMfaV5.jump_clock = 0u;
    }

    if (sPcMfaV5.jump_ticks > 0) {
        ++sPcMfaV5.jump_clock;
        if (sPcMfaV5.jump_clock >= 4u) { /* original Every 60 ms */
            sPcMfaV5.jump_clock = 0u;
            --sPcMfaV5.jump_ticks;
            sSecret.y -= 20;
            if (pc_mfa_v5_sensor_solid(5, -31))
                sPcMfaV5.jump_ticks = 0;
        }
    } else {
        sPcMfaV5.jump_clock = 0u;
        const bool floor1 = pc_mfa_v5_sensor_floor(3, 41);
        const bool floor2 = pc_mfa_v5_sensor_floor(3, 60);
        if (!floor1 && !floor2 && !pc_mfa_v5_on_zip_up()) {
            ++sPcMfaV5.gravity_clock;
            if (sPcMfaV5.gravity_clock >= 6u) { /* original Every 100 ms */
                sPcMfaV5.gravity_clock = 0u;
                sSecret.y += 10;
            }
        } else {
            sPcMfaV5.gravity_clock = 0u;
        }
    }

    /* Original feel-bottom3 penetration correction: Y(hitbox) - 5. */
    if (pc_mfa_v5_sensor_floor(3, 30))
        sSecret.y -= 5;

    sSecret.x = pc_mfa_clamp(sSecret.x, -256, 3400);
    sSecret.y = pc_mfa_clamp(sSecret.y, -800, 2340);
    sPcMfaV5.grounded =
        pc_mfa_v5_sensor_floor(3, 60) || pc_mfa_v5_sensor_floor(3, 41);

    if (jump_started)
        pc_minigame_sfx_play(pc_mfa_v2_jump_sound(), 0.50f, false);
    if (!was_grounded && sPcMfaV5.grounded && sPcMfaV5.jump_ticks == 0)
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LAND, 0.42f, false);

    return jump_started;
}

static void pc_mfa_v5_update_run_sound(void)
{
    if (sPcMfaV5.moved_horizontal && !sPcMfaRunSound) {
        pc_minigame_sfx_play(PC_MINIGAME_SFX_RUN, 0.34f, true);
        sPcMfaRunSound = true;
    } else if (!sPcMfaV5.moved_horizontal && sPcMfaRunSound) {
        pc_minigame_sfx_stop(PC_MINIGAME_SFX_RUN);
        sPcMfaRunSound = false;
    }
}

static void pc_mfa_v5_update_bb(Game *game, const InputState *input)
{
    (void) pc_mfa_v5_apply_physics(input);
    pc_mfa_collect(game, kPcMfaBbCollect, 8, 34);
    if (sSecret.collected == 0xFFu && pc_mfa_v3_near(kPcMfaBbExit, 48))
        secret_complete(game);
}

static void pc_mfa_v5_update_mangle(Game *game, const InputState *input)
{
    (void) pc_mfa_v5_apply_physics(input);
    pc_mfa_collect(game, kPcMfaMangleParts, 4, 42);
    if ((sSecret.collected & 0x0Fu) == 0x0Fu &&
        (pc_mfa_v3_near(kPcMfaMangleExit, 56) ||
         pc_mfa_v3_near(kPcMfaMangleAltExit, 56)))
        secret_complete(game);
}

static void pc_mfa_v5_update_chica(Game *game, const InputState *input)
{
    (void) pc_mfa_v5_apply_physics(input);
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

static void pc_mfa_v5_update_stage01(Game *game, const InputState *input)
{
    (void) pc_mfa_v5_apply_physics(input);
    pc_mfa_v3_apply_zip_rules();
    pc_mfa_v4_apply_stage_left_zip();
    pc_mfa_collect(game, kPcMfaStageGoalNodes, 3, 52);
    if ((sSecret.collected & 0x07u) == 0x07u &&
        pc_mfa_v3_near(kPcMfaStageCake, 60))
        secret_complete(game);
}

static void pc_mfa_v5_update_shadow(Game *game, const InputState *input)
{
    (void) pc_mfa_v5_apply_physics(input);

    if ((sSecret.frames % 3u) == 0u) {
        if (pc_mfa_v3_near(kPcMfaV4ShadowLeftZip, 34))
            sSecret.x -= 10;
        static const PcMfaPoint zip_up = {1166, -184};
        static const PcMfaPoint zip_right = {3164, 466};
        if (pc_mfa_v3_near(zip_up, 34)) sSecret.y -= 10;
        if (pc_mfa_v3_near(zip_right, 34)) sSecret.x += 20;
    }
    pc_mfa_v3_apply_rwq_blocks();

    /* Original PC S key: only cycle the watch-screen view, never the player. */
    if (input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) % 5;
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.60f, false);
        game->dirty = true;
    }

    if (pc_mfa_v3_near(kPcMfaShadowKid, 64) ||
        pc_mfa_v3_near(kPcMfaShadowCake, 64))
        secret_complete(game);
}

static void pc_mfa_v5_update_happiest(Game *game, const InputState *input)
{
    (void) pc_mfa_v5_apply_physics(input);
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

static void pc_mfa_v5_update_active(Game *game, const InputState *input)
{
    if (!sPcMfaV3.active || sPcMfaV3.kind != sSecret.kind) {
        memset(&sPcMfaV3, 0, sizeof(sPcMfaV3));
        sPcMfaV3.active = true;
        sPcMfaV3.kind = sSecret.kind;
        sSecret.x = kPcMfaV3HitboxStart[sSecret.kind].x;
        sSecret.y = kPcMfaV3HitboxStart[sSecret.kind].y;
        sSecret.room = 0;
        sSecret.frames = 0u;
        pc_mfa_v5_reset_physics();
        game->dirty = true;
    } else if (!sPcMfaV5.active || sPcMfaV5.kind != sSecret.kind) {
        pc_mfa_v5_reset_physics();
    }

    ++sSecret.frames;
    ++game->screen_frames;
    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending) {
        secret_return_to_menu(game);
        sPcMfaV3.active = false;
        sPcMfaV5.active = false;
        if (sPcMfaRunSound) {
            pc_minigame_sfx_stop(PC_MINIGAME_SFX_RUN);
            sPcMfaRunSound = false;
        }
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
            sPcMfaV3.active = false;
            sPcMfaV5.active = false;
        }
        return;
    }

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_v5_update_bb(game, input); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_v5_update_mangle(game, input); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_v5_update_chica(game, input); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_v5_update_stage01(game, input); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_v5_update_shadow(game, input); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_v5_update_happiest(game, input); break;
        default: break;
    }
    pc_mfa_v5_update_run_sound();
    game->dirty = true;
}

static void pc_mfa_v5_audio_after_update(bool before_active,
                                         uint8_t before_collected,
                                         int before_delivered,
                                         uint32_t before_follow_event)
{
    if (sSecret.active && !sSecret.completed) {
        if (sSecret.delivered > before_delivered)
            pc_minigame_sfx_play(PC_MINIGAME_SFX_GET2, 0.58f, false);
        else if (sSecret.collected != before_collected)
            pc_minigame_sfx_play(PC_MINIGAME_SFX_GET, 0.56f, false);
    } else if (sPcMfaRunSound) {
        pc_minigame_sfx_stop(PC_MINIGAME_SFX_RUN);
        sPcMfaRunSound = false;
    }

    if (!before_active && sSecret.active)
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.36f, false);
    if (before_follow_event == 0u && sFinishing.minigame_event_frames > 0u)
        pc_minigame_sfx_play(PC_MINIGAME_SFX_INSUIT, 0.54f, false);
}

static void pc_mfa_v5_exact_update_game(Game *game, const InputState *input)
{
    const bool before_active = sSecret.active;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;

    if (sSecret.active && !sSecret.good_ending) {
        pc_mfa_v5_update_active(game, input);
    } else {
        sPcMfaV3.active = false;
        sPcMfaV5.active = false;
        pc_mfa_v2_exact_update_game(game, input);
    }

    pc_mfa_v5_audio_after_update(before_active, before_collected,
                                 before_delivered, before_follow_event);
}
