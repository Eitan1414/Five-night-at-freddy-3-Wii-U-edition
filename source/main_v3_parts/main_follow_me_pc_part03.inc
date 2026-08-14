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
    /* Exact Make Visible events 52..60 from fivenights3-94.mfa. */
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
    /* Events 34..38 set party-table alterable value 1..5; events 64..68
     * map those values to the original animation IDs 0, 12, 13, 14 and 15. */
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
    /* Backdrop item 300: four 310x88 strips using PC image 31. */
    static const int wide_positions[4][2] = {
        {-2, -8}, {-2, 680}, {714, -8}, {716, 682}
    };
    /* Backdrop item 301: four 102x134 strips using PC image 42. */
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
    /* These visibility cells come directly from events 9, 16..19 and 156.
     * Blood objects are layer 0 in the original frame. */
    if (follow_me_pc_mfa_cell(3, 4) ||
        follow_me_pc_mfa_cell(4, 2) ||
        follow_me_pc_mfa_cell(1, 5)) {
        follow_me_pc_draw_mfa(&gFollowMfaBlood, 316, 308); /* blood 1 */
    }

    if (follow_me_pc_mfa_cell(1, 5)) {
        follow_me_pc_draw_mfa(&gFollowMfaBlood, 316, 468); /* blood 3 */
    }

    if (follow_me_pc_mfa_cell(3, 3) ||
        follow_me_pc_mfa_cell(3, 4) ||
        follow_me_pc_mfa_cell(3, 5) ||
        follow_me_pc_mfa_cell(1, 5)) {
        follow_me_pc_draw_mfa(&gFollowMfaBlood, 711, 468); /* blood 4 */
    }
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
    /* Show-stage objects: events 40, 41, 47 and 48. */
    if (follow_me_pc_mfa_cell(2, 3)) {
        follow_me_pc_draw_mfa(&gFollowMfaStage, 495, 393);
        follow_me_pc_draw_mfa(&gFollowMfaChicaDecor, 654, 200);
        follow_me_pc_draw_mfa(&gFollowMfaBonnieDecor, 342, 202);
    }

    /* Curtain is visible in the show-stage cell and explicitly re-enabled by
     * event 72 in cell (3,2). */
    if (follow_me_pc_mfa_cell(2, 3) ||
        follow_me_pc_mfa_cell(3, 2)) {
        follow_me_pc_draw_mfa(&gFollowMfaCurtain, 28, 370);
    }

    /* trash1: exact Make Visible events 49..51. */
    if (follow_me_pc_mfa_cell(2, 3) ||
        follow_me_pc_mfa_cell(4, 2) ||
        follow_me_pc_mfa_cell(2, 4)) {
        follow_me_pc_draw_mfa(&gFollowMfaTrash, 492, 362);
    }

    follow_me_pc_draw_party_tables_mfa();

    /* hallway: two exact instances, enabled in events 20 and 21. */
    if (follow_me_pc_mfa_cell(4, 2) ||
        follow_me_pc_mfa_cell(4, 4)) {
        follow_me_pc_draw_mfa(&gFollowMfaHallway, 214, -9);
        follow_me_pc_draw_mfa(&gFollowMfaHallway, 716, -3);
    }

    /* arcade: three exact instances, enabled with the room-1/5 set in event
     * 156. */
    if (follow_me_pc_mfa_cell(1, 5)) {
        follow_me_pc_draw_mfa(&gFollowMfaArcade, 557, 208);
        follow_me_pc_draw_mfa(&gFollowMfaArcade, 381, 210);
        follow_me_pc_draw_mfa(&gFollowMfaArcade, 211, 214);
    }

    /* blocked: exact events 78 and 79. */
    if (follow_me_pc_mfa_cell(2, 5) ||
        follow_me_pc_mfa_cell(3, 5)) {
        follow_me_pc_draw_mfa(&gFollowMfaBlocked, 958, 352);
    }
}

static void follow_me_pc_draw_secret_clue_compat(void)
{
    /* The real clue objects are MFA handles 373..384 and are only enabled in
     * cell (4,2), selected by the cutscene counter (events 186..189). Their
     * individual image-bank frames are being extracted in the next pass.
     * Keep the previous compatibility representation confined to the correct
     * source room meanwhile. */
    if (!follow_me_pc_mfa_cell(4, 2)) return;

    if (sFollowMePc.chapter == 1) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           follow_me_pc_mfa_x(134),
                           follow_me_pc_mfa_y(126),
                           1, "BBDBLCLICK",
                           GRAPHICS_RGB(154, 154, 159));
    } else if (sFollowMePc.chapter == 2) {
        for (int index = 0; index < 4; ++index) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                               follow_me_pc_mfa_x(150),
                               follow_me_pc_mfa_y(187 + index * 114),
                               15, 11, GRAPHICS_RGB(130, 130, 134));
        }
    } else if (sFollowMePc.chapter == 3) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           follow_me_pc_mfa_x(137),
                           follow_me_pc_mfa_y(204),
                           2, "395248",
                           GRAPHICS_RGB(164, 164, 168));
    } else if (sFollowMePc.chapter == 4) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           follow_me_pc_mfa_x(864),
                           follow_me_pc_mfa_y(352),
                           18, 28, GRAPHICS_RGB(18, 5, 24));
    }
}

static void follow_me_pc_draw_room(const Game *game)
{
    (void) game;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);

    /* Layer 0 is now driven by the original room-grid visibility events rather
     * than the seven-room reconstruction. */
    follow_me_pc_draw_checker_mfa();
    follow_me_pc_draw_environment_blood_mfa();

    /* The eight backdrop instances are static frame objects. */
    follow_me_pc_draw_backdrops_mfa();

    /* Layer 1 object visibility comes from the original cutscenes event sheet. */
    follow_me_pc_draw_layer1_room_mfa();

    /* Temporary only until handles 373..384 are converted from the image bank. */
    follow_me_pc_draw_secret_clue_compat();
}
