static const MiniPcPlatform kMiniHappiestHall[] = {
    {0, 414, 854, 26},
};

static const MiniPcPlatform kMiniHappiestParty[] = {
    {0, 414, 854, 26},
    {160, 330, 535, 16},
};

static const int kMiniHappiestChildren[4][2] = {
    {185, 365}, {340, 365}, {495, 365}, {650, 365}
};

static void secret_update_happiest_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_HAPPIEST_DAY, 70, 390, 0);
    ++sMiniPc.room_frames;
    const MiniPcPlatform *platforms =
        sSecret.room == 0 ? kMiniHappiestHall : kMiniHappiestParty;
    const int count = sSecret.room == 0
        ? mini_pc_count(kMiniHappiestHall, sizeof(kMiniHappiestHall))
        : mini_pc_count(kMiniHappiestParty, sizeof(kMiniHappiestParty));

    mini_pc_move_horizontal(input, 4);
    mini_pc_apply_platform_physics(input, platforms, count, 10, 19, -9, 9);

    if (sSecret.room == 0) {
        for (int index = 0; index < 4; ++index) {
            const uint8_t bit = (uint8_t) (1u << index);
            if ((sSecret.collected & bit) != 0u) continue;
            if (!secret_near(sSecret.x, sSecret.y,
                             kMiniHappiestChildren[index][0],
                             kMiniHappiestChildren[index][1], 34)) continue;
            sSecret.collected |= bit;
            ++sSecret.delivered;
            audio_play(AUDIO_CUE_END, 0.42f, false);
        }
        if (sSecret.delivered >= 4 &&
            secret_near(sSecret.x, sSecret.y, 800, 360, 45)) {
            sSecret.room = 1;
            sSecret.x = 80;
            sSecret.y = 390;
            sMiniPc.velocity_y = 0;
            content_play_select();
            return;
        }
    } else if (secret_near(sSecret.x, sSecret.y, 425, 285, 52)) {
        secret_complete(game);
        return;
    }

    mini_pc_clamp_horizontal();
    mini_pc_clamp_top(19);
    if (sSecret.y > kMiniPcBottom + 60)
        mini_pc_begin(SECRET_MINIGAME_HAPPIEST_DAY, 70, 390, 0);
    game->dirty = true;
}

static void secret_draw_happiest_pc(void)
{
    static const TextureRle *const children[5] = {
        &gMiniPartyChild1, &gMiniPartyChild2, &gMiniPartyChild3,
        &gMiniPartyChild4, &gMiniPartyChild5,
    };
    static const TextureRle *const balloons[4] = {
        &gMiniBalloonPurple, &gMiniBalloonGreen,
        &gMiniBalloonOrange, &gMiniBalloonRed,
    };

    if (sSecret.room == 0) {
        secret_draw_header("HAPPIEST DAY",
                           "VISIT 4 CHILDREN  THEN ENTER THE EXIT");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniHappiestHall,
            mini_pc_count(kMiniHappiestHall, sizeof(kMiniHappiestHall)),
            GRAPHICS_RGB(83, 83, 83));
        for (int index = 0; index < 4; ++index) {
            minigame_draw_pc_texture_centered(
                children[index],
                kMiniHappiestChildren[index][0],
                kMiniHappiestChildren[index][1]);
            if ((sSecret.collected & (uint8_t) (1u << index)) != 0u)
                minigame_draw_pc_texture_centered(
                    balloons[index],
                    kMiniHappiestChildren[index][0],
                    285);
        }
        minigame_draw_pc_texture_centered(&gMiniExitDoor, 800, 355);
    } else {
        secret_draw_header("HAPPIEST DAY", "JOIN THE FINAL CAKE TABLE");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(12, 12, 14));
        mini_pc_draw_platforms(kMiniHappiestParty,
            mini_pc_count(kMiniHappiestParty, sizeof(kMiniHappiestParty)),
            GRAPHICS_RGB(83, 83, 83));
        for (int index = 0; index < 5; ++index)
            minigame_draw_pc_texture_centered(
                children[index], 205 + index * 110, 365);
        minigame_draw_pc_texture_centered(&gMiniPartyCake, 425, 285);
    }

    minigame_draw_pc_texture_centered(
        sMiniPc.facing_left ? &gMiniPuppetLeft : &gMiniPuppetRight,
        sSecret.x, sSecret.y);
}

/* ------------------------------------------------------------------------- */
/* Shared active dispatcher                                                  */
/* ------------------------------------------------------------------------- */

static void secret_update_active_pc(Game *game, const InputState *input)
{
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
