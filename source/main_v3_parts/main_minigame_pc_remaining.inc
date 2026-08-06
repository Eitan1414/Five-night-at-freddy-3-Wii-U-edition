/* ------------------------------------------------------------------------- */
/* STAGE 01                                                                  */
/* ------------------------------------------------------------------------- */

static const MiniPcPlatform kMiniStagePlatforms[] = {
    {0, 414, 854, 26},
    {42, 350, 180, 16},
    {260, 305, 170, 16},
    {475, 260, 180, 16},
    {665, 205, 165, 16},
    {120, 205, 155, 16},
    {350, 155, 170, 16},
};

static const MiniPcPlatform kMiniStageSecretPlatforms[] = {
    {0, 414, 854, 26},
    {80, 350, 160, 14},
    {300, 285, 160, 14},
    {520, 220, 160, 14},
    {700, 145, 130, 14},
};

static const int kMiniStageChildren[3][2] = {
    {170, 320}, {430, 274}, {730, 174}
};

static void secret_update_stage01_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_STAGE01, 76, 388, 0);
    ++sMiniPc.room_frames;

    const MiniPcPlatform *platforms =
        sSecret.room == 0 ? kMiniStagePlatforms : kMiniStageSecretPlatforms;
    const int count = sSecret.room == 0
        ? mini_pc_count(kMiniStagePlatforms, sizeof(kMiniStagePlatforms))
        : mini_pc_count(kMiniStageSecretPlatforms,
                        sizeof(kMiniStageSecretPlatforms));

    mini_pc_move_horizontal(input, 4);
    mini_pc_apply_platform_physics(input, platforms, count, 25, 34, -12, 11);

    if (sSecret.room == 0) {
        for (int index = 0; index < 3; ++index) {
            const uint8_t bit = (uint8_t) (1u << index);
            if ((sSecret.collected & bit) != 0u) continue;
            if (!secret_near(sSecret.x, sSecret.y,
                             kMiniStageChildren[index][0],
                             kMiniStageChildren[index][1], 38)) continue;
            sSecret.collected |= bit;
            content_play_select();
        }

        if ((sSecret.collected & 0x07u) == 0x07u &&
            sSecret.x > 820 && sSecret.y < 185) {
            sSecret.room = 1;
            sSecret.x = 40;
            sSecret.y = 390;
            sMiniPc.velocity_y = 0;
            sMiniPc.grounded = false;
            content_play_select();
            return;
        }
    } else if (secret_near(sSecret.x, sSecret.y, 770, 105, 45)) {
        secret_complete(game);
        return;
    }

    mini_pc_clamp_horizontal();
    mini_pc_clamp_top(34);
    if (sSecret.y > kMiniPcBottom + 80)
        mini_pc_begin(SECRET_MINIGAME_STAGE01, 76, 388, 0);
    game->dirty = true;
}

static void secret_draw_stage01_pc(void)
{
    if (sSecret.room == 0) {
        secret_draw_header("STAGE 01",
                           "FIND 3 CHILDREN  THEN TAKE THE HIGH RIGHT EXIT");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniStagePlatforms,
            mini_pc_count(kMiniStagePlatforms, sizeof(kMiniStagePlatforms)),
            GRAPHICS_RGB(139, 62, 42));

        for (int index = 0; index < 3; ++index) {
            if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
                minigame_draw_pc_texture_centered(
                    &gMiniStageChild,
                    kMiniStageChildren[index][0],
                    kMiniStageChildren[index][1]);
        }
    } else {
        secret_draw_header("STAGE 01", "TAKE THE CAKE TO THE CHILD");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniStageSecretPlatforms,
            mini_pc_count(kMiniStageSecretPlatforms,
                          sizeof(kMiniStageSecretPlatforms)),
            GRAPHICS_RGB(139, 62, 42));
        minigame_draw_pc_texture_centered(&gMiniChildSad, 770, 105);
        minigame_draw_pc_texture_centered(&gMiniPartyCake, 770, 60);
    }

    const TextureRle *player =
        sMiniPc.facing_left ? &gMiniStageLeft : &gMiniStageRight;
    if (sMiniPc.grounded && sMiniPc.room_frames < 8)
        player = &gMiniStageFront;
    minigame_draw_pc_texture_centered(player, sSecret.x, sSecret.y);
}

/* ------------------------------------------------------------------------- */
/* SHADOW BONNIE                                                             */
/* ------------------------------------------------------------------------- */

static const int kMiniShadowTargets[4][2] = {
    {724, 112}, {166, 116}, {710, 358}, {168, 358}
};

static const uint32_t kMiniShadowRoomColours[4] = {
    GRAPHICS_RGB(59, 0, 59),
    GRAPHICS_RGB(93, 35, 102),
    GRAPHICS_RGB(31, 0, 49),
    GRAPHICS_RGB(120, 55, 126),
};

static void mini_shadow_move(const InputState *input)
{
    if (input_is_held(input, GAME_BUTTON_LEFT)) {
        sSecret.x -= 5;
        sMiniPc.facing_left = true;
    }
    if (input_is_held(input, GAME_BUTTON_RIGHT)) {
        sSecret.x += 5;
        sMiniPc.facing_left = false;
    }
    if (input_is_held(input, GAME_BUTTON_UP)) sSecret.y -= 5;
    if (input_is_held(input, GAME_BUTTON_DOWN)) sSecret.y += 5;
    if (sSecret.x < 25) sSecret.x = 25;
    if (sSecret.x > 829) sSecret.x = 829;
    if (sSecret.y < 82) sSecret.y = 82;
    if (sSecret.y > 414) sSecret.y = 414;
}

static void secret_update_shadow_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_SHADOW_BONNIE, 105, 120, 0);
    ++sMiniPc.room_frames;
    mini_shadow_move(input);

    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) & 3;
        sSecret.x = (sSecret.room == 1 || sSecret.room == 3) ? 730 : 105;
        sSecret.y = sSecret.room >= 2 ? 350 : 120;
        content_play_select();
    }

    const uint8_t bit = (uint8_t) (1u << sSecret.room);
    if ((sSecret.collected & bit) == 0u &&
        secret_near(sSecret.x, sSecret.y,
                    kMiniShadowTargets[sSecret.room][0],
                    kMiniShadowTargets[sSecret.room][1], 38)) {
        sSecret.collected |= bit;
        content_play_select();
    }

    if ((sSecret.collected & 0x0Fu) == 0x0Fu) {
        secret_complete(game);
        return;
    }
    game->dirty = true;
}

static void secret_draw_shadow_pc(void)
{
    secret_draw_header("SHADOW BONNIE",
                       "A OR R: PHASE ROOMS  REACH EACH HIDDEN CHILD");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       kMiniShadowRoomColours[sSecret.room]);

    for (int y = 100; y < 430; y += 75) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 25, y, 804, 18,
                           GRAPHICS_RGB(0, 0, 0));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 25, y + 18, 804, 7,
                           COLOUR_WHITE);
    }

    const uint8_t bit = (uint8_t) (1u << sSecret.room);
    minigame_draw_pc_texture_centered(
        (sSecret.collected & bit) != 0u
            ? &gMiniShadowChildHappy : &gMiniShadowChildSad,
        kMiniShadowTargets[sSecret.room][0],
        kMiniShadowTargets[sSecret.room][1]);

    minigame_draw_pc_texture_centered(
        sMiniPc.facing_left ? &gMiniShadowLeft : &gMiniShadowRight,
        sSecret.x, sSecret.y);

    char room_text[32];
    snprintf(room_text, sizeof(room_text), "ROOM %d/4  FOUND %d/4",
             sSecret.room + 1,
             ((sSecret.collected & 1u) != 0u) +
             ((sSecret.collected & 2u) != 0u) +
             ((sSecret.collected & 4u) != 0u) +
             ((sSecret.collected & 8u) != 0u));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 600, 22, 1, room_text,
                       GRAPHICS_RGB(225, 160, 240));
}

/* ------------------------------------------------------------------------- */
/* HAPPIEST DAY                                                              */
/* ------------------------------------------------------------------------- */

static const int kMiniPartyChildren[5][2] = {
    {210, 352}, {335, 352}, {460, 352}, {585, 352}, {710, 352}
};

static void secret_update_happiest_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_HAPPIEST_DAY, 80, 350, 0);
    ++sMiniPc.room_frames;

    if (input_is_held(input, GAME_BUTTON_LEFT)) {
        sSecret.x -= 4;
        sMiniPc.facing_left = true;
    }
    if (input_is_held(input, GAME_BUTTON_RIGHT)) {
        sSecret.x += 4;
        sMiniPc.facing_left = false;
    }
    if (sSecret.x < 55) sSecret.x = 55;
    if (sSecret.x > 815) sSecret.x = 815;
    sSecret.y = 350;

    for (int index = 0; index < 5; ++index) {
        const uint8_t bit = (uint8_t) (1u << index);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!secret_near(sSecret.x, sSecret.y,
                         kMiniPartyChildren[index][0],
                         kMiniPartyChildren[index][1], 34)) continue;
        sSecret.collected |= bit;
        ++sSecret.delivered;
        audio_play(AUDIO_CUE_END, 0.42f, false);
    }

    if (sSecret.delivered >= 5 &&
        secret_near(sSecret.x, sSecret.y, 790, 285, 52)) {
        secret_complete(game);
        return;
    }
    game->dirty = true;
}

static void secret_draw_happiest_pc(void)
{
    static const TextureRle *const children[5] = {
        &gMiniPartyChild1, &gMiniPartyChild2, &gMiniPartyChild3,
        &gMiniPartyChild4, &gMiniPartyChild5,
    };
    static const TextureRle *const balloons[5] = {
        &gMiniBalloonPurple, &gMiniBalloonGreen, &gMiniBalloonOrange,
        &gMiniBalloonRed, &gMiniBalloonPurple,
    };

    secret_draw_header("HAPPIEST DAY",
                       "VISIT EVERY CHILD  THEN JOIN THE CAKE TABLE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       GRAPHICS_RGB(55, 177, 182));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 40, 390, 780, 28,
                       GRAPHICS_RGB(31, 31, 31));

    minigame_draw_pc_texture_centered(&gMiniExitDoor, 65, 334);
    for (int index = 0; index < 5; ++index) {
        minigame_draw_pc_texture_centered(children[index],
                                         kMiniPartyChildren[index][0],
                                         kMiniPartyChildren[index][1]);
        if ((sSecret.collected & (uint8_t) (1u << index)) != 0u)
            minigame_draw_pc_texture_centered(
                balloons[index], kMiniPartyChildren[index][0], 270);
    }

    minigame_draw_pc_texture_centered(&gMiniPartyCake, 790, 285);
    minigame_draw_pc_texture_centered(
        sMiniPc.facing_left ? &gMiniPuppetLeft : &gMiniPuppetRight,
        sSecret.x, sSecret.y);

    char progress[24];
    snprintf(progress, sizeof(progress), "CHILDREN %d/5", sSecret.delivered);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 680, 22, 1, progress,
                       GRAPHICS_RGB(255, 192, 203));
}

/* ------------------------------------------------------------------------- */
/* SHARED PC DISPATCH                                                        */
/* ------------------------------------------------------------------------- */

static void secret_update_active_pc(Game *game, const InputState *input)
{
    minigame_pc_extended_assets_init();
    ++sSecret.frames;
    ++game->screen_frames;

    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending) {
        sMiniPc.initialized = false;
        secret_return_to_menu(game);
        return;
    }

    if (sSecret.completed) {
        ++sSecret.completed_frames;
        game->dirty = true;
        if (sSecret.completed_frames >= 150u &&
            (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
             input_was_pressed(input, GAME_BUTTON_START) ||
             input_was_pressed(input, GAME_BUTTON_BACK))) {
            sMiniPc.initialized = false;
            secret_return_to_menu(game);
        }
        return;
    }

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB:
            secret_update_bb_pc(game, input);
            break;
        case SECRET_MINIGAME_MANGLE:
            secret_update_mangle_pc(game, input);
            break;
        case SECRET_MINIGAME_CHICA:
            secret_update_chica_pc(game, input);
            break;
        case SECRET_MINIGAME_STAGE01:
            secret_update_stage01_pc(game, input);
            break;
        case SECRET_MINIGAME_SHADOW_BONNIE:
            secret_update_shadow_pc(game, input);
            break;
        case SECRET_MINIGAME_HAPPIEST_DAY:
            secret_update_happiest_pc(game, input);
            break;
        default:
            break;
    }
    game->dirty = true;
}
