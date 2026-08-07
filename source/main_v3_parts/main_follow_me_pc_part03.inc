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
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(8, 7, 11));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 64,
                       GRAPHICS_RGB(26, 23, 31));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 64, 854, 230,
                       GRAPHICS_RGB(60, 59, 66));
    follow_me_pc_draw_checker_floor(294);
    follow_me_pc_draw_decay(game->screen_frames +
                            (uint32_t) sFollowMePc.room * 173u);

    if (sFollowMePc.room == FOLLOW_ROOM_SHOW_STAGE) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 145, 120, 565, 150,
                           GRAPHICS_RGB(28, 25, 31));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 145, 270, 565, 20,
                           GRAPHICS_RGB(90, 82, 88));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 320, 420, 220, 60,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 330, 25, 2,
                           "SHOW STAGE", GRAPHICS_RGB(148, 148, 153));
    } else if (sFollowMePc.room == FOLLOW_ROOM_DINING) {
        for (int row = 0; row < 2; ++row) {
            for (int col = 0; col < 3; ++col) {
                const int x = 130 + col * 230;
                const int y = 160 + row * 145;
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 112, 28,
                                   GRAPHICS_RGB(85, 80, 87));
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + 12, y + 28, 12, 32,
                                   GRAPHICS_RGB(47, 44, 50));
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + 88, y + 28, 12, 32,
                                   GRAPHICS_RGB(47, 44, 50));
            }
        }
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 815, 165, 39, 170,
                           GRAPHICS_RGB(3, 3, 5));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 325, 25, 2,
                           "DINING AREA", GRAPHICS_RGB(148, 148, 153));
    } else if (sFollowMePc.room == FOLLOW_ROOM_HALL) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 170, 90, 170,
                           GRAPHICS_RGB(4, 4, 6));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 560, 64, 200, 42,
                           GRAPHICS_RGB(3, 3, 5));
        /* The original post-night screens hint at the secret minigames here. */
        if (sFollowMePc.chapter == 1) {
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 116, 122, 1,
                               "BBDBLCLICK", GRAPHICS_RGB(154, 154, 159));
            for (int i = 0; i < 4; ++i)
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 116 + i * 34, 150,
                                   22, 10,
                                   i == 2 ? GRAPHICS_RGB(215, 193, 56)
                                          : GRAPHICS_RGB(110, 110, 115));
        } else if (sFollowMePc.chapter == 2) {
            for (int i = 0; i < 4; ++i) {
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 116 + i * 42, 140,
                                   24, 18, GRAPHICS_RGB(130, 130, 134));
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 121 + i * 42, 132,
                                   14, 9, GRAPHICS_RGB(185, 185, 188));
            }
        } else if (sFollowMePc.chapter == 3) {
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 116, 142, 2,
                               "395248", GRAPHICS_RGB(164, 164, 168));
        } else if (sFollowMePc.chapter == 4) {
            /* Placeholder silhouette until the dedicated Shadow Bonnie hint
             * frame is mapped from the MFA. Do not treat this as final art. */
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 151, 142, 28, 44,
                               GRAPHICS_RGB(18, 5, 24));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 155, 130, 7, 15,
                               GRAPHICS_RGB(18, 5, 24));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 169, 130, 7, 15,
                               GRAPHICS_RGB(18, 5, 24));
        }
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 330, 25, 2,
                           "BACK HALL", GRAPHICS_RGB(148, 148, 153));
    } else if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 610, 64, 170, 42,
                           GRAPHICS_RGB(2, 2, 4));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 606, 60, 178, 50, 3,
                            GRAPHICS_RGB(83, 80, 85));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 266, 25, 2,
                           "SAFE ROOM ENTRANCE", GRAPHICS_RGB(148, 148, 153));
    } else {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 335, 25, 2,
                           "SAFE ROOM", GRAPHICS_RGB(148, 148, 153));
        for (int index = 0; index < 3; ++index) {
            const int x = 270 + index * 120;
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 92, 82, 105,
                               GRAPHICS_RGB(54, 54, 57));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + 14, 110, 54, 45,
                               GRAPHICS_RGB(57, 142, 203));
        }
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 352, 420, 150, 60,
                           GRAPHICS_RGB(2, 2, 4));
    }
}
