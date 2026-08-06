/* Authentic PC secret-minigame gameplay layer.
 *
 * The original PC sprite sheets are used for every playable character and
 * objective. A shared platform/sensor engine replaces the old unrestricted
 * four-direction placeholders for BB, Mangle, Chica and Stage 01. Shadow
 * Bonnie keeps its original phase-style free movement, while Happiest Day
 * uses the original Puppet, masked children, balloons, exit and cake.
 *
 * The room coordinates are expressed on the Wii U logical 854x480 surface.
 * They are deliberately data-driven so the remaining MFA collision audit can
 * adjust rectangles without rewriting gameplay code.
 */
typedef struct MiniPcPlatform {
    int x;
    int y;
    int width;
    int height;
} MiniPcPlatform;

typedef struct MiniPcRuntime {
    bool initialized;
    SecretMinigameKind kind;
    bool facing_left;
    bool grounded;
    int velocity_y;
    int drop_frames;
    int room_frames;
} MiniPcRuntime;

static MiniPcRuntime sMiniPc;

static const int kMiniPcTop = 58;
static const int kMiniPcBottom = 440;

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

static int mini_pc_count(const MiniPcPlatform *platforms, size_t bytes)
{
    (void) platforms;
    return (int) (bytes / sizeof(MiniPcPlatform));
}

static bool mini_pc_horizontal_overlap(int centre_x,
                                       int half_width,
                                       const MiniPcPlatform *platform)
{
    const int left = centre_x - half_width;
    const int right = centre_x + half_width;
    return right > platform->x && left < platform->x + platform->width;
}

static void mini_pc_begin(SecretMinigameKind kind,
                          int x,
                          int y,
                          int room)
{
    memset(&sMiniPc, 0, sizeof(sMiniPc));
    sMiniPc.initialized = true;
    sMiniPc.kind = kind;
    sMiniPc.grounded = false;
    sSecret.x = x;
    sSecret.y = y;
    sSecret.room = room;
}

static void mini_pc_reset_if_needed(SecretMinigameKind kind,
                                    int x,
                                    int y,
                                    int room)
{
    if (!sMiniPc.initialized || sMiniPc.kind != kind || sSecret.frames <= 1u)
        mini_pc_begin(kind, x, y, room);
}

static void mini_pc_move_horizontal(const InputState *input, int speed)
{
    int direction = 0;
    if (input_is_held(input, GAME_BUTTON_LEFT)) --direction;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) ++direction;
    sSecret.x += direction * speed;
    if (direction < 0) sMiniPc.facing_left = true;
    if (direction > 0) sMiniPc.facing_left = false;
}

static void mini_pc_apply_platform_physics(const InputState *input,
                                           const MiniPcPlatform *platforms,
                                           int platform_count,
                                           int half_width,
                                           int half_height,
                                           int jump_velocity,
                                           int gravity_limit)
{
    const bool jump_pressed =
        input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_UP);
    const bool drop_pressed =
        input_is_held(input, GAME_BUTTON_DOWN) && jump_pressed;

    if (drop_pressed && sMiniPc.grounded) {
        sMiniPc.drop_frames = 8;
        sMiniPc.grounded = false;
        sMiniPc.velocity_y = 2;
    } else if (jump_pressed && sMiniPc.grounded) {
        sMiniPc.velocity_y = jump_velocity;
        sMiniPc.grounded = false;
    }

    if (sMiniPc.drop_frames > 0) --sMiniPc.drop_frames;
    if (sMiniPc.velocity_y < gravity_limit) ++sMiniPc.velocity_y;

    const int old_y = sSecret.y;
    int new_y = old_y + sMiniPc.velocity_y;
    sMiniPc.grounded = false;

    if (sMiniPc.velocity_y >= 0 && sMiniPc.drop_frames == 0) {
        const int old_bottom = old_y + half_height;
        const int new_bottom = new_y + half_height;
        for (int index = 0; index < platform_count; ++index) {
            const MiniPcPlatform *platform = &platforms[index];
            if (!mini_pc_horizontal_overlap(sSecret.x, half_width, platform))
                continue;
            if (old_bottom > platform->y || new_bottom < platform->y)
                continue;
            new_y = platform->y - half_height;
            sMiniPc.velocity_y = 0;
            sMiniPc.grounded = true;
            break;
        }
    }
    sSecret.y = new_y;
}

static void mini_pc_clamp_horizontal(void)
{
    if (sSecret.x < 16) sSecret.x = 16;
    if (sSecret.x > 838) sSecret.x = 838;
}

static void mini_pc_clamp_top(int half_height)
{
    const int top = kMiniPcTop + half_height;
    if (sSecret.y < top) {
        sSecret.y = top;
        if (sMiniPc.velocity_y < 0) sMiniPc.velocity_y = 0;
    }
}

static void mini_pc_draw_platforms(const MiniPcPlatform *platforms,
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

static void mini_pc_draw_striped_void(uint32_t primary,
                                      uint32_t secondary)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382, COLOUR_BLACK);
    for (int y = 72; y < 440; y += 54) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y, 854, 24, primary);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y + 24, 854, 6,
                           COLOUR_WHITE);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y + 30, 854, 10,
                           secondary);
    }
}

static void mini_pc_draw_rainbow_balloon(int x, int y)
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

/* ------------------------------------------------------------------------- */
/* BB'S AIR ADVENTURE                                                        */
/* ------------------------------------------------------------------------- */

static const int kMiniBbBalloons[5][2] = {
    {178, 306}, {320, 154}, {468, 282}, {626, 128}, {754, 326}
};

static const MiniPcPlatform kMiniBbPlatforms[] = {
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

static const MiniPcPlatform kMiniBbRainbowPlatforms[] = {
    {0,   414, 854, 26},
    {620, 342, 205, 16},
};

static const MiniPcPlatform kMiniBbChildPlatforms[] = {
    {0,   414, 854, 26},
    {55,  370, 100, 14},
    {185, 320, 100, 14},
    {315, 270, 100, 14},
    {445, 220, 100, 14},
    {575, 170, 100, 14},
    {705, 120, 120, 14},
};

static void mini_bb_platforms(const MiniPcPlatform **platforms, int *count)
{
    if (sSecret.room == 0) {
        *platforms = kMiniBbPlatforms;
        *count = mini_pc_count(kMiniBbPlatforms, sizeof(kMiniBbPlatforms));
    } else if (sSecret.room == 2 &&
               progress_save_is_secret_minigame_completed(
                   (int) SECRET_MINIGAME_MANGLE + 1)) {
        *platforms = kMiniBbChildPlatforms;
        *count = mini_pc_count(kMiniBbChildPlatforms,
                               sizeof(kMiniBbChildPlatforms));
    } else if (sSecret.room == 2) {
        *platforms = kMiniBbRainbowPlatforms;
        *count = mini_pc_count(kMiniBbRainbowPlatforms,
                               sizeof(kMiniBbRainbowPlatforms));
    } else {
        *platforms = NULL;
        *count = 0;
    }
}

static void mini_bb_enter_room(int room, int x, int y, int velocity)
{
    sSecret.room = room;
    sSecret.x = x;
    sSecret.y = y;
    sMiniPc.velocity_y = velocity;
    sMiniPc.grounded = false;
    sMiniPc.room_frames = 0;
}

static void secret_update_bb_pc(Game *game, const InputState *input)
{
    mini_pc_reset_if_needed(SECRET_MINIGAME_BB, 84, 390, 0);
    ++sMiniPc.room_frames;

    const MiniPcPlatform *platforms = NULL;
    int platform_count = 0;
    mini_bb_platforms(&platforms, &platform_count);

    mini_pc_move_horizontal(input, 4);
    mini_pc_apply_platform_physics(input, platforms, platform_count,
                                   12, 15, -11, 10);

    if (sSecret.room == 0) {
        for (int index = 0; index < 5; ++index) {
            const uint8_t bit = (uint8_t) (1u << index);
            if ((sSecret.collected & bit) != 0u) continue;
            if (!secret_near(sSecret.x, sSecret.y,
                             kMiniBbBalloons[index][0],
                             kMiniBbBalloons[index][1], 28)) continue;
            sSecret.collected |= bit;
            content_play_select();
        }

        const bool all = (sSecret.collected & 0x1Fu) == 0x1Fu;
        if (all && sSecret.x < 155 &&
            sSecret.y < kMiniPcTop + 20) {
            mini_bb_enter_room(1, 105, kMiniPcTop + 18, 4);
            content_play_select();
            return;
        }
        mini_pc_clamp_horizontal();
        mini_pc_clamp_top(15);
        if (all && secret_near(sSecret.x, sSecret.y, 790, 92, 38)) {
            secret_complete(game);
            return;
        }
    } else if (sSecret.room == 1) {
        mini_pc_clamp_horizontal();
        if (sSecret.y > kMiniPcBottom + 30) {
            mini_bb_enter_room(2, 55, 90, 2);
            return;
        }
    } else {
        mini_pc_clamp_horizontal();
        mini_pc_clamp_top(15);
        const bool child_route =
            progress_save_is_secret_minigame_completed(
                (int) SECRET_MINIGAME_MANGLE + 1);
        if (child_route &&
            secret_near(sSecret.x, sSecret.y, 782, 82, 36)) {
            secret_complete(game);
            return;
        }
        if (!child_route &&
            secret_near(sSecret.x, sSecret.y, 770, 305, 36)) {
            secret_complete(game);
            return;
        }
    }

    if (sSecret.room != 1 && sSecret.y > kMiniPcBottom + 70) {
        if (sSecret.room == 0)
            mini_pc_begin(SECRET_MINIGAME_BB, 84, 390, 0);
        else
            mini_bb_enter_room(2, 55, 90, 2);
    }
    game->dirty = true;
}

static void secret_draw_bb_pc(void)
{
    static const TextureRle *const balloons[5] = {
        &gMiniBalloonSmallRed, &gMiniBalloonRed, &gMiniBalloonPurple,
        &gMiniBalloonGreen, &gMiniBalloonOrange,
    };
    const bool child_route =
        progress_save_is_secret_minigame_completed(
            (int) SECRET_MINIGAME_MANGLE + 1);

    if (sSecret.room == 0) {
        secret_draw_header("BBS AIR ADVENTURE",
                           "D-PAD MOVE  A/UP JUMP  DOWN+A DROP");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        mini_pc_draw_platforms(kMiniBbPlatforms,
            mini_pc_count(kMiniBbPlatforms, sizeof(kMiniBbPlatforms)),
            GRAPHICS_RGB(0, 0, 8));
        for (int index = 0; index < 5; ++index) {
            if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
                minigame_draw_pc_texture_centered(
                    balloons[index],
                    kMiniBbBalloons[index][0],
                    kMiniBbBalloons[index][1]);
        }
        minigame_draw_pc_texture_centered(&gMiniExitCloud, 790, 92);
    } else if (sSecret.room == 1) {
        secret_draw_header("BBS AIR ADVENTURE", "OUT OF BOUNDS");
        mini_pc_draw_striped_void(GRAPHICS_RGB(167, 167, 167),
                                  GRAPHICS_RGB(31, 31, 31));
    } else {
        secret_draw_header("BBS AIR ADVENTURE",
                           child_route ? "FOLLOW THE BALLOONS TO THE CHILD"
                                       : "COLLECT THE HIDDEN RAINBOW BALLOON");
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                           GRAPHICS_RGB(55, 177, 182));
        if (child_route) {
            mini_pc_draw_platforms(kMiniBbChildPlatforms,
                mini_pc_count(kMiniBbChildPlatforms,
                              sizeof(kMiniBbChildPlatforms)),
                GRAPHICS_RGB(50, 95, 178));
            for (int index = 0; index < 6; ++index) {
                minigame_draw_pc_texture_centered(
                    (index & 1) != 0 ? &gMiniBalloonGreen
                                     : &gMiniBalloonPurple,
                    105 + index * 130,
                    340 - index * 50);
            }
            minigame_draw_pc_texture_centered(&gMiniChildSad, 782, 84);
            minigame_draw_pc_texture_centered(&gMiniPartyCake, 782, 52);
        } else {
            mini_pc_draw_platforms(kMiniBbRainbowPlatforms,
                mini_pc_count(kMiniBbRainbowPlatforms,
                              sizeof(kMiniBbRainbowPlatforms)),
                GRAPHICS_RGB(50, 95, 178));
            mini_pc_draw_rainbow_balloon(770, 305);
        }
    }
    minigame_draw_pc_texture_centered(
        sMiniPc.facing_left ? &gMiniBbLeft : &gMiniBbRight,
        sSecret.x, sSecret.y);
}

/* ------------------------------------------------------------------------- */
/* MANGLE'S QUEST                                                            */
/* ------------------------------------------------------------------------- */

