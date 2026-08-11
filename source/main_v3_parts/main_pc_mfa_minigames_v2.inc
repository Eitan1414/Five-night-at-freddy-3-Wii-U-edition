/* Second MFA fidelity pass.
 *
 * Every coordinate in this file is read directly from an object instance in
 * fivenights3-94.mfa.  This layer deliberately keeps unknown Clickteam event
 * semantics out of the renderer: invisible collision objects stay invisible,
 * while visible actors are placed at their original project coordinates. */

static const PcMfaPoint kPcMfaV2MangleKid = {2741, 586};
static const PcMfaPoint kPcMfaV2MangleGetThis[2] = {
    {3367, 1459}, {413, 1301},
};

static const PcMfaPoint kPcMfaV2ChicaExtraCollect[3] = {
    {874, 61},   /* collect 3 */
    {479, 521},  /* collect 4 */
    {783, -70},  /* collect 2 */
};
static const PcMfaPoint kPcMfaV2ChicaExit2 = {128, 2206};
static const PcMfaPoint kPcMfaV2ChicaGetThis2 = {402, 984};

static const PcMfaPoint kPcMfaV2StageChildren[23] = {
    {556, 666}, {662, 668}, {768, 668},
    {558, 1418}, {664, 1420}, {770, 1420},
    {1604, 1416}, {1710, 1418}, {1816, 1418},
    {1596, 652}, {1702, 654}, {1808, 654},
    {2646, 1416}, {2858, 1418},
    {560, 2134}, {666, 2136}, {772, 2136},
    {1600, 2138}, {1706, 2140}, {1812, 2140},
    {2618, 2143}, {2724, 2145}, {2830, 2145},
};

static const PcMfaPoint kPcMfaV2ShadowCollect[10] = {
    {2734, 462}, {2830, 464}, {2476, 396}, {2276, 312},
    {2562, 234}, {2636, 232}, {2874, 210},
    {1388, 541}, {1780, 470}, {731, -182},
};
static const PcMfaPoint kPcMfaV2ShadowKid = {777, 1324};
static const PcMfaPoint kPcMfaV2ShadowView = {504, -124};

static const PcMfaPoint kPcMfaV2HappiestCollect3[6] = {
    {2132, 324}, {2212, 376}, {2354, 288},
    {2436, 338}, {2820, 364}, {2554, 296},
};
static const PcMfaPoint kPcMfaV2HappiestCollect4[10] = {
    {1766, 296}, {1638, 336}, {1526, 262}, {1122, 302}, {1250, 330},
    {226, 338}, {312, 250}, {444, 362}, {822, 326}, {924, 276},
};
static const PcMfaPoint kPcMfaV2HappiestTrigger = {2559, 524};

static bool sPcMfaRunSound = false;
static unsigned sPcMfaJumpIndex = 0u;

static bool pc_mfa_v2_is_moving(const InputState *input)
{
    return input_is_held(input, GAME_BUTTON_LEFT) ||
           input_is_held(input, GAME_BUTTON_RIGHT) ||
           input_is_held(input, GAME_BUTTON_UP) ||
           input_is_held(input, GAME_BUTTON_DOWN);
}

static PcMinigameSfx pc_mfa_v2_jump_sound(void)
{
    static const PcMinigameSfx jumps[4] = {
        PC_MINIGAME_SFX_JUMP1, PC_MINIGAME_SFX_JUMP2,
        PC_MINIGAME_SFX_JUMP3, PC_MINIGAME_SFX_JUMP4,
    };
    const PcMinigameSfx result = jumps[sPcMfaJumpIndex & 3u];
    ++sPcMfaJumpIndex;
    return result;
}

static void pc_mfa_v2_draw_stage01(void)
{
    int cx, cy;
    pc_mfa_begin_scene(&cx, &cy, GRAPHICS_RGB(19, 9, 27));

    /* The eight Active 8 instances are room anchors.  The twenty-three
     * Active 9 instances are the actual visible child rows in the MFA. */
    for (int i = 0; i < 8; ++i) {
        const PcMfaPoint p = kPcMfaStageNodes[i];
        pc_mfa_draw_platform(p.x - 180, p.y + 96, 360, 34, cx, cy,
                             GRAPHICS_RGB(107, 47, 119));
    }
    for (int i = 0; i < 23; ++i)
        pc_mfa_draw_sprite(&gPcSecretStageChildTexture,
                           kPcMfaV2StageChildren[i], cx, cy, 82, 90);

    for (int i = 0; i < 8; ++i)
        pc_mfa_draw_world_marker(kPcMfaStageZipUp[i], cx, cy,
                                 GRAPHICS_RGB(53, 84, 126));
    for (int i = 0; i < 2; ++i)
        pc_mfa_draw_world_marker(kPcMfaStageZipRight[i], cx, cy,
                                 GRAPHICS_RGB(126, 84, 53));
    pc_mfa_draw_cake(kPcMfaStageCake, cx, cy, true);
    pc_mfa_draw_player(&gPcSecretStagePlayerTexture, cx, cy, 90, 116);
}

static void pc_mfa_v2_overlay_mangle(int cx, int cy)
{
    /* 'kid' is a distinct event object at this exact MFA location. */
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV2MangleKid, cx, cy, 78, 88);
}

static void pc_mfa_v2_overlay_shadow(int cx, int cy)
{
    pc_mfa_draw_sprite(&gPcSecretChicaChildBlueTexture,
                       kPcMfaV2ShadowKid, cx, cy, 78, 88);
}

static bool pc_mfa_v2_secret_render_override(Game *game)
{
    if (!sSecret.active || sSecret.good_ending) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    int cx = 0;
    int cy = 0;
    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB:
            pc_mfa_draw_bb();
            break;
        case SECRET_MINIGAME_MANGLE:
            pc_mfa_draw_mangle();
            pc_mfa_camera(&cx, &cy);
            pc_mfa_v2_overlay_mangle(cx, cy);
            break;
        case SECRET_MINIGAME_CHICA:
            pc_mfa_draw_chica();
            break;
        case SECRET_MINIGAME_STAGE01:
            pc_mfa_v2_draw_stage01();
            break;
        case SECRET_MINIGAME_SHADOW_BONNIE:
            pc_mfa_draw_shadow();
            pc_mfa_camera(&cx, &cy);
            pc_mfa_v2_overlay_shadow(cx, cy);
            break;
        case SECRET_MINIGAME_HAPPIEST_DAY:
            pc_mfa_draw_happiest();
            break;
        default:
            return false;
    }

    /* No coordinate HUD or page-seam overlay in the release presentation. */
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

static void pc_mfa_v2_audio_after_update(const InputState *input,
                                         bool before_active,
                                         uint8_t before_collected,
                                         int before_delivered,
                                         int before_room,
                                         uint32_t before_follow_event)
{
    if (sSecret.active && !sSecret.completed) {
        const bool moving = pc_mfa_v2_is_moving(input);
        if (moving && !sPcMfaRunSound) {
            pc_minigame_sfx_play(PC_MINIGAME_SFX_RUN, 0.34f, true);
            sPcMfaRunSound = true;
        } else if (!moving && sPcMfaRunSound) {
            pc_minigame_sfx_stop(PC_MINIGAME_SFX_RUN);
            sPcMfaRunSound = false;
        }

        if (input_was_pressed(input, GAME_BUTTON_UP))
            pc_minigame_sfx_play(pc_mfa_v2_jump_sound(), 0.50f, false);
        if (input_was_pressed(input, GAME_BUTTON_DOWN))
            pc_minigame_sfx_play(PC_MINIGAME_SFX_LAND, 0.42f, false);

        if (sSecret.delivered > before_delivered) {
            pc_minigame_sfx_play(PC_MINIGAME_SFX_GET2, 0.58f, false);
        } else if (sSecret.collected != before_collected) {
            pc_minigame_sfx_play(PC_MINIGAME_SFX_GET, 0.56f, false);
        }

        if (sSecret.kind == SECRET_MINIGAME_SHADOW_BONNIE &&
            sSecret.room != before_room) {
            pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.60f, false);
        }
    } else if (sPcMfaRunSound) {
        pc_minigame_sfx_stop(PC_MINIGAME_SFX_RUN);
        sPcMfaRunSound = false;
    }

    if (!before_active && sSecret.active)
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LONG_GLITCH, 0.36f, false);

    if (before_follow_event == 0u && sFinishing.minigame_event_frames > 0u)
        pc_minigame_sfx_play(PC_MINIGAME_SFX_INSUIT, 0.54f, false);
}

static void pc_mfa_v2_exact_update_game(Game *game, const InputState *input)
{
    const bool before_active = sSecret.active;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const int before_room = sSecret.room;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;

    pc_mfa_exact_update_game(game, input);

    /* Chica's second exit is a real MFA object, not an estimated shortcut. */
    if (sSecret.active && !sSecret.completed &&
        sSecret.kind == SECRET_MINIGAME_CHICA && sSecret.delivered >= 4 &&
        secret_near(sSecret.x, sSecret.y,
                    kPcMfaV2ChicaExit2.x, kPcMfaV2ChicaExit2.y, 58)) {
        secret_complete(game);
    }

    pc_mfa_v2_audio_after_update(input, before_active, before_collected,
                                 before_delivered, before_room,
                                 before_follow_event);
}
