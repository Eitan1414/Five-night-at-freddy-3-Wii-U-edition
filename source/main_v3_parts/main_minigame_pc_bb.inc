/* Authentic BB's Air Adventure platform pass.
 *
 * The supplied PC sprites are rendered directly and the old free-flight
 * placeholder is replaced by a three-room platform route: the normal balloon
 * room, the out-of-bounds fall, and the hidden rainbow-balloon/child room.
 * Coordinates use the Wii U logical 854x480 surface while preserving the
 * original 4:3 playfield inside the existing minigame header/footer.
 */
typedef struct MiniBbPlatform {
    int x;
    int y;
    int width;
    int height;
} MiniBbPlatform;

typedef struct MiniBbRuntime {
    bool initialized;
    bool facing_left;
    bool grounded;
    int velocity_y;
    int previous_x;
} MiniBbRuntime;

static MiniBbRuntime sMiniBb;

static const int kMiniBbPlayerHalfWidth = 12;
static const int kMiniBbPlayerHalfHeight = 15;
static const int kMiniBbPlayTop = 58;
static const int kMiniBbPlayBottom = 440;

static const int kMiniBbBalloons[5][2] = {
    {178, 306}, {320, 154}, {468, 282}, {626, 128}, {754, 326}
};

static const MiniBbPlatform kMiniBbRoomPlatforms[] = {
    {0,   414, 854, 26},
    {30,  350, 205, 16},
    {62,  271, 190, 16},
    {92,  191, 180, 16},
    {30,  111, 150, 16},
    {285, 337, 205, 16},
    {350, 257, 180, 16},
    {330, 177, 230, 16},
    {606, 390, 170, 16},
    {646, 311, 150, 16},
    {606, 231, 160, 16},
    {606, 151, 225, 16},
};

static const MiniBbPlatform kMiniBbRainbowPlatforms[] = {
    {0,   414, 854, 26},
    {640, 342, 180, 16},
};

static const MiniBbPlatform kMiniBbChildPlatforms[] = {
    {0,   414, 854, 26},
    {55,  370, 100, 14},
    {185, 320, 100, 14},
    {315, 270, 100, 14},
    {445, 220, 100, 14},
    {575, 170, 100, 14},
    {705, 120, 120, 14},
};

static void minigame_draw_pc_texture_centered(const TextureRle *texture,
                                               int centre_x,
                                               int centre_y)
{
    if (texture == NULL) return;
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     centre_x - (int) texture->width / 2,
                     centre_y - (int) texture->height / 2,
                     texture->width,
                     texture->height,
                     texture);
}

static bool mini_bb_horizontal_overlap(int centre_x,
                                       const MiniBbPlatform *platform)
{
    const int left = centre_x - kMiniBbPlayerHalfWidth;
    const int right = centre_x + kMiniBbPlayerHalfWidth;
    return right > platform->x && left < platform->x + platform->width;
}

static void mini_bb_room_platforms(const MiniBbPlatform **platforms,
                                   int *count)
{
    if (sSecret.room == 0) {
        *platforms = kMiniBbRoomPlatforms;
        *count = (int) (sizeof(kMiniBbRoomPlatforms) /
                        sizeof(kMiniBbRoomPlatforms[0]));
        return;
    }
    if (sSecret.room == 2 &&
        progress_save_is_secret_minigame_completed(
            (int) SECRET_MINIGAME_MANGLE + 1)) {
        *platforms = kMiniBbChildPlatforms;
        *count = (int) (sizeof(kMiniBbChildPlatforms) /
                        sizeof(kMiniBbChildPlatforms[0]));
        return;
    }
    if (sSecret.room == 2) {
        *platforms = kMiniBbRainbowPlatforms;
        *count = (int) (sizeof(kMiniBbRainbowPlatforms) /
                        sizeof(kMiniBbRainbowPlatforms[0]));
        return;
    }
    *platforms = NULL;
    *count = 0;
}

static void mini_bb_reset(void)
{
    memset(&sMiniBb, 0, sizeof(sMiniBb));
    sMiniBb.initialized = true;
    sMiniBb.previous_x = 84;
    sSecret.x = 84;
    sSecret.y = 390;
    sSecret.room = 0;
}

static void mini_bb_collect_normal_balloons(Game *game)
{
    for (int index = 0; index < 5; ++index) {
        const uint8_t bit = (uint8_t) (1u << index);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!secret_near(sSecret.x, sSecret.y,
                         kMiniBbBalloons[index][0],
                         kMiniBbBalloons[index][1], 28)) {
            continue;
        }
        sSecret.collected |= bit;
        content_play_select();
        game->dirty = true;
    }
}

static void mini_bb_move_horizontal(const InputState *input)
{
    int direction = 0;
    if (input_is_held(input, GAME_BUTTON_LEFT)) --direction;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) ++direction;

    sSecret.x += direction * 4;
    if (direction < 0) sMiniBb.facing_left = true;
    if (direction > 0) sMiniBb.facing_left = false;

    if (sSecret.x < 16) sSecret.x = 16;
    if (sSecret.x > 838) sSecret.x = 838;
}

static void mini_bb_apply_vertical(const InputState *input)
{
    const MiniBbPlatform *platforms = NULL;
    int platform_count = 0;
    mini_bb_room_platforms(&platforms, &platform_count);

    if (sMiniBb.grounded &&
        (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
         input_was_pressed(input, GAME_BUTTON_UP))) {
        sMiniBb.velocity_y = -11;
        sMiniBb.grounded = false;
    }

    if (sMiniBb.velocity_y < 10) ++sMiniBb.velocity_y;

    const int old_y = sSecret.y;
    int new_y = old_y + sMiniBb.velocity_y;
    sMiniBb.grounded = false;

    if (sMiniBb.velocity_y >= 0 && platforms != NULL) {
        const int old_bottom = old_y + kMiniBbPlayerHalfHeight;
        const int new_bottom = new_y + kMiniBbPlayerHalfHeight;
        for (int index = 0; index < platform_count; ++index) {
            const MiniBbPlatform *platform = &platforms[index];
            if (!mini_bb_horizontal_overlap(sSecret.x, platform)) continue;
            if (old_bottom > platform->y || new_bottom < platform->y) continue;
            new_y = platform->y - kMiniBbPlayerHalfHeight;
            sMiniBb.velocity_y = 0;
            sMiniBb.grounded = true;
            break;
        }
    }

    sSecret.y = new_y;
}

static void mini_bb_enter_fall_room(void)
{
    sSecret.room = 1;
    sSecret.x = 105;
    sSecret.y = kMiniBbPlayTop + 18;
    sMiniBb.velocity_y = 4;
    sMiniBb.grounded = false;
}

static void mini_bb_enter_secret_room(void)
{
    sSecret.room = 2;
    sSecret.x = 55;
    sSecret.y = 90;
    sMiniBb.velocity_y = 2;
    sMiniBb.grounded = false;
}

static void secret_update_bb_pc(Game *game, const InputState *input)
{
    if (!sMiniBb.initialized || sSecret.frames <= 1u) mini_bb_reset();

    sMiniBb.previous_x = sSecret.x;
    mini_bb_move_horizontal(input);
    mini_bb_apply_vertical(input);

    if (sSecret.room == 0) {
        mini_bb_collect_normal_balloons(game);

        const bool all_balloons = (sSecret.collected & 0x1Fu) == 0x1Fu;
        if (all_balloons && sSecret.x < 155 && sSecret.y < 74) {
            mini_bb_enter_fall_room();
            content_play_select();
            game->dirty = true;
            return;
        }

        if (sSecret.y < kMiniBbPlayTop + kMiniBbPlayerHalfHeight) {
            sSecret.y = kMiniBbPlayTop + kMiniBbPlayerHalfHeight;
            if (sMiniBb.velocity_y < 0) sMiniBb.velocity_y = 0;
        }

        if (all_balloons && secret_near(sSecret.x, sSecret.y, 790, 92, 38)) {
            secret_complete(game);
            return;
        }
    } else if (sSecret.room == 1) {
        if (sSecret.y > kMiniBbPlayBottom + 30) {
            mini_bb_enter_secret_room();
            game->dirty = true;
            return;
        }
    } else {
        const bool child_route = progress_save_is_secret_minigame_completed(
            (int) SECRET_MINIGAME_MANGLE + 1);
        if (child_route) {
            if (secret_near(sSecret.x, sSecret.y, 782, 82, 34)) {
                secret_complete(game);
                return;
            }
        } else if (secret_near(sSecret.x, sSecret.y, 770, 305, 34)) {
            secret_complete(game);
            return;
        }
    }

    if (sSecret.room != 1 && sSecret.y > kMiniBbPlayBottom + 70) {
        if (sSecret.room == 0) mini_bb_reset();
        else mini_bb_enter_secret_room();
    }
    game->dirty = true;
}

static void mini_bb_draw_platforms(const MiniBbPlatform *platforms,
                                   int count,
                                   uint32_t colour)
{
    for (int index = 0; index < count; ++index) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           platforms[index].x,
                           platforms[index].y,
                           platforms[index].width,
                           platforms[index].height,
                           colour);
    }
}

static void mini_bb_draw_rainbow_balloon(int x, int y)
{
    static const uint32_t colours[5] = {
        GRAPHICS_RGB(235, 55, 63),
        GRAPHICS_RGB(239, 142, 37),
        GRAPHICS_RGB(239, 219, 55),
        GRAPHICS_RGB(62, 201, 86),
        GRAPHICS_RGB(83, 118, 220),
    };
    for (int index = 0; index < 5; ++index) {
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH,
                              x - 22 + index * 11,
                              y,
                              11,
                              17,
                              colours[index]);
    }
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 1, y + 17, 3, 25,
                       COLOUR_WHITE);
}

static void secret_draw_bb_pc(void)
{
    static const TextureRle *const balloon_textures[5] = {
        &gMiniBalloonSmallRed,
        &gMiniBalloonRed,
        &gMiniBalloonPurple,
        &gMiniBalloonGreen,
        &gMiniBalloonOrange,
    };

    const bool child_route = progress_save_is_secret_minigame_completed(
        (int) SECRET_MINIGAME_MANGLE + 1);

    if (sSecret.room == 0) {
        secret_draw_header("BBS AIR ADVENTURE",
                           "D-PAD: MOVE  A/UP: JUMP  FIND THE FALSE WALL");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_bb_draw_platforms(kMiniBbRoomPlatforms,
            (int) (sizeof(kMiniBbRoomPlatforms) /
                   sizeof(kMiniBbRoomPlatforms[0])),
            GRAPHICS_RGB(0, 0, 8));

        for (int index = 0; index < 5; ++index) {
            if ((sSecret.collected & (uint8_t) (1u << index)) == 0u) {
                minigame_draw_pc_texture_centered(balloon_textures[index],
                                                   kMiniBbBalloons[index][0],
                                                   kMiniBbBalloons[index][1]);
            }
        }
        minigame_draw_pc_texture_centered(&gMiniExitCloud, 790, 92);
    } else if (sSecret.room == 1) {
        secret_draw_header("BBS AIR ADVENTURE", "OUT OF BOUNDS");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(0, 0, 0));
        for (int y = 78; y < 440; y += 58) {
            const uint32_t stripe = ((y / 58) & 1) != 0
                ? GRAPHICS_RGB(167, 167, 167)
                : GRAPHICS_RGB(31, 31, 31);
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y, 854, 32, stripe);
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y + 32, 854, 7,
                               COLOUR_WHITE);
        }
    } else {
        secret_draw_header("BBS AIR ADVENTURE",
                           child_route ? "FOLLOW THE BALLOONS TO THE CHILD"
                                       : "COLLECT THE HIDDEN RAINBOW BALLOON");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        if (child_route) {
            mini_bb_draw_platforms(kMiniBbChildPlatforms,
                (int) (sizeof(kMiniBbChildPlatforms) /
                       sizeof(kMiniBbChildPlatforms[0])),
                GRAPHICS_RGB(50, 95, 178));
            for (int index = 0; index < 6; ++index) {
                const int bx = 105 + index * 130;
                const int by = 340 - index * 50;
                minigame_draw_pc_texture_centered(
                    index % 2 == 0 ? &gMiniBalloonPurple : &gMiniBalloonGreen,
                    bx, by);
            }
            content_draw_pixel_person(782, 82,
                                      GRAPHICS_RGB(70, 70, 70),
                                      GRAPHICS_RGB(180, 180, 180), false);
            content_draw_cake(782, 58, true);
        } else {
            mini_bb_draw_platforms(kMiniBbRainbowPlatforms,
                (int) (sizeof(kMiniBbRainbowPlatforms) /
                       sizeof(kMiniBbRainbowPlatforms[0])),
                GRAPHICS_RGB(50, 95, 178));
            mini_bb_draw_rainbow_balloon(770, 305);
        }
    }

    minigame_draw_pc_texture_centered(sMiniBb.facing_left ? &gMiniBbLeft
                                                          : &gMiniBbRight,
                                      sSecret.x,
                                      sSecret.y);
}

static void secret_update_active_pc(Game *game, const InputState *input)
{
    ++sSecret.frames;
    ++game->screen_frames;
    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending) {
        sMiniBb.initialized = false;
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
            sMiniBb.initialized = false;
            secret_return_to_menu(game);
        }
        return;
    }
    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: secret_update_bb_pc(game, input); break;
        case SECRET_MINIGAME_MANGLE: secret_update_mangle(game, input); break;
        case SECRET_MINIGAME_CHICA: secret_update_chica(game, input); break;
        case SECRET_MINIGAME_STAGE01: secret_update_stage01(game, input); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: secret_update_shadow(game, input); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: secret_update_happiest(game, input); break;
        default: break;
    }
    game->dirty = true;
}
