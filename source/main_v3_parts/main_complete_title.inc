#include "assets/menu_pc_assets.h"

static void complete_draw_title(const Game *game)
{
    static const char *const items[TITLE_ITEM_COUNT] = {
        "NEW GAME", "CONTINUE", "EXTRAS"
    };
    static const int item_y[TITLE_ITEM_COUNT] = {286, 334, 405};
    static const TextureRle *const pc_title_frames[TITLE_FRAME_COUNT] = {
        &gPcTitleSpringtrap1,
        &gPcTitleSpringtrap2,
        &gPcTitleSpringtrap3,
        &gPcTitleSpringtrap4,
        &gPcTitleSpringtrap5
    };

    menu_pc_assets_init();
    const bool pc_title_ready =
        gPcTitleLogo.width > 0u &&
        gPcTitleSpringtrap1.width > 0u &&
        gPcTitleSpringtrap2.width > 0u &&
        gPcTitleSpringtrap3.width > 0u &&
        gPcTitleSpringtrap4.width > 0u &&
        gPcTitleSpringtrap5.width > 0u;

    const int frame = title_frame_index(game);
    const int glitch = frame == 0
        ? (int) ((game->seed >> 8) % 7u) - 3
        : (frame - 2) * 3;

    /* PC-style title composition: almost-black menu field on the left and
     * Springtrap occupying the right side. */
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(0, 2, 1));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 410, 480,
                       GRAPHICS_RGB(1, 5, 3));

    if (pc_title_ready) {
        /* Original narrow PC FNAF3 logo, kept at its native aspect ratio. */
        texture_draw_rle(GRAPHICS_TARGET_BOTH, 38, 39, 55, 377,
                         &gPcTitleLogo);
    } else {
        /* Safe fallback if the packed PC asset cannot be decoded. */
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 47, 7, "F", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 119, 7, "N", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 191, 7, "A", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 263, 7, "F", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 349, 8, "3", COLOUR_GREEN);
    }

    /* Keep the four completion stars, but present them as part of the title
     * rather than as a separate status bar. */
    const uint32_t star = GRAPHICS_RGB(222, 219, 142);
    if (progress_save_is_night_completed(5))
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 66, 3, "*", star);
    if (progress_save_is_night_completed(6))
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 148, 66, 3, "*", star);
    if (progress_save_good_ending_unlocked())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 175, 66, 3, "*", star);
    if (progress_save_aggressive_nightmare_completed())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 202, 66, 3, "*", star);

    for (int item = 0; item < TITLE_ITEM_COUNT; ++item) {
        const int y = item_y[item];
        const bool selected = item == game->title_selection;
        uint32_t colour = selected
            ? COLOUR_WHITE : GRAPHICS_RGB(137, 151, 137);
        if (item == 2 && !progress_save_extras_unlocked())
            colour = GRAPHICS_RGB(67, 76, 68);

        if (selected)
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 112, y, 2,
                               ">>", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 157, y, 2,
                           items[item], colour);
    }

    /* Like the PC Continue entry, show the save's current night as a small
     * secondary line instead of a permanent technical status panel. */
    char night_text[20];
    snprintf(night_text, sizeof(night_text), "NIGHT %d", game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 157, 361, 1,
                       night_text, GRAPHICS_RGB(102, 115, 102));

    if (!progress_save_extras_unlocked())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 267, 411, 1,
                           "LOCKED", GRAPHICS_RGB(67, 76, 68));

    if (sFinishing.title_notice_frames > 0u)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 112, 449, 1,
                           "COMPLETE NIGHT 5 TO UNLOCK EXTRAS", COLOUR_RED);
    else if (progress_save_load_status_is_error())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 112, 449, 1,
                           progress_save_load_status_text(), COLOUR_RED);

    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 410, 0, 444, 480, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 410 + glitch, 16, 444, 448,
                     pc_title_ready ? pc_title_frames[frame]
                                    : kTitleFrames[frame]);
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
