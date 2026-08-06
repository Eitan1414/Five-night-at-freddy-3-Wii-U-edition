static const MiniPcPlatform kMiniStagePlatforms[] = {
    {0, 414, 854, 26},
    {50, 350, 190, 14},
    {300, 320, 180, 14},
    {570, 285, 190, 14},
    {100, 235, 170, 14},
    {365, 205, 175, 14},
    {630, 160, 175, 14},
};

static const MiniPcPlatform kMiniStageSecretPlatforms[] = {
    {0, 414, 854, 26},
    {70, 340, 180, 14},
    {310, 270, 180, 14},
    {550, 200, 180, 14},
    {700, 125, 125, 14},
};

static const int kMiniStageChildren[3][2] = {
    {190, 320}, {440, 290}, {690, 255}
};

static void secret_update_stage01_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_STAGE01, 85, 365, 0);
    ++sMiniPc.room_frames;
    const MiniPcPlatform *platforms =
        sSecret.room == 0 ? kMiniStagePlatforms : kMiniStageSecretPlatforms;
    const int count = sSecret.room == 0
        ? mini_pc_count(kMiniStagePlatforms, sizeof(kMiniStagePlatforms))
        : mini_pc_count(kMiniStageSecretPlatforms,
                        sizeof(kMiniStageSecretPlatforms));

    mini_pc_move_horizontal(input, 4);
    mini_pc_apply_platform_physics(input, platforms, count, 19, 37, -12, 11);

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
            sSecret.x > 820 && sSecret.y < 190) {
            sSecret.room = 1;
            sSecret.x = 45;
            sSecret.y = 360;
            sMiniPc.velocity_y = 0;
            content_play_select();
            return;
        }
    } else if (secret_near(sSecret.x, sSecret.y, 770, 92, 46)) {
        secret_complete(game);
        return;
    }

    mini_pc_clamp_horizontal();
    mini_pc_clamp_top(37);
    if (sSecret.y > kMiniPcBottom + 90)
        mini_pc_begin(SECRET_MINIGAME_STAGE01, 85, 365, 0);
    game->dirty = true;
}

static void secret_draw_stage01_pc(void)
{
    if (sSecret.room == 0) {
        secret_draw_header("STAGE 01",
                           "FIND 3 CHILDREN  THEN USE THE HIGH RIGHT EXIT");
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
        minigame_draw_pc_texture_centered(&gMiniStageChild, 770, 92);
        minigame_draw_pc_texture_centered(&gMiniPartyCake, 770, 50);
    }

    const TextureRle *player = &gMiniStageRight;
    if (sMiniPc.facing_left) player = &gMiniStageLeft;
    minigame_draw_pc_texture_centered(player, sSecret.x, sSecret.y);
}

/* ------------------------------------------------------------------------- */
/* SHADOW BONNIE                                                             */
/* ------------------------------------------------------------------------- */

static const int kMiniShadowTargets[4][2] = {
    {710, 125}, {165, 125}, {700, 350}, {165, 350}
};

static void secret_update_shadow_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_SHADOW_BONNIE, 100, 130, 0);
    ++sMiniPc.room_frames;

    const int speed = 5;
    if (input_is_held(input, GAME_BUTTON_LEFT)) {
        sSecret.x -= speed;
        sMiniPc.facing_left = true;
    }
    if (input_is_held(input, GAME_BUTTON_RIGHT)) {
        sSecret.x += speed;
        sMiniPc.facing_left = false;
    }
    if (input_is_held(input, GAME_BUTTON_UP)) sSecret.y -= speed;
    if (input_is_held(input, GAME_BUTTON_DOWN)) sSecret.y += speed;

    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) % 5;
        sMiniPc.room_frames = 0;
        content_play_select();
    }

    if (sSecret.room < 4) {
        const uint8_t bit = (uint8_t) (1u << sSecret.room);
        if ((sSecret.collected & bit) == 0u &&
            secret_near(sSecret.x, sSecret.y,
                        kMiniShadowTargets[sSecret.room][0],
                        kMiniShadowTargets[sSecret.room][1], 36)) {
            sSecret.collected |= bit;
            content_play_select();
        }
    } else if ((sSecret.collected & 0x0Fu) == 0x0Fu &&
               secret_near(sSecret.x, sSecret.y, 730, 215, 46)) {
        secret_complete(game);
        return;
    }

    mini_pc_clamp_horizontal();
    if (sSecret.y < 78) sSecret.y = 78;
    if (sSecret.y > 420) sSecret.y = 420;
    game->dirty = true;
}

static void secret_draw_shadow_pc(void)
{
    static const uint32_t room_colours[5] = {
        GRAPHICS_RGB(70, 8, 75), GRAPHICS_RGB(122, 42, 127),
        GRAPHICS_RGB(45, 0, 51), GRAPHICS_RGB(178, 104, 183),
        GRAPHICS_RGB(55, 177, 182),
    };
    secret_draw_header("SHADOW BONNIE",
                       "A/R PHASE ROOMS  TOUCH 4 SHADOWS  FIND THE CHILD");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       room_colours[sSecret.room]);

    if (sSecret.room < 4) {
        for (int x = 55; x < 835; x += 110)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 90, 8, 310,
                               GRAPHICS_RGB(35, 0, 42));
        const uint8_t bit = (uint8_t) (1u << sSecret.room);
        if ((sSecret.collected & bit) == 0u)
            minigame_draw_pc_texture_centered(
                &gMiniShadowChildSad,
                kMiniShadowTargets[sSecret.room][0],
                kMiniShadowTargets[sSecret.room][1]);
    } else {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 40, 390, 774, 24,
                           GRAPHICS_RGB(83, 83, 83));
        minigame_draw_pc_texture_centered(&gMiniChildSad, 730, 215);
        minigame_draw_pc_texture_centered(&gMiniPartyCake, 730, 170);
    }

    minigame_draw_pc_texture_centered(
        sMiniPc.facing_left ? &gMiniShadowLeft : &gMiniShadowRight,
        sSecret.x, sSecret.y);
    char room[24];
    snprintf(room, sizeof(room), "PHASE %d/5", sSecret.room + 1);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 700, 22, 1, room,
                       GRAPHICS_RGB(230, 180, 240));
}

/* ------------------------------------------------------------------------- */
/* HAPPIEST DAY                                                              */
/* ------------------------------------------------------------------------- */

