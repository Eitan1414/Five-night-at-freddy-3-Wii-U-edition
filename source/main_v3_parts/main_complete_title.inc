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
    const int credits_text_y = 454;
    const int credits_cursor_y = 448;
    const bool bonus_rows = progress_save_is_night_completed(5);

    /* Exact PC NEW GAME object (General Sprites image 592). */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 67, new_y, 152, 31,
                     &gPcCompatTitleNewGameTexture);

    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 67, load_y, 152, 31,
                     &gPcCompatTitleLoadGameTexture);

    if (bonus_rows) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         content_x + 67, nightmare_y, 161, 31,
                         &gPcCompatTitleNightmareTexture);
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         content_x + 67, extra_y, 91, 31,
                         &gPcCompatTitleExtraTexture);
    }

    /* Credits is Wii U-specific navigation, so there is no retail PC title
     * sprite to rip for it. Draw it with the project's PC-title-matched pixel
     * alphabet instead of borrowing any PSX menu art. It remains available
     * before and after the Night 5 bonus rows unlock. */
    graphics_draw_fnaf3_menu_text(GRAPHICS_TARGET_BOTH,
                                  content_x + 67, credits_text_y, 1,
                                  "CREDITS", COLOUR_WHITE);

    /* The LOAD GAME night value is still rendered through the compatibility
     * bitmap font. Do not claim an MFA counter match until the title-frame
     * counter object is positively identified. */
    char night_number[8];
    snprintf(night_number, sizeof(night_number), "%d",
             progress_save_continue_night());
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       content_x + 226, load_y + 9, 2,
                       night_number, COLOUR_WHITE);

    /* Exact glowing PC cursor block (image 833). Nightmare and Extra remain
     * separate PC rows; the final position selects the Wii U Credits row. */
    int marker_y = new_y;
    if (game->title_selection == 1) {
        marker_y = load_y;
    } else if (bonus_rows && game->title_selection == 2) {
        marker_y = nightmare_y;
    } else if (bonus_rows && game->title_selection == 3) {
        marker_y = extra_y;
    } else if ((!bonus_rows && game->title_selection == 2) ||
               (bonus_rows && game->title_selection == 4)) {
        marker_y = credits_cursor_y;
    }
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

    /* Wii U options remain bound to the platform input layer but no longer
     * advertise themselves over the retail-PC title composition. Likewise the
     * prototype `EXTRA LOCKED` notice is intentionally presentation-silent. */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 456, 456, 114, 13,
                     &gPcCompatTitleCopyrightTexture);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 385, 469, 161, 8,
                     &gPcCompatTitleResetTexture);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     content_x + 574, 468, 47, 9,
                     &gPcCompatTitleVersionTexture);

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
