static void complete_draw_title(const Game *game)
{
    static const char *const items[TITLE_ITEM_COUNT] = {
        "NEW GAME", "LOAD GAME", "EXTRA"
    };
    const int frame = title_frame_index(game);
    const int glitch = frame == 0
        ? (int)((game->seed >> 8) % 7u) - 3
        : (frame - 2) * 3;

    /* PC title: black field, stacked title at left, Springtrap emerging from
     * darkness at right and simple text selection.  Do not use the previous
     * Wii U green cards/filled selection rectangles. */
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);

    /* Subtle green horizontal interference, kept behind every title object. */
    for (int y = 4; y < GRAPHICS_LOGICAL_HEIGHT; y += 7) {
        const uint32_t phase =
            (game->screen_frames + (uint32_t)y * 3u) % 31u;
        const uint32_t shade = phase < 3u ? 18u : 7u;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           0, y, GRAPHICS_LOGICAL_WIDTH, 1,
                           GRAPHICS_RGB(2, shade, 4));
    }

    /* The five PC title animation images are generated from the MFA's
     * original Active 2 image IDs. */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     398 + glitch, -12, 472, 516,
                     kTitleFrames[frame]);

    const uint32_t title_main = GRAPHICS_RGB(196, 207, 194);
    const uint32_t title_dim = GRAPHICS_RGB(102, 115, 100);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 52, 42, 4,
                       "FIVE", title_main);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 52, 82, 4,
                       "NIGHTS", title_main);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 52, 122, 3,
                       "AT", title_dim);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 52, 156, 4,
                       "FREDDY'S", title_main);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 52, 203, 5,
                       "3", GRAPHICS_RGB(107, 149, 98));

    for (int item = 0; item < TITLE_ITEM_COUNT; ++item) {
        const int y = 300 + item * 42;
        const bool selected = item == game->title_selection;
        uint32_t colour = selected
            ? COLOUR_WHITE : GRAPHICS_RGB(106, 116, 104);
        if (item == 2 && !progress_save_extras_unlocked())
            colour = selected
                ? GRAPHICS_RGB(143, 143, 137)
                : GRAPHICS_RGB(68, 72, 67);

        if (selected) {
            graphics_draw_text(GRAPHICS_TARGET_BOTH,
                               48, y, 2, ">>", COLOUR_WHITE);
        }
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           91, y, 3, items[item], colour);
    }

    char continue_text[16];
    snprintf(continue_text, sizeof(continue_text),
             "NIGHT %d", game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       274, 344, 1,
                       continue_text,
                       GRAPHICS_RGB(104, 118, 103));

    const uint32_t star = GRAPHICS_RGB(216, 216, 202);
    int star_x = 305;
    if (progress_save_is_night_completed(5)) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, star_x, 42, 3, "*", star);
        star_x += 28;
    }
    if (progress_save_is_night_completed(6)) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, star_x, 42, 3, "*", star);
        star_x += 28;
    }
    if (progress_save_good_ending_unlocked()) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, star_x, 42, 3, "*", star);
        star_x += 28;
    }
    if (progress_save_aggressive_nightmare_completed())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, star_x, 42, 3, "*", star);

    if (sFinishing.title_notice_frames > 0u) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           51, 434, 1,
                           "COMPLETE NIGHT 5 TO UNLOCK EXTRA",
                           GRAPHICS_RGB(145, 145, 137));
    }

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
