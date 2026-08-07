/* PC-style Nightmare / Night 6 newspaper ending.
 *
 * This is a native Wii U reconstruction of the original composition rather
 * than a copied bitmap: dark paper, IT BURNS masthead, burned-attraction
 * photograph, Freddy desk figurine and the barely visible Springtrap survivor.
 */

static void night6_draw_paper_grain(void)
{
    uint32_t seed = 0xF3A6B91Du;
    for (int index = 0; index < 92; ++index) {
        seed = seed * 1664525u + 1013904223u;
        const int x = 78 + (int) ((seed >> 9) % 700u);
        seed = seed * 1664525u + 1013904223u;
        const int y = 23 + (int) ((seed >> 10) % 431u);
        const int shade = 152 + (int) ((seed >> 24) & 31u);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y,
                           1 + (index % 3 == 0), 1,
                           GRAPHICS_RGB(shade, shade, shade - 5));
    }
}

static void night6_draw_rubble_photo(const Game *game)
{
    const int px = 108;
    const int py = 171;
    const int pw = 326;
    const int ph = 220;
    const uint32_t flicker = ((game->screen_frames / 7u) & 1u)
        ? GRAPHICS_RGB(67, 66, 62) : GRAPHICS_RGB(58, 57, 54);

    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px, py, pw, ph,
                       GRAPHICS_RGB(21, 21, 20));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, px, py, pw, ph, 3,
                        GRAPHICS_RGB(37, 35, 32));

    /* Charred wall / roof remains. */
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 12, py + 38, 302, 10,
                       GRAPHICS_RGB(38, 37, 35));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 44, py + 13, 18, 105,
                       GRAPHICS_RGB(42, 41, 38));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 250, py + 25, 16, 112,
                       GRAPHICS_RGB(43, 42, 39));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 126, py + 21, 91, 9,
                       GRAPHICS_RGB(47, 45, 41));

    /* Barely-visible Springtrap behind the foreground object. */
    const uint32_t spring = GRAPHICS_RGB(29, 32, 27);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, px + 252, py + 83, 27, 35,
                          spring);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 231, py + 47, 12, 34,
                       spring);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 260, py + 39, 11, 42,
                       spring);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 237, py + 110, 39, 60,
                       spring);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 242, py + 78, 7, 4,
                       GRAPHICS_RGB(67, 70, 57));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 260, py + 78, 7, 4,
                       GRAPHICS_RGB(67, 70, 57));

    /* Freddy bobble-head / salvaged desk figure in the foreground. */
    const uint32_t freddy = GRAPHICS_RGB(109, 108, 103);
    const uint32_t freddy_dark = GRAPHICS_RGB(74, 73, 70);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, px + 160, py + 116, 43, 42,
                          freddy);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, px + 126, py + 91, 14, 15,
                          freddy_dark);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, px + 194, py + 91, 14, 15,
                          freddy_dark);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 137, py + 63, 46, 12,
                       freddy_dark);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 146, py + 53, 28, 12,
                       freddy_dark);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 126, py + 151, 68, 49,
                       freddy);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, px + 160, py + 170, 18, 22,
                          GRAPHICS_RGB(142, 141, 135));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 139, py + 112, 8, 5,
                       GRAPHICS_RGB(225, 225, 215));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, px + 174, py + 112, 8, 5,
                       GRAPHICS_RGB(225, 225, 215));

    /* Broken attraction debris across the lower photo. */
    for (int index = 0; index < 15; ++index) {
        const int x = px + 8 + index * 22;
        const int h = 11 + (index * 17) % 31;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, py + ph - h - 4,
                           28, h, index & 1 ? flicker : GRAPHICS_RGB(45, 44, 42));
    }
}

static void night6_draw_article_lines(void)
{
    const uint32_t ink = GRAPHICS_RGB(43, 41, 38);
    static const int widths[] = {
        257, 244, 263, 231, 252, 218,
        255, 238, 266, 228, 247, 215
    };
    for (int index = 0; index < 12; ++index) {
        const int y = 263 + index * 10;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 469, y,
                           widths[index], 3, ink);
    }
}

static void night6_draw_newspaper(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(5, 4, 5));

    /* Newspaper page and slight uneven shadow. */
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 66, 13, 728, 454,
                       GRAPHICS_RGB(19, 17, 16));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 73, 18, 710, 442,
                       GRAPHICS_RGB(196, 193, 178));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 73, 18, 710, 442, 2,
                        GRAPHICS_RGB(90, 84, 75));
    night6_draw_paper_grain();

    /* Original-like black top masthead. */
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 73, 18, 710, 82,
                       GRAPHICS_RGB(18, 16, 18));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 260, 36, 7,
                       "IT BURNS!", COLOUR_WHITE);

    graphics_draw_text(GRAPHICS_TARGET_BOTH, 118, 112, 3,
                       "FAZBEARS FRIGHT", GRAPHICS_RGB(35, 32, 29));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 119, 140, 3,
                       "BURNS TO THE GROUND!", GRAPHICS_RGB(35, 32, 29));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 103, 163, 641, 3,
                       GRAPHICS_RGB(80, 75, 68));

    night6_draw_rubble_photo(game);

    /* Right-hand article: use short paraphrased lines, keeping the same visual
     * rhythm as the PC clipping instead of adding port/debug text. */
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 470, 178, 1,
                       "LOCAL HORROR ATTRACTION", GRAPHICS_RGB(48, 45, 41));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 470, 195, 1,
                       "DESTROYED OVERNIGHT", GRAPHICS_RGB(48, 45, 41));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 470, 221, 1,
                       "FAULTY WIRING SUSPECTED", GRAPHICS_RGB(57, 53, 48));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 470, 238, 1,
                       "FOUL PLAY NOT RULED OUT", GRAPHICS_RGB(57, 53, 48));
    night6_draw_article_lines();
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 470, 397, 1,
                       "SALVAGED ITEMS TO BE", GRAPHICS_RGB(57, 53, 48));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 470, 414, 1,
                       "SOLD AT PUBLIC AUCTION", GRAPHICS_RGB(57, 53, 48));

    /* Keep the PC image itself clean; only reveal the Wii U return hint after
     * the player has had time to read the clipping. */
    if (game->screen_frames >= 220u) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 250, 438, 354, 20,
                           GRAPHICS_RGB(14, 13, 13));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 302, 445, 1,
                           "A: RETURN TO TITLE", COLOUR_WHITE);
    }

    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void fnaf3_night6_render_game(Game *game)
{
    if (!game->dirty) return;
    if (game->screen == SCREEN_PLACEHOLDER &&
        sFinishing.mode == FINISHING_NIGHT6_ENDING) {
        game->dirty = false;
        night6_draw_newspaper(game);
        return;
    }
    fnaf3_follow_me_render_game(game);
}
