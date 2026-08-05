static void complete_draw_title(const Game *game)
{
    static const char *const items[TITLE_ITEM_COUNT] = {
        "NEW GAME", "LOAD GAME", "EXTRAS"
    };
    const int frame = title_frame_index(game);
    const int glitch = frame == 0
        ? (int) ((game->seed >> 8) % 7u) - 3
        : (frame - 2) * 3;

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 410, 480,
                       GRAPHICS_RGB(2, 7, 4));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 47, 7, "F", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 119, 7, "N", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 191, 7, "A", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 263, 7, "F", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 349, 8, "3", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 120, 72, 2,
                       "FIVE NIGHTS AT FREDDYS 3", COLOUR_GREEN);

    char continue_text[32];
    snprintf(continue_text, sizeof(continue_text),
             "CURRENT NIGHT: %d", game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 119, 1,
                       continue_text, GRAPHICS_RGB(74, 111, 73));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 145, 1,
                       progress_save_load_status_text(),
                       progress_save_load_status_is_error()
                           ? COLOUR_RED : GRAPHICS_RGB(74, 111, 73));

    const uint32_t star = GRAPHICS_RGB(225, 219, 103);
    if (progress_save_is_night_completed(5))
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 304, 40, 4, "*", star);
    if (progress_save_is_night_completed(6))
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 336, 40, 4, "*", star);
    if (progress_save_good_ending_unlocked())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 368, 40, 4, "*", star);

    for (int item = 0; item < TITLE_ITEM_COUNT; ++item) {
        const int y = 284 + item * 48;
        if (item == game->title_selection) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 262, 36,
                               GRAPHICS_RGB(31, 72, 37));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 5, 36,
                               GRAPHICS_RGB(129, 207, 121));
        }
        uint32_t colour = item == game->title_selection
            ? COLOUR_WHITE : GRAPHICS_RGB(112, 139, 113);
        if (item == 2 && !progress_save_extras_unlocked())
            colour = GRAPHICS_RGB(70, 78, 70);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 124, y, 3,
                           items[item], colour);
        if (item == 2 && !progress_save_extras_unlocked())
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 300, y + 5, 1,
                               "LOCKED", GRAPHICS_RGB(91, 67, 62));
    }

    if (sFinishing.title_notice_frames > 0u)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 112, 445, 1,
                           "COMPLETE NIGHT 5 TO UNLOCK EXTRAS", COLOUR_RED);

    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 410, 0, 444, 480, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 410 + glitch, 16, 444, 448,
                     kTitleFrames[frame]);
    draw_scanlines(GRAPHICS_TARGET_BOTH,
                   game->seed + game->screen_frames);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void fnaf3_complete_content_render_game(Game *game)
{
    if (game->screen == SCREEN_TITLE) {
        game->dirty = false;
        complete_draw_title(game);
        return;
    }
    fnaf3_content_render_game(game);
}
