static const MiniPcPlatform kMiniManglePlatforms[] = {
    {0, 414, 854, 26},
    {30, 350, 220, 14},
    {300, 330, 190, 14},
    {560, 350, 250, 14},
    {75, 265, 175, 14},
    {330, 245, 180, 14},
    {600, 255, 170, 14},
    {125, 175, 190, 14},
    {405, 165, 180, 14},
    {660, 145, 165, 14},
};

static const MiniPcPlatform kMiniMangleSecretPlatforms[] = {
    {0, 414, 854, 26},
    {80, 350, 170, 14},
    {310, 280, 180, 14},
    {560, 210, 180, 14},
    {700, 135, 130, 14},
};

static const int kMiniMangleParts[4][2] = {
    {165, 320}, {395, 214}, {645, 224}, {744, 116}
};

static void secret_update_mangle_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_MANGLE, 72, 390, 0);
    ++sMiniPc.room_frames;
    const MiniPcPlatform *platforms =
        sSecret.room == 0 ? kMiniManglePlatforms
                          : kMiniMangleSecretPlatforms;
    const int count = sSecret.room == 0
        ? mini_pc_count(kMiniManglePlatforms, sizeof(kMiniManglePlatforms))
        : mini_pc_count(kMiniMangleSecretPlatforms,
                        sizeof(kMiniMangleSecretPlatforms));

    mini_pc_move_horizontal(input, 4);
    mini_pc_apply_platform_physics(input, platforms, count, 10, 10, -10, 10);

    if (sSecret.room == 0) {
        for (int index = 0; index < 4; ++index) {
            const uint8_t bit = (uint8_t) (1u << index);
            if ((sSecret.collected & bit) != 0u) continue;
            if (!secret_near(sSecret.x, sSecret.y,
                             kMiniMangleParts[index][0],
                             kMiniMangleParts[index][1], 25)) continue;
            sSecret.collected |= bit;
            content_play_select();
        }
        const bool all = (sSecret.collected & 0x0Fu) == 0x0Fu;
        if (all && sSecret.x > 820 && sSecret.y < 170) {
            sSecret.room = 1;
            sSecret.x = 50;
            sSecret.y = 390;
            sMiniPc.velocity_y = 0;
            sMiniPc.grounded = false;
            content_play_select();
            return;
        }
    } else if (secret_near(sSecret.x, sSecret.y, 778, 92, 40)) {
        secret_complete(game);
        return;
    }

    mini_pc_clamp_horizontal();
    mini_pc_clamp_top(10);
    if (sSecret.y > kMiniPcBottom + 60)
        mini_pc_begin(SECRET_MINIGAME_MANGLE, 72, 390, 0);
    game->dirty = true;
}

static void secret_draw_mangle_pc(void)
{
    static const TextureRle *const parts[4] = {
        &gMiniManglePart1, &gMiniManglePart2,
        &gMiniManglePart3, &gMiniManglePart4,
    };
    if (sSecret.room == 0) {
        secret_draw_header("MANGLES QUEST",
                           "FIND 4 PARTS  THEN USE THE HIGH RIGHT EXIT");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniManglePlatforms,
            mini_pc_count(kMiniManglePlatforms,
                          sizeof(kMiniManglePlatforms)),
            GRAPHICS_RGB(230, 0, 0));
        for (int index = 0; index < 4; ++index) {
            if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
                minigame_draw_pc_texture_centered(
                    parts[index], kMiniMangleParts[index][0],
                    kMiniMangleParts[index][1]);
        }
    } else {
        secret_draw_header("MANGLES QUEST", "TAKE THE CAKE TO THE CHILD");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(7, 8, 18));
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 680, 110, 95, 82,
                              GRAPHICS_RGB(73, 113, 224));
        mini_pc_draw_platforms(kMiniMangleSecretPlatforms,
            mini_pc_count(kMiniMangleSecretPlatforms,
                          sizeof(kMiniMangleSecretPlatforms)),
            GRAPHICS_RGB(122, 42, 127));
        minigame_draw_pc_texture_centered(&gMiniChildSad, 778, 92);
        minigame_draw_pc_texture_centered(&gMiniPartyCake, 778, 55);
    }
    minigame_draw_pc_texture_centered(
        sMiniPc.facing_left ? &gMiniMangleLeft : &gMiniMangleRight,
        sSecret.x, sSecret.y);
}

/* ------------------------------------------------------------------------- */
/* CHICA'S PARTY                                                             */
/* ------------------------------------------------------------------------- */

static const MiniPcPlatform kMiniChicaRoom0[] = {
    {0, 414, 854, 26},
    {35, 350, 175, 14},
    {270, 330, 180, 14},
    {520, 285, 180, 14},
    {680, 210, 145, 14},
    {90, 245, 150, 14},
    {330, 195, 170, 14},
};

static const MiniPcPlatform kMiniChicaRoom1[] = {
    {0, 414, 854, 26},
    {55, 350, 190, 14},
    {310, 305, 180, 14},
    {570, 255, 180, 14},
    {690, 150, 135, 14},
    {135, 205, 165, 14},
};

static const MiniPcPlatform kMiniChicaSecret[] = {
    {0, 414, 854, 26},
    {110, 340, 180, 14},
    {360, 260, 180, 14},
    {610, 180, 180, 14},
};

static const int kMiniChicaChildren[4][3] = {
    {0, 355, 305}, {0, 716, 180}, {1, 220, 320}, {1, 620, 225}
};

static void mini_chica_refill(void)
{
    const int needed = 4 - sSecret.delivered;
    if (needed > sSecret.carried) sSecret.carried = needed;
}

static void secret_update_chica_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_CHICA, 85, 388, 0);
    ++sMiniPc.room_frames;
    const MiniPcPlatform *platforms = kMiniChicaRoom0;
    int count = mini_pc_count(kMiniChicaRoom0, sizeof(kMiniChicaRoom0));
    if (sSecret.room == 1) {
        platforms = kMiniChicaRoom1;
        count = mini_pc_count(kMiniChicaRoom1, sizeof(kMiniChicaRoom1));
    } else if (sSecret.room == 2) {
        platforms = kMiniChicaSecret;
        count = mini_pc_count(kMiniChicaSecret, sizeof(kMiniChicaSecret));
    }

    mini_pc_move_horizontal(input, 4);
    mini_pc_apply_platform_physics(input, platforms, count, 12, 19, -11, 10);

    if (sSecret.room == 0 && secret_near(sSecret.x, sSecret.y, 90, 365, 38)) {
        mini_chica_refill();
        sSecret.collected |= 0x10u;
    }

    if (sSecret.room <= 1) {
        for (int index = 0; index < 4; ++index) {
            const uint8_t bit = (uint8_t) (1u << index);
            if ((sSecret.collected & bit) != 0u ||
                kMiniChicaChildren[index][0] != sSecret.room ||
                sSecret.carried <= 0) continue;
            if (!secret_near(sSecret.x, sSecret.y,
                             kMiniChicaChildren[index][1],
                             kMiniChicaChildren[index][2], 34)) continue;
            sSecret.collected |= bit;
            --sSecret.carried;
            ++sSecret.delivered;
            audio_play(AUDIO_CUE_END, 0.42f, false);
        }

        if (sSecret.x > 850) {
            if (sSecret.room == 0) {
                sSecret.room = 1;
                sSecret.x = 20;
            } else if (sSecret.delivered >= 4 && sSecret.y < 180) {
                sSecret.room = 2;
                sSecret.x = 35;
                sSecret.y = 390;
                content_play_select();
            } else {
                sSecret.x = 838;
            }
        } else if (sSecret.x < 4 && sSecret.room == 1) {
            sSecret.room = 0;
            sSecret.x = 834;
        }
    } else if (secret_near(sSecret.x, sSecret.y, 760, 145, 44)) {
        secret_complete(game);
        return;
    }

    if (sSecret.room != 1 || (sSecret.x >= 4 && sSecret.x <= 850))
        mini_pc_clamp_horizontal();
    mini_pc_clamp_top(19);
    if (sSecret.y > kMiniPcBottom + 60)
        mini_pc_begin(SECRET_MINIGAME_CHICA, 85, 388, 0);
    game->dirty = true;
}

static void mini_chica_draw_child(int index)
{
    const uint8_t bit = (uint8_t) (1u << index);
    const TextureRle *child =
        (sSecret.collected & bit) != 0u ? &gMiniChildHappy : &gMiniChildSad;
    minigame_draw_pc_texture_centered(child,
                                     kMiniChicaChildren[index][1],
                                     kMiniChicaChildren[index][2]);
}

static void secret_draw_chica_pc(void)
{
    if (sSecret.room == 0) {
        secret_draw_header("CHICAS PARTY",
                           "TAKE CUPCAKES  DELIVER TO EVERY CHILD");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniChicaRoom0,
            mini_pc_count(kMiniChicaRoom0, sizeof(kMiniChicaRoom0)),
            GRAPHICS_RGB(0, 107, 35));
        minigame_draw_pc_texture_centered(&gMiniCupcakeLarge, 90, 365);
        mini_chica_draw_child(0);
        mini_chica_draw_child(1);
    } else if (sSecret.room == 1) {
        secret_draw_header("CHICAS PARTY",
                           "DELIVER CUPCAKES  HIGH RIGHT = SECRET");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniChicaRoom1,
            mini_pc_count(kMiniChicaRoom1, sizeof(kMiniChicaRoom1)),
            GRAPHICS_RGB(0, 107, 35));
        mini_chica_draw_child(2);
        mini_chica_draw_child(3);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 710, 75, 100, 100,
                           COLOUR_BLACK);
        for (int wx = 720; wx < 800; wx += 38)
            for (int wy = 85; wy < 165; wy += 38)
                graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                                   wx, wy, 28, 28,
                                   GRAPHICS_RGB(255, 255, 50));
    } else {
        secret_draw_header("CHICAS PARTY", "TAKE THE CAKE TO THE CHILD");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniChicaSecret,
            mini_pc_count(kMiniChicaSecret, sizeof(kMiniChicaSecret)),
            GRAPHICS_RGB(0, 107, 35));
        minigame_draw_pc_texture_centered(&gMiniChildSad, 760, 145);
        minigame_draw_pc_texture_centered(&gMiniPartyCake, 760, 100);
    }

    minigame_draw_pc_texture_centered(
        sMiniPc.facing_left ? &gMiniChicaLeft : &gMiniChicaRight,
        sSecret.x, sSecret.y);
    char carried[32];
    snprintf(carried, sizeof(carried), "CUPCAKES %d  GIVEN %d/4",
             sSecret.carried, sSecret.delivered);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 560, 22, 1, carried,
                       GRAPHICS_RGB(255, 240, 65));
}

/* ------------------------------------------------------------------------- */
/* STAGE 01                                                                  */
/* ------------------------------------------------------------------------- */

