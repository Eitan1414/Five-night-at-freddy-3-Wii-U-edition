static void complete_draw_title(const Game *game)
{
    const int frame = title_frame_index(game);
    const int glitch = frame == 0
        ? (int)((game->seed >> 8) % 7u) - 3
        : (frame - 2) * 3;

    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);

    /* Original PC title treatment: thin olive/green horizontal scan bars. */
    for (int y = 18; y < GRAPHICS_LOGICAL_HEIGHT; y += 39) {
        const uint32_t flicker =
            (game->screen_frames + (uint32_t)y) % 17u;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           0, y, GRAPHICS_LOGICAL_WIDTH,
                           flicker < 2u ? 2 : 1,
                           GRAPHICS_RGB(36, 54, 8));
    }

    /* MFA-derived original PC title Springtrap frames. */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     398 + glitch, -8, 470, 508,
                     kTitleFrames[frame]);

    /* The retail screen uses the compact vertical F N A F 3 mark. */
    const uint32_t logo = GRAPHICS_RGB(222, 232, 204);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 66, 35, 3, "F", logo);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 66, 67, 3, "N", logo);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 66, 99, 3, "A", logo);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 66, 131, 3, "F", logo);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 66, 163, 3, "3", logo);

    /* Stars occupy the original upper-left title area. */
    const uint32_t star = GRAPHICS_RGB(241, 239, 203);
    int sx = 115;
    if (progress_save_is_night_completed(5)) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, sx, 34, 4, "*", star);
        sx += 46;
    }
    if (progress_save_is_night_completed(6)) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, sx, 34, 4, "*", star);
        sx += 46;
    }
    if (progress_save_good_ending_unlocked()) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, sx, 78, 4, "*", star);
        sx += 46;
    }
    if (progress_save_aggressive_nightmare_completed())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, sx, 78, 4, "*", star);

    const int new_y = 285;
    const int load_y = 331;
    const int nightmare_y = 377;
    const int extra_y = 423;
    const uint32_t dim = GRAPHICS_RGB(160, 171, 151);

    graphics_draw_text(GRAPHICS_TARGET_BOTH, 69, new_y, 3,
                       "NEW GAME",
                       game->title_selection == 0 ? COLOUR_WHITE : dim);

    graphics_draw_text(GRAPHICS_TARGET_BOTH, 69, load_y, 3,
                       "LOAD GAME",
                       game->title_selection == 1 ? COLOUR_WHITE : dim);
    char night_number[8];
    snprintf(night_number, sizeof(night_number), "%d",
             game->unlocked_night > 5 ? 5 : game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 289, load_y + 5, 2,
                       night_number,
                       game->title_selection == 1 ? COLOUR_WHITE : dim);

    /* Keep the Nightmare row visible like the PC title.  The recovery pass
     * does not alter save/menu routing yet; Night 6 remains reachable through
     * the existing LOAD GAME path until the runtime regressions are stable. */
    if (progress_save_is_night_completed(5)) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 69, nightmare_y, 3,
                           "NIGHTMARE", dim);
    }

    uint32_t extra_colour = game->title_selection == 2
        ? COLOUR_WHITE : dim;
    if (!progress_save_extras_unlocked())
        extra_colour = GRAPHICS_RGB(74, 80, 72);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 69, extra_y, 3,
                       "EXTRA", extra_colour);

    /* PC-style selection is text brightness plus a small marker, never a
     * large filled green rectangle. */
    const int marker_y = game->title_selection == 0
        ? new_y : (game->title_selection == 1 ? load_y : extra_y);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, marker_y + 1, 2,
                       ">", COLOUR_WHITE);

    if (sFinishing.title_notice_frames > 0u) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           372, 458, 1,
                           "EXTRA LOCKED",
                           GRAPHICS_RGB(155, 155, 145));
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
