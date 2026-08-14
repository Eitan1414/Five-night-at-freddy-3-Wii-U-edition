/* PC/MFA coordinate helpers. Original Follow Me is a 1024x768 Clickteam frame.
 * Wii U preserves it as a centered 640x480 image instead of stretching 16:9. */
static int follow_me_pc_mfa_scale(int value)
{
    if (value >= 0) return (value * 5 + 4) / 8;
    return -(((-value) * 5 + 4) / 8);
}

static int follow_me_pc_mfa_x(int pc_x)
{
    return 107 + follow_me_pc_mfa_scale(pc_x);
}

static int follow_me_pc_mfa_y(int pc_y)
{
    return follow_me_pc_mfa_scale(pc_y);
}

static void follow_me_pc_draw_mfa(const FollowMeMfaTexture *asset,
                                  int pc_x, int pc_y)
{
    if (asset == NULL || asset->texture.width == 0u ||
        asset->texture.height == 0u) return;
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     follow_me_pc_mfa_x(pc_x) - asset->hotspot_x,
                     follow_me_pc_mfa_y(pc_y) - asset->hotspot_y,
                     (int) asset->texture.width,
                     (int) asset->texture.height,
                     &asset->texture);
}

static bool follow_me_pc_mfa_cell(int vertical, int horizontal)
{
    return sFollowMfaVertical == vertical &&
           sFollowMfaHorizontal == horizontal;
}

static bool follow_me_pc_mfa_checker_visible(void)
{
    return follow_me_pc_mfa_cell(3, 2) ||
           follow_me_pc_mfa_cell(4, 2) ||
           follow_me_pc_mfa_cell(2, 4) ||
           follow_me_pc_mfa_cell(3, 4) ||
           follow_me_pc_mfa_cell(2, 3) ||
           follow_me_pc_mfa_cell(2, 2) ||
           follow_me_pc_mfa_cell(3, 5) ||
           follow_me_pc_mfa_cell(1, 5) ||
           follow_me_pc_mfa_cell(5, 3);
}

static const FollowMeMfaTexture *follow_me_pc_mfa_party_table(void)
{
    if (follow_me_pc_mfa_cell(2, 2)) return &gFollowMfaPartyTable0;
    if (follow_me_pc_mfa_cell(2, 4)) return &gFollowMfaPartyTable12;
    if (follow_me_pc_mfa_cell(3, 2)) return &gFollowMfaPartyTable13;
    if (follow_me_pc_mfa_cell(3, 3)) return &gFollowMfaPartyTable14;
    if (follow_me_pc_mfa_cell(3, 4)) return &gFollowMfaPartyTable15;
    return NULL;
}

static void follow_me_pc_draw_checker_mfa(void)
{
    if (!follow_me_pc_mfa_checker_visible()) return;
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 107, 0,
                     (int) gFollowMfaChecker.texture.width,
                     (int) gFollowMfaChecker.texture.height,
                     &gFollowMfaChecker.texture);
}

static void follow_me_pc_draw_backdrops_mfa(void)
{
    static const int wide_positions[4][2] = {
        {-2, -8}, {-2, 680}, {714, -8}, {716, 682}
    };
    static const int tall_positions[4][2] = {
        {-2, 80}, {-2, 550}, {926, 79}, {926, 550}
    };

    for (int index = 0; index < 4; ++index) {
        follow_me_pc_draw_mfa(&gFollowMfaBackdropWide,
                              wide_positions[index][0],
                              wide_positions[index][1]);
        follow_me_pc_draw_mfa(&gFollowMfaBackdropTall,
                              tall_positions[index][0],
                              tall_positions[index][1]);
    }
}

static void follow_me_pc_draw_environment_blood_mfa(void)
{
    if (follow_me_pc_mfa_cell(3, 4) ||
        follow_me_pc_mfa_cell(4, 2) ||
        follow_me_pc_mfa_cell(1, 5))
        follow_me_pc_draw_mfa(&gFollowMfaBlood, 316, 308);

    if (follow_me_pc_mfa_cell(1, 5))
        follow_me_pc_draw_mfa(&gFollowMfaBlood, 316, 468);

    if (follow_me_pc_mfa_cell(3, 3) ||
        follow_me_pc_mfa_cell(3, 4) ||
        follow_me_pc_mfa_cell(3, 5) ||
        follow_me_pc_mfa_cell(1, 5))
        follow_me_pc_draw_mfa(&gFollowMfaBlood, 711, 468);
}

static void follow_me_pc_draw_party_tables_mfa(void)
{
    const FollowMeMfaTexture *table = follow_me_pc_mfa_party_table();
    if (table == NULL) return;

    static const int positions[4][2] = {
        {256, 185}, {763, 185}, {256, 536}, {763, 536}
    };
    for (int index = 0; index < 4; ++index)
        follow_me_pc_draw_mfa(table, positions[index][0], positions[index][1]);
}

static void follow_me_pc_draw_layer1_room_mfa(void)
{
    if (follow_me_pc_mfa_cell(2, 3)) {
        follow_me_pc_draw_mfa(&gFollowMfaStage, 495, 393);
        follow_me_pc_draw_mfa(&gFollowMfaChicaDecor, 654, 200);
        follow_me_pc_draw_mfa(&gFollowMfaBonnieDecor, 342, 202);
    }

    if (follow_me_pc_mfa_cell(2, 3) ||
        follow_me_pc_mfa_cell(3, 2))
        follow_me_pc_draw_mfa(&gFollowMfaCurtain, 28, 370);

    if (follow_me_pc_mfa_cell(2, 3) ||
        follow_me_pc_mfa_cell(4, 2) ||
        follow_me_pc_mfa_cell(2, 4))
        follow_me_pc_draw_mfa(&gFollowMfaTrash, 492, 362);

    follow_me_pc_draw_party_tables_mfa();

    if (follow_me_pc_mfa_cell(4, 2) ||
        follow_me_pc_mfa_cell(4, 4)) {
        follow_me_pc_draw_mfa(&gFollowMfaHallway, 214, -9);
        follow_me_pc_draw_mfa(&gFollowMfaHallway, 716, -3);
    }

    /* Event 40: after the `new frame` reset, table fan handle 324 is created
     * only for vertical=5,horizontal=3. It is created relative to leave-top
     * (499,-1) with offset (+1,+185), giving exact source (500,184).
     * Its only animation frame is image 326, hotspot (175,151). */
    if (follow_me_pc_mfa_cell(5, 3))
        follow_me_pc_draw_mfa(&gFollowMfaTableFan, 500, 184);

    /* Event 63: gift handle 331 exists only for vertical=2,horizontal=1 and is
     * created relative to leave-top with (-101,+343), source (398,342).
     * Its exact frame is image 340, hotspot (223,189). */
    if (follow_me_pc_mfa_cell(2, 1))
        follow_me_pc_draw_mfa(&gFollowMfaGift, 398, 342);

    if (follow_me_pc_mfa_cell(1, 5)) {
        follow_me_pc_draw_mfa(&gFollowMfaArcade, 557, 208);
        follow_me_pc_draw_mfa(&gFollowMfaArcade, 381, 210);
        follow_me_pc_draw_mfa(&gFollowMfaArcade, 211, 214);
    }

    if (follow_me_pc_mfa_cell(2, 5) ||
        follow_me_pc_mfa_cell(3, 5))
        follow_me_pc_draw_mfa(&gFollowMfaBlocked, 958, 352);
}

static void follow_me_pc_draw_secret_clues_mfa(void)
{
    /* Events 186..189 enable these only in source cell (4,2). The image IDs,
     * hotspots and transparency keys below are generated from the actual MFA
     * image-bank metadata, so there is no reconstructed text/geometry left. */
    if (!follow_me_pc_mfa_cell(4, 2)) return;

    if (sFollowMePc.chapter == 1) {
        follow_me_pc_draw_mfa(&gFollowMfaClueBBDouble, 134, 126); /* 373 */
        follow_me_pc_draw_mfa(&gFollowMfaClue6, 863, 229);        /* 374 */
        follow_me_pc_draw_mfa(&gFollowMfaClue7, 863, 324);        /* 375 */
        follow_me_pc_draw_mfa(&gFollowMfaClue8, 863, 419);        /* 376 */
        follow_me_pc_draw_mfa(&gFollowMfaClue9, 863, 514);        /* 377 */
    } else if (sFollowMePc.chapter == 2) {
        follow_me_pc_draw_mfa(&gFollowMfaClueButtons, 150, 187);  /* 378 */
        follow_me_pc_draw_mfa(&gFollowMfaClueButtons, 150, 301);  /* 379 */
        follow_me_pc_draw_mfa(&gFollowMfaClueButtons, 150, 415);  /* 380 */
        follow_me_pc_draw_mfa(&gFollowMfaClueButtons, 150, 529);  /* 381 */
    } else if (sFollowMePc.chapter == 3) {
        follow_me_pc_draw_mfa(&gFollowMfaClue14, 137, 204);       /* 382 */
    } else if (sFollowMePc.chapter == 4) {
        follow_me_pc_draw_mfa(&gFollowMfaClue15, -194, 208);      /* 383 */
        follow_me_pc_draw_mfa(&gFollowMfaClue16, 864, 352);       /* 384 */
    }
}

static void follow_me_pc_draw_room(const Game *game)
{
    (void) game;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);

    follow_me_pc_draw_checker_mfa();
    follow_me_pc_draw_environment_blood_mfa();
    follow_me_pc_draw_backdrops_mfa();
    follow_me_pc_draw_layer1_room_mfa();
    follow_me_pc_draw_secret_clues_mfa();
}
