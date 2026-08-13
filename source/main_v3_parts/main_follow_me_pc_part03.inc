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

static void follow_me_pc_draw_checker_mfa(void)
{
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 107, 0,
                     (int) gFollowMfaChecker.texture.width,
                     (int) gFollowMfaChecker.texture.height,
                     &gFollowMfaChecker.texture);
}

static void follow_me_pc_draw_room(const Game *game)
{
    (void) game;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);

    /* Exact PC image-bank ID 335. This removes the procedural checker and the
     * pseudo-random wall damage from the previous reconstruction. */
    follow_me_pc_draw_checker_mfa();

    if (sFollowMePc.room == FOLLOW_ROOM_SHOW_STAGE) {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_CURTAIN, 72, 66, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_CURTAIN, 676, 66, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_STAGE, 178, 220, 3);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_POSTERS, 555, 92, 1);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 320, 420, 220, 60,
                           GRAPHICS_RGB(3, 3, 5));
    } else if (sFollowMePc.room == FOLLOW_ROOM_DINING) {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_CLEAN, 82, 128, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_SPIKY_A, 346, 128, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_SPIKY_B, 610, 128, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_STAINED, 214, 282, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_FADED, 486, 282, 2);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 165, 38, 195,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 816, 165, 38, 195,
                           GRAPHICS_RGB(3, 3, 5));
    } else if (sFollowMePc.room == FOLLOW_ROOM_WEST_HALL) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 165, 40, 205,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 815, 165, 39, 205,
                           GRAPHICS_RGB(3, 3, 5));
        follow_me_pc_draw_env(FOLLOW_ME_ENV_POSTERS, 94, 82, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DESK_FAN, 336, 218, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_ARCADE, 670, 246, 2);
    } else if (sFollowMePc.room == FOLLOW_ROOM_EAST_HALL) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 165, 40, 205,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 815, 165, 39, 205,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 560, 64, 200, 42,
                           GRAPHICS_RGB(3, 3, 5));
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_FADED, 92, 122, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_PARTY_TABLE, 610, 260, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_KEYPAD, 742, 84, 2);

        if (sFollowMePc.chapter == 1) {
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 116, 235, 1,
                               "BBDBLCLICK", GRAPHICS_RGB(154, 154, 159));
        } else if (sFollowMePc.chapter == 2) {
            for (int i = 0; i < 4; ++i) {
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 116 + i * 42, 235,
                                   24, 18, GRAPHICS_RGB(130, 130, 134));
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 121 + i * 42, 227,
                                   14, 9, GRAPHICS_RGB(185, 185, 188));
            }
        } else if (sFollowMePc.chapter == 3) {
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 116, 238, 2,
                               "395248", GRAPHICS_RGB(164, 164, 168));
        } else if (sFollowMePc.chapter == 4) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 151, 228, 28, 44,
                               GRAPHICS_RGB(18, 5, 24));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 155, 216, 7, 15,
                               GRAPHICS_RGB(18, 5, 24));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 169, 216, 7, 15,
                               GRAPHICS_RGB(18, 5, 24));
        }
    } else if (sFollowMePc.room == FOLLOW_ROOM_PIRATE_COVE) {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_CURTAIN, 82, 70, 3);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_POSTERS, 320, 100, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_ARCADE, 650, 245, 2);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 815, 165, 39, 230,
                           GRAPHICS_RGB(3, 3, 5));
    } else if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE) {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_PARTY_TABLE, 165, 250, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DESK_FAN, 392, 216, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DOOR_WALL, 685, 64, 2);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 610, 64, 170, 42,
                           GRAPHICS_RGB(2, 2, 4));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 606, 60, 178, 50, 3,
                            GRAPHICS_RGB(83, 80, 85));
    } else {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_ARCADE, 112, 278, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DESK_FAN, 274, 92, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_STAINED, 380, 94, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_KEYPAD, 718, 82, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DOOR_WALL, 752, 250, 2);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 352, 420, 150, 60,
                           GRAPHICS_RGB(2, 2, 4));
    }
}
