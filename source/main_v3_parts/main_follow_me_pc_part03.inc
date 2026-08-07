static void follow_me_pc_draw_decay(uint32_t seed)
{
    for (int index = 0; index < 18; ++index) {
        seed = seed * 1664525u + 1013904223u;
        const int x = (int) ((seed >> 8) % 810u);
        seed = seed * 1664525u + 1013904223u;
        const int y = 70 + (int) ((seed >> 8) % 330u);
        const int w = 12 + (int) ((seed >> 20) % 45u);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, w, 6,
                           GRAPHICS_RGB(10, 10, 12));
    }
}

static void follow_me_pc_draw_room(const Game *game)
{
    (void) game;
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(8, 7, 11));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 64,
                       GRAPHICS_RGB(26, 23, 31));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 64, 854, 230,
                       GRAPHICS_RGB(60, 59, 66));
    follow_me_pc_draw_checker_floor(294);
    /* Room damage is static like the PC minigame, not animated noise. */
    follow_me_pc_draw_decay(0x91E10DA5u + (uint32_t) sFollowMePc.room * 173u);

    if (sFollowMePc.room == FOLLOW_ROOM_SHOW_STAGE) {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_CURTAIN, 72, 66, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_CURTAIN, 676, 66, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_STAGE, 178, 220, 3);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_POSTERS, 555, 92, 1);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 320, 420, 220, 60,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 330, 25, 2,
                           "SHOW STAGE", GRAPHICS_RGB(148, 148, 153));
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
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 325, 25, 2,
                           "DINING AREA", GRAPHICS_RGB(148, 148, 153));
    } else if (sFollowMePc.room == FOLLOW_ROOM_WEST_HALL) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 165, 40, 205,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 815, 165, 39, 205,
                           GRAPHICS_RGB(3, 3, 5));
        follow_me_pc_draw_env(FOLLOW_ME_ENV_POSTERS, 94, 82, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DESK_FAN, 336, 218, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_ARCADE, 670, 246, 2);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 328, 25, 2,
                           "WEST HALL", GRAPHICS_RGB(148, 148, 153));
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

        /* Keep the original post-night secret clues visible in the hall. */
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
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 328, 25, 2,
                           "EAST HALL", GRAPHICS_RGB(148, 148, 153));
    } else if (sFollowMePc.room == FOLLOW_ROOM_PIRATE_COVE) {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_CURTAIN, 82, 70, 3);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_POSTERS, 320, 100, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_ARCADE, 650, 245, 2);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 815, 165, 39, 230,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 320, 25, 2,
                           "PIRATE COVE", GRAPHICS_RGB(148, 148, 153));
    } else if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE) {
        follow_me_pc_draw_env(FOLLOW_ME_ENV_PARTY_TABLE, 165, 250, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DESK_FAN, 392, 216, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DOOR_WALL, 685, 64, 2);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 610, 64, 170, 42,
                           GRAPHICS_RGB(2, 2, 4));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 606, 60, 178, 50, 3,
                            GRAPHICS_RGB(83, 80, 85));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 266, 25, 2,
                           "SAFE ROOM ENTRANCE", GRAPHICS_RGB(148, 148, 153));
    } else {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 335, 25, 2,
                           "SAFE ROOM", GRAPHICS_RGB(148, 148, 153));
        follow_me_pc_draw_env(FOLLOW_ME_ENV_ARCADE, 112, 278, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DESK_FAN, 274, 92, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_TABLE_STAINED, 380, 94, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_KEYPAD, 718, 82, 2);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_DOOR_WALL, 752, 250, 2);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 352, 420, 150, 60,
                           GRAPHICS_RGB(2, 2, 4));
    }
}
