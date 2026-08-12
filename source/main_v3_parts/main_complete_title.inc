static void complete_draw_title(const Game *game)
{
    const int frame = title_frame_index(game);
    const int content_x = (GRAPHICS_LOGICAL_WIDTH - 640) / 2;

    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);

    /*
     * The five MFA title images are complete 1024x768 PC frames, not isolated
     * Springtrap cut-outs. Preserve the original 4:3 composition and center it
     * on the Wii U's wider 854x480 logical surface instead of stretching or
     * cropping the image into the right-hand side.
     */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x, 0, 640, 480,
                     kTitleFrames[frame]);

    /* Exact transparent PC horizontal/glitch-line frames (863/865/866/867/
     * 868/869/871), scaled with the same 4:3 transform as the background. */
    const int line_frame = (int)((game->screen_frames / 4u) % 7u);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x, 0, 640, 480,
                     gPcCompatTitleLineTextures[line_frame]);

    /* Original PC image-bank UI. No Wii U font reconstruction here. */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 66, 29, 21, 141,
                     &gPcCompatTitleLogoTexture);

    const int new_y = 284;
    const int load_y = 330;
    const int nightmare_y = 376;
    const int extra_y = 422;

    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 67, new_y, 135, 24,
                     &gPcCompatTitleNewGameTexture);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 67, load_y, 152, 31,
                     &gPcCompatTitleLoadGameTexture);

    if (progress_save_is_night_completed(5)) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         content_x + 67, nightmare_y, 161, 31,
                         &gPcCompatTitleNightmareTexture);
    }

    if (progress_save_extras_unlocked()) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         content_x + 67, extra_y, 91, 31,
                         &gPcCompatTitleExtraTexture);
    } else {
        /* Keep the row position stable while locked without inventing a large
         * green selection card. */
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           content_x + 67, extra_y + 7, 2,
                           "EXTRA", GRAPHICS_RGB(66, 72, 64));
    }

    /* The current recovery runtime still routes Night 6 through LOAD GAME.
     * Show its saved-night digit without changing navigation behavior yet. */
    char night_number[8];
    snprintf(night_number, sizeof(night_number), "%d",
             game->unlocked_night > 5 ? 5 : game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       content_x + 226, load_y + 9, 2,
                       night_number, COLOUR_WHITE);

    /* Exact glowing PC cursor block (image 833), not a synthetic '>' glyph. */
    const int marker_y = game->title_selection == 0
        ? new_y : (game->title_selection == 1 ? load_y : extra_y);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 36, marker_y, 21, 31,
                     &gPcCompatTitleCursorTexture);

    /* Exact PC star sprite. */
    int sx = content_x + 112;
    int sy = 30;
    if (progress_save_is_night_completed(5)) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         sx, sy, 35, 33,
                         &gPcCompatTitleStarTexture);
        sx += 47;
    }
    if (progress_save_is_night_completed(6)) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         sx, sy, 35, 33,
                         &gPcCompatTitleStarTexture);
        sx += 47;
    }
    if (progress_save_good_ending_unlocked()) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         content_x + 159, 70, 35, 33,
                         &gPcCompatTitleStarTexture);
    }
    if (progress_save_aggressive_nightmare_completed()) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         content_x + 206, 70, 35, 33,
                         &gPcCompatTitleStarTexture);
    }

    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 456, 456, 114, 13,
                     &gPcCompatTitleCopyrightTexture);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 385, 469, 161, 8,
                     &gPcCompatTitleResetTexture);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 574, 468, 47, 9,
                     &gPcCompatTitleVersionTexture);

    if (sFinishing.title_notice_frames > 0u) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           content_x + 430, 440, 1,
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
