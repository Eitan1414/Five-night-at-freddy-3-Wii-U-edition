/* Original Clickteam MFA layout override for the six secret minigames.
 *
 * Source reference: Five Nights at Freddy's 3 PC project fivenights3-94.mfa.
 * The original minigame frames are 3072x2304 (a 3x3 world of 1024x768
 * camera pages).  Wii U keeps that 4:3 composition in a centered 640x480
 * viewport instead of flattening the whole world into the 854x480 output.
 *
 * Object/world placement below comes from the MFA frame instances.  The
 * existing portable state/save layer is intentionally retained; event-sheet
 * parity (platform physics, every warp/glitch rule) can be layered on top
 * without moving these original coordinates again. */

#define PC_MFA_WORLD_W 3072
#define PC_MFA_WORLD_H 2304
#define PC_MFA_CAMERA_W 1024
#define PC_MFA_CAMERA_H 768
#define PC_MFA_SCREEN_X 107
#define PC_MFA_SCREEN_Y 0
#define PC_MFA_SCREEN_W 640
#define PC_MFA_SCREEN_H 480

typedef struct PcMfaPoint {
    int x;
    int y;
} PcMfaPoint;

typedef struct PcMfaRect {
    int x;
    int y;
    int w;
    int h;
} PcMfaRect;

static const PcMfaPoint kPcMfaStart[SECRET_MINIGAME_COUNT] = {
    {445, 573}, /* BB, MFA frame 15 */
    {453, 587}, /* Mangle, frame 17 */
    {445, 558}, /* Toy Chica, frame 20 */
    {255, 479}, /* GFreddy / Stage 01, frame 21 */
    {195, 339}, /* RWQFSFASXC, frame 22 */
    {445, 573}, /* Marion / Happiest Day, frame 23 */
};

/* BB: event collision objects named collect / collect 2. */
static const PcMfaPoint kPcMfaBbCollect[8] = {
    {646, 482}, {742, 484}, {388, 416}, {188, 332},
    {474, 254}, {548, 252}, {786, 230}, {641, 42},
};
static const PcMfaPoint kPcMfaBbBalloons[8] = {
    {65, 1130}, {340, 1240}, {656, 1328}, {860, 1270},
    {1050, 1222}, {1242, 1168}, {1464, 1166}, {1698, 1166},
};
static const PcMfaPoint kPcMfaBbExit = {198, 601};
static const PcMfaPoint kPcMfaBbHidden = {2540, 1754};
static const PcMfaPoint kPcMfaBbBigCake = {2326, 1076};

/* Mangle: the four individual part Active objects in the MFA. */
static const PcMfaPoint kPcMfaMangleParts[4] = {
    {673, 504}, {1393, 471}, {2191, 419}, {1801, 334},
};
static const PcMfaPoint kPcMfaMangleExit = {2722, 600};
static const PcMfaPoint kPcMfaMangleAltExit = {150, 2207};
static const PcMfaPoint kPcMfaMangleKid = {2741, 586};
static const PcMfaPoint kPcMfaMangleShadow = {310, 666};
static const PcMfaPoint kPcMfaMangleBalloonPath[12] = {
    {909, 1305}, {1320, 2150}, {1134, 2006}, {1418, 1932},
    {1716, 1862}, {1952, 1726}, {1738, 1624}, {1542, 1526},
    {1270, 1422}, {1016, 1305}, {658, 1304}, {414, 1412},
};

/* Chica: four event pickup points and the four Active 8 child placements. */
static const PcMfaPoint kPcMfaChicaCupcakes[4] = {
    {1283, 513}, {1972, 240}, {2334, 1240}, {2659, 1238},
};
static const PcMfaPoint kPcMfaChicaChildren[4] = {
    {2679, 668}, {2375, 668}, {1678, 1154}, {1224, 1356},
};
static const PcMfaPoint kPcMfaChicaExit = {1874, 1289};
static const PcMfaPoint kPcMfaChicaAltExit = {128, 2206};
static const PcMfaPoint kPcMfaChicaBalloons[2] = {
    {785, 1260}, {612, 1184},
};

/* Stage 01: repeated Active 8 placements form the original room grid. */
static const PcMfaPoint kPcMfaStageNodes[8] = {
    {386, 486}, {388, 1238}, {1434, 1236}, {1426, 472},
    {2476, 1236}, {390, 1954}, {1430, 1958}, {2448, 1963},
};
static const PcMfaPoint kPcMfaStageGoalNodes[3] = {
    {386, 486}, {1434, 1236}, {2476, 1236},
};
static const PcMfaPoint kPcMfaStageCake = {2732, 566};
static const PcMfaPoint kPcMfaStageExit = {2996, 2207};
static const PcMfaPoint kPcMfaStageZipUp[8] = {
    {1881, 1713}, {2188, 1939}, {2190, 1727}, {1308, 1380},
    {1338, 584}, {1158, 222}, {1912, 1120}, {1912, 978},
};
static const PcMfaPoint kPcMfaStageZipRight[2] = {
    {2180, 452}, {2910, 410},
};

/* RWQFSFASXC: four scene anchors plus original cake / child / blockers. */
static const PcMfaPoint kPcMfaShadowAnchors[4] = {
    {1346, 574}, {2547, 587}, {1604, 648}, {227, 1157},
};
static const PcMfaPoint kPcMfaShadowCake = {1210, 1440};
static const PcMfaPoint kPcMfaShadowKid = {777, 1324};
static const PcMfaPoint kPcMfaShadowExit = {772, 227};
static const PcMfaRect kPcMfaShadowBlocks[4] = {
    {1012, 381, 86, 46}, {8, 385, 86, 46},
    {511, 759, 86, 46}, {513, 6, 86, 46},
};

/* Happiest Day: the four mask-child actors and party cake line. */
static const PcMfaPoint kPcMfaHappiestChildren[4] = {
    {2151, 598}, {2246, 598}, {2352, 598}, {2462, 598},
};
static const PcMfaPoint kPcMfaHappiestGuests[5] = {
    {533, 587}, {913, 590}, {1150, 591}, {1531, 590}, {1665, 591},
};
static const PcMfaPoint kPcMfaHappiestCakes[4] = {
    {730, 476}, {1337, 478}, {1840, 478}, {2686, 501},
};
static const PcMfaPoint kPcMfaHappiestTrigger = {2559, 524};

static int pc_mfa_clamp(int value, int minimum, int maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static void pc_mfa_camera(int *camera_x, int *camera_y)
{
    int x = sSecret.x - PC_MFA_CAMERA_W / 2;
    int y = sSecret.y - PC_MFA_CAMERA_H / 2;
    x = pc_mfa_clamp(x, 0, PC_MFA_WORLD_W - PC_MFA_CAMERA_W);
    y = pc_mfa_clamp(y, 0, PC_MFA_WORLD_H - PC_MFA_CAMERA_H);
    *camera_x = x;
    *camera_y = y;
}

static int pc_mfa_screen_x(int world_x, int camera_x)
{
    return PC_MFA_SCREEN_X +
        (world_x - camera_x) * PC_MFA_SCREEN_W / PC_MFA_CAMERA_W;
}

static int pc_mfa_screen_y(int world_y, int camera_y)
{
    return PC_MFA_SCREEN_Y +
        (world_y - camera_y) * PC_MFA_SCREEN_H / PC_MFA_CAMERA_H;
}

static bool pc_mfa_visible(int world_x, int world_y,
                           int camera_x, int camera_y, int margin)
{
    return world_x >= camera_x - margin &&
           world_x <= camera_x + PC_MFA_CAMERA_W + margin &&
           world_y >= camera_y - margin &&
           world_y <= camera_y + PC_MFA_CAMERA_H + margin;
}

static void pc_mfa_draw_sprite(const TextureRle *texture,
                               PcMfaPoint point,
                               int camera_x, int camera_y,
                               int width, int height)
{
    if (!pc_mfa_visible(point.x, point.y, camera_x, camera_y, 160)) return;
    pc_finishing_draw_sprite_centered(
        texture,
        pc_mfa_screen_x(point.x, camera_x),
        pc_mfa_screen_y(point.y, camera_y),
        width * PC_MFA_SCREEN_W / PC_MFA_CAMERA_W,
        height * PC_MFA_SCREEN_H / PC_MFA_CAMERA_H);
}

static void pc_mfa_draw_balloon(PcMfaPoint point, int camera_x, int camera_y,
                                uint32_t colour)
{
    if (!pc_mfa_visible(point.x, point.y, camera_x, camera_y, 80)) return;
    const int x = pc_mfa_screen_x(point.x, camera_x);
    const int y = pc_mfa_screen_y(point.y, camera_y);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, x, y, 8, 10, colour);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 1, y + 10, 2, 14,
                       COLOUR_WHITE);
}

static void pc_mfa_draw_cake(PcMfaPoint point, int camera_x, int camera_y,
                             bool lit)
{
    if (!pc_mfa_visible(point.x, point.y, camera_x, camera_y, 100)) return;
    content_draw_cake(pc_mfa_screen_x(point.x, camera_x),
                      pc_mfa_screen_y(point.y, camera_y), lit);
}

static void pc_mfa_draw_world_marker(PcMfaPoint point,
                                     int camera_x, int camera_y,
                                     uint32_t colour)
{
    if (!pc_mfa_visible(point.x, point.y, camera_x, camera_y, 40)) return;
    const int x = pc_mfa_screen_x(point.x, camera_x);
    const int y = pc_mfa_screen_y(point.y, camera_y);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 11, y - 4, 22, 8, colour);
}

static void pc_mfa_draw_platform(int world_x, int world_y,
                                 int world_w, int world_h,
                                 int camera_x, int camera_y,
                                 uint32_t colour)
{
    const int x0 = pc_mfa_screen_x(world_x, camera_x);
    const int y0 = pc_mfa_screen_y(world_y, camera_y);
    const int x1 = pc_mfa_screen_x(world_x + world_w, camera_x);
    const int y1 = pc_mfa_screen_y(world_y + world_h, camera_y);
    if (x1 < PC_MFA_SCREEN_X || x0 > PC_MFA_SCREEN_X + PC_MFA_SCREEN_W ||
        y1 < 0 || y0 > PC_MFA_SCREEN_H) return;
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x0, y0,
                       x1 - x0 > 1 ? x1 - x0 : 1,
                       y1 - y0 > 1 ? y1 - y0 : 1, colour);
}

static void pc_mfa_draw_page_grid(int camera_x, int camera_y,
                                  uint32_t colour)
{
    /* Page seams are not gameplay art; they are deliberately very dim and
     * only help hardware testing verify the original 1024x768 scrolling. */
    for (int x = 1024; x < PC_MFA_WORLD_W; x += 1024) {
        const int sx = pc_mfa_screen_x(x, camera_x);
        if (sx >= PC_MFA_SCREEN_X && sx < PC_MFA_SCREEN_X + PC_MFA_SCREEN_W)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, sx, 0, 1, 480, colour);
    }
    for (int y = 768; y < PC_MFA_WORLD_H; y += 768) {
        const int sy = pc_mfa_screen_y(y, camera_y);
        if (sy >= 0 && sy < 480)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, PC_MFA_SCREEN_X, sy,
                               PC_MFA_SCREEN_W, 1, colour);
    }
}

static void pc_mfa_begin_scene(int *camera_x, int *camera_y,
                               uint32_t grid_colour)
{
    pc_mfa_camera(camera_x, camera_y);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       PC_MFA_SCREEN_X, 0, PC_MFA_SCREEN_W, 480,
                       GRAPHICS_RGB(2, 2, 3));
    pc_mfa_draw_page_grid(*camera_x, *camera_y, grid_colour);
}

static void pc_mfa_draw_player(const TextureRle *texture,
                               int camera_x, int camera_y,
                               int width, int height)
{
    PcMfaPoint player = {sSecret.x, sSecret.y};
    pc_mfa_draw_sprite(texture, player, camera_x, camera_y, width, height);
}

static void pc_mfa_draw_bb(void)
{
    int cx, cy;
    pc_mfa_begin_scene(&cx, &cy, GRAPHICS_RGB(5, 20, 7));

    /* The original BB frame has a long lower balloon/glitch route. */
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
    pc_mfa_draw_world_marker(kPcMfaBbHidden, cx, cy, GRAPHICS_RGB(70, 70, 70));
    pc_mfa_draw_player(&gPcSecretBBPlayerTexture, cx, cy, 92, 116);
}

static void pc_mfa_draw_mangle(void)
{
    int cx, cy;
    pc_mfa_begin_scene(&cx, &cy, GRAPHICS_RGB(18, 5, 20));
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
    pc_mfa_draw_world_marker(kPcMfaMangleExit, cx, cy, COLOUR_WHITE);
    pc_mfa_draw_world_marker(kPcMfaMangleAltExit, cx, cy,
                             GRAPHICS_RGB(150, 150, 150));
    pc_mfa_draw_world_marker(kPcMfaMangleKid, cx, cy, GRAPHICS_RGB(64, 184, 219));
    pc_mfa_draw_world_marker(kPcMfaMangleShadow, cx, cy, GRAPHICS_RGB(90, 28, 116));
    pc_mfa_draw_player(&gPcSecretManglePlayerTexture, cx, cy, 92, 96);
}

static void pc_mfa_draw_chica(void)
{
    int cx, cy;
    pc_mfa_begin_scene(&cx, &cy, GRAPHICS_RGB(5, 20, 7));
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
    pc_mfa_draw_world_marker(kPcMfaChicaAltExit, cx, cy,
                             GRAPHICS_RGB(150, 150, 150));
    pc_mfa_draw_player(&gPcSecretChicaPlayerTexture, cx, cy, 88, 112);
}

static void pc_mfa_draw_stage01(void)
{
    int cx, cy;
    pc_mfa_begin_scene(&cx, &cy, GRAPHICS_RGB(19, 9, 27));
    /* Room lattice follows the eight original Active 8 placements. */
    for (int i = 0; i < 8; ++i) {
        const PcMfaPoint p = kPcMfaStageNodes[i];
        pc_mfa_draw_platform(p.x - 180, p.y + 96, 360, 34, cx, cy,
                             GRAPHICS_RGB(107, 47, 119));
        pc_mfa_draw_sprite(&gPcSecretStageChildTexture, p, cx, cy, 82, 90);
    }
    for (int i = 0; i < 8; ++i)
        pc_mfa_draw_world_marker(kPcMfaStageZipUp[i], cx, cy,
                                 GRAPHICS_RGB(80, 137, 204));
    for (int i = 0; i < 2; ++i)
        pc_mfa_draw_world_marker(kPcMfaStageZipRight[i], cx, cy,
                                 GRAPHICS_RGB(204, 137, 80));
    pc_mfa_draw_cake(kPcMfaStageCake, cx, cy, true);
    pc_mfa_draw_world_marker(kPcMfaStageExit, cx, cy, COLOUR_WHITE);
    pc_mfa_draw_player(&gPcSecretStagePlayerTexture, cx, cy, 90, 116);
}

static void pc_mfa_draw_shadow(void)
{
    int cx, cy;
    pc_mfa_begin_scene(&cx, &cy, GRAPHICS_RGB(23, 7, 31));
    static const uint32_t anchor_colours[4] = {
        GRAPHICS_RGB(57, 18, 70), GRAPHICS_RGB(89, 28, 105),
        GRAPHICS_RGB(38, 10, 53), GRAPHICS_RGB(108, 37, 126),
    };
    for (int i = 0; i < 4; ++i) {
        pc_mfa_draw_platform(kPcMfaShadowAnchors[i].x - 170,
                             kPcMfaShadowAnchors[i].y + 88,
                             340, 30, cx, cy, anchor_colours[i]);
        pc_mfa_draw_world_marker(kPcMfaShadowAnchors[i], cx, cy,
                                 anchor_colours[i]);
    }
    for (int i = 0; i < 4; ++i) {
        const PcMfaRect r = kPcMfaShadowBlocks[i];
        pc_mfa_draw_platform(r.x, r.y, r.w, r.h, cx, cy,
                             GRAPHICS_RGB(20, 7, 28));
    }
    pc_mfa_draw_cake(kPcMfaShadowCake, cx, cy, false);
    pc_mfa_draw_world_marker(kPcMfaShadowKid, cx, cy,
                             GRAPHICS_RGB(94, 166, 205));
    pc_mfa_draw_world_marker(kPcMfaShadowExit, cx, cy, COLOUR_WHITE);
    pc_mfa_draw_player(&gPcSecretShadowBonniePlayerTexture, cx, cy, 92, 120);
}

static void pc_mfa_draw_happiest(void)
{
    int cx, cy;
    pc_mfa_begin_scene(&cx, &cy, GRAPHICS_RGB(6, 18, 10));
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
    pc_mfa_draw_world_marker(kPcMfaHappiestTrigger, cx, cy,
                             GRAPHICS_RGB(125, 175, 105));
    pc_mfa_draw_player(&gPcSecretHappiestPlayerTexture, cx, cy, 82, 104);
}

static void pc_mfa_draw_test_hud(void)
{
    /* Tiny coordinate readout: useful for matching hardware screenshots to
     * the MFA editor; deliberately kept outside the 4:3 game viewport. */
    char text[40];
    snprintf(text, sizeof(text), "X%04d", sSecret.x);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 8, 12, 1, text,
                       GRAPHICS_RGB(76, 90, 76));
    snprintf(text, sizeof(text), "Y%04d", sSecret.y);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 8, 30, 1, text,
                       GRAPHICS_RGB(76, 90, 76));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 757, 448, 1, "B:EXIT",
                       GRAPHICS_RGB(92, 108, 92));
}

static bool pc_mfa_secret_render_override(Game *game)
{
    if (!sSecret.active || sSecret.good_ending) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_draw_bb(); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_draw_mangle(); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_draw_chica(); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_draw_stage01(); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_draw_shadow(); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_draw_happiest(); break;
        default: return false;
    }

    pc_mfa_draw_test_hud();
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

static void pc_mfa_move_player(const InputState *input, int speed)
{
    if (input_is_held(input, GAME_BUTTON_LEFT)) sSecret.x -= speed;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) sSecret.x += speed;
    if (input_is_held(input, GAME_BUTTON_UP)) sSecret.y -= speed;
    if (input_is_held(input, GAME_BUTTON_DOWN)) sSecret.y += speed;

    /* A handful of original secret triggers live slightly outside the frame,
     * so keep a small overscan instead of the old one-screen clamp. */
    sSecret.x = pc_mfa_clamp(sSecret.x, -128, 3400);
    sSecret.y = pc_mfa_clamp(sSecret.y, -220, 2340);
}

static void pc_mfa_collect(Game *game, const PcMfaPoint *points,
                           int count, int radius)
{
    if (count > 8) count = 8;
    for (int i = 0; i < count; ++i) {
        const uint8_t bit = (uint8_t)(1u << i);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!secret_near(sSecret.x, sSecret.y, points[i].x, points[i].y,
                         radius)) continue;
        sSecret.collected |= bit;
        content_play_select();
        game->dirty = true;
    }
}

static void pc_mfa_update_bb(Game *game, const InputState *input)
{
    pc_mfa_move_player(input, 4);
    pc_mfa_collect(game, kPcMfaBbCollect, 8, 34);
    if (sSecret.collected == 0xFFu &&
        secret_near(sSecret.x, sSecret.y,
                    kPcMfaBbExit.x, kPcMfaBbExit.y, 48))
        secret_complete(game);
}

static void pc_mfa_update_mangle(Game *game, const InputState *input)
{
    pc_mfa_move_player(input, 4);
    pc_mfa_collect(game, kPcMfaMangleParts, 4, 42);
    if ((sSecret.collected & 0x0Fu) == 0x0Fu &&
        (secret_near(sSecret.x, sSecret.y,
                     kPcMfaMangleExit.x, kPcMfaMangleExit.y, 56) ||
         secret_near(sSecret.x, sSecret.y,
                     kPcMfaMangleAltExit.x, kPcMfaMangleAltExit.y, 56)))
        secret_complete(game);
}

static void pc_mfa_update_chica(Game *game, const InputState *input)
{
    pc_mfa_move_player(input, 4);
    for (int i = 0; i < 4; ++i) {
        const uint8_t pickup = (uint8_t)(1u << i);
        if ((sSecret.collected & pickup) == 0u &&
            secret_near(sSecret.x, sSecret.y,
                        kPcMfaChicaCupcakes[i].x,
                        kPcMfaChicaCupcakes[i].y, 42)) {
            sSecret.collected |= pickup;
            ++sSecret.carried;
            content_play_select();
        }
        const uint8_t delivered = (uint8_t)(1u << (i + 4));
        if (sSecret.carried > 0 && (sSecret.collected & delivered) == 0u &&
            secret_near(sSecret.x, sSecret.y,
                        kPcMfaChicaChildren[i].x,
                        kPcMfaChicaChildren[i].y, 52)) {
            sSecret.collected |= delivered;
            --sSecret.carried;
            ++sSecret.delivered;
            audio_play(AUDIO_CUE_END, 0.42f, false);
        }
    }
    if (sSecret.delivered >= 4 &&
        secret_near(sSecret.x, sSecret.y,
                    kPcMfaChicaExit.x, kPcMfaChicaExit.y, 58))
        secret_complete(game);
}

static void pc_mfa_update_stage01(Game *game, const InputState *input)
{
    pc_mfa_move_player(input, 4);
    pc_mfa_collect(game, kPcMfaStageGoalNodes, 3, 52);
    if ((sSecret.collected & 0x07u) == 0x07u &&
        secret_near(sSecret.x, sSecret.y,
                    kPcMfaStageCake.x, kPcMfaStageCake.y, 60))
        secret_complete(game);
}

static void pc_mfa_update_shadow(Game *game, const InputState *input)
{
    pc_mfa_move_player(input, 5);

    /* Keep the Wii U phase button but move through actual MFA scene anchors. */
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) % 4;
        sSecret.x = kPcMfaShadowAnchors[sSecret.room].x;
        sSecret.y = kPcMfaShadowAnchors[sSecret.room].y;
        content_play_select();
    }
    const uint8_t bit = (uint8_t)(1u << sSecret.room);
    if ((sSecret.collected & bit) == 0u &&
        secret_near(sSecret.x, sSecret.y,
                    kPcMfaShadowAnchors[sSecret.room].x,
                    kPcMfaShadowAnchors[sSecret.room].y, 60)) {
        sSecret.collected |= bit;
        content_play_select();
    }
    if ((sSecret.collected & 0x0Fu) == 0x0Fu &&
        (secret_near(sSecret.x, sSecret.y,
                     kPcMfaShadowKid.x, kPcMfaShadowKid.y, 64) ||
         secret_near(sSecret.x, sSecret.y,
                     kPcMfaShadowCake.x, kPcMfaShadowCake.y, 64)))
        secret_complete(game);
}

static void pc_mfa_update_happiest(Game *game, const InputState *input)
{
    pc_mfa_move_player(input, 4);
    for (int i = 0; i < 4; ++i) {
        const uint8_t bit = (uint8_t)(1u << i);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!secret_near(sSecret.x, sSecret.y,
                         kPcMfaHappiestChildren[i].x,
                         kPcMfaHappiestChildren[i].y, 54)) continue;
        sSecret.collected |= bit;
        ++sSecret.delivered;
        audio_play(AUDIO_CUE_END, 0.42f, false);
    }
    if (sSecret.delivered >= 4 &&
        (secret_near(sSecret.x, sSecret.y,
                     kPcMfaHappiestTrigger.x,
                     kPcMfaHappiestTrigger.y, 64) ||
         secret_near(sSecret.x, sSecret.y,
                     kPcMfaHappiestCakes[3].x,
                     kPcMfaHappiestCakes[3].y, 64)))
        secret_complete(game);
}

static void pc_mfa_secret_update_active(Game *game, const InputState *input)
{
    if (sSecret.frames == 0u) {
        const PcMfaPoint start = kPcMfaStart[sSecret.kind];
        sSecret.x = start.x;
        sSecret.y = start.y;
        sSecret.room = 0;
        game->dirty = true;
    }

    ++sSecret.frames;
    ++game->screen_frames;

    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending) {
        secret_return_to_menu(game);
        return;
    }

    if (sSecret.completed) {
        ++sSecret.completed_frames;
        game->dirty = true;
        if (sSecret.completed_frames >= 150u &&
            (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
             input_was_pressed(input, GAME_BUTTON_START) ||
             input_was_pressed(input, GAME_BUTTON_BACK)))
            secret_return_to_menu(game);
        return;
    }

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: pc_mfa_update_bb(game, input); break;
        case SECRET_MINIGAME_MANGLE: pc_mfa_update_mangle(game, input); break;
        case SECRET_MINIGAME_CHICA: pc_mfa_update_chica(game, input); break;
        case SECRET_MINIGAME_STAGE01: pc_mfa_update_stage01(game, input); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: pc_mfa_update_shadow(game, input); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: pc_mfa_update_happiest(game, input); break;
        default: break;
    }
    game->dirty = true;
}

static void pc_mfa_exact_update_game(Game *game, const InputState *input)
{
    if (sSecret.active && !sSecret.good_ending) {
        pc_mfa_secret_update_active(game, input);
        return;
    }
    update_game(game, input);
}
