static void complete_draw_title(const Game *game)
{
    const int frame = title_frame_index(game);
    const int content_x = (GRAPHICS_LOGICAL_WIDTH - 640) / 2;

    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);

    /* The five MFA title images are complete 1024x768 PC frames, not isolated
     * Springtrap cut-outs. Preserve their original 4:3 composition. */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x, 0, 640, 480,
                     kTitleFrames[frame]);

    /* The PC title spends most of its time on thin horizontal interference and
     * only enters the heavier glitch-line frames during the short title burst. */
    const uint32_t phase = game->screen_frames % 180u;
    const int line_frame = phase < 144u
        ? 3
        : (int)(((phase - 144u) / 6u) % 7u);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x, 0, 640, 480,
                     gPcCompatTitleLineTextures[line_frame]);

    /* Original PC image-bank UI. */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 66, 29, 21, 141,
                     &gPcCompatTitleLogoTexture);

    const int new_y = 284;
    const int load_y = 330;
    const int nightmare_y = 376;
    const int extra_y = 422;

    /* ID 594 in the dump is REBOOT ALL, not NEW GAME. Until the converter's
     * New Game entry is switched to verified ID 592, keep this single row as
     * text rather than rendering a knowingly wrong PC sprite. */
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       content_x + 67, new_y + 4, 3,
                       "NEW GAME", COLOUR_WHITE);

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
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           content_x + 67, extra_y + 7, 2,
                           "EXTRA", GRAPHICS_RGB(66, 72, 64));
    }

    /* Recovery keeps current menu routing for this hardware test. */
    char night_number[8];
    snprintf(night_number, sizeof(night_number), "%d",
             game->unlocked_night > 5 ? 5 : game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       content_x + 226, load_y + 9, 2,
                       night_number, COLOUR_WHITE);

    /* Exact glowing PC cursor block (image 833). */
    const int marker_y = game->title_selection == 0
        ? new_y : (game->title_selection == 1 ? load_y : extra_y);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 36, marker_y, 21, 31,
                     &gPcCompatTitleCursorTexture);

    /* Exact PC star sprite. */
    int sx = content_x + 112;
    if (progress_save_is_night_completed(5)) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         sx, 30, 35, 33,
                         &gPcCompatTitleStarTexture);
        sx += 47;
    }
    if (progress_save_is_night_completed(6)) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         sx, 30, 35, 33,
                         &gPcCompatTitleStarTexture);
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
