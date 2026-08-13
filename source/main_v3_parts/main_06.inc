    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, six ? 301 : 275, 145, 8,
                       six ? "6:00" : "5:59", six ? GRAPHICS_RGB(224, 222, 93) : COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 577, 193, 4, "AM", COLOUR_GREEN);
    /* Retail PC owns this screen entirely with the 5:59 -> 6:00 animation.
     * The old Wii U prototype appended `NIGHT n COMPLETE`, save diagnostics and
     * an A-button prompt after 180 frames. Inter-night progression is handled
     * by the finishing layer, so none of that platform/debug UI belongs here. */
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_game_over(const Game *game)
{
    /* Retail PC FNaF 3 deliberately drops to an almost empty death screen:
     * black background and red `Game Over` text.  The previous Wii U prototype
     * added a red scanline field, `SPRINGTRAP GOT YOU`, and visible retry/menu
     * prompts.  Keep the existing controller input behavior, but do not alter
     * the source presentation to advertise those platform bindings. */
    (void)game;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 247, 218, 5,
                       "Game Over", GRAPHICS_RGB(160, 0, 0));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_placeholder(void)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 5, 3));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 61, 73, 732, 334, 3, GRAPHICS_RGB(36, 91, 44));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 194, 165, 5, "NOT PORTED YET", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 238, 335, 2, "B: RETURN", COLOUR_WHITE);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void render_game(Game *game)
{
    if (!game->dirty) return;
    game->dirty = false;
    switch (game->screen) {
        case SCREEN_WARNING: draw_warning(game); break;
        case SCREEN_TITLE: draw_title(game); break;
        case SCREEN_NIGHT_INTRO: draw_night_intro(game); break;
        case SCREEN_OFFICE: draw_office(game); break;
        case SCREEN_VICTORY: draw_victory(game); break;
        case SCREEN_SPRINGTRAP_JUMPSCARE: draw_springtrap_jumpscare(game); break;
        case SCREEN_GAME_OVER: draw_game_over(game); break;
        case SCREEN_PLACEHOLDER: draw_placeholder(); break;
    }
}

int main(int argc, char **argv)
{
    (void) argc; (void) argv;
    WHBProcInit();
    if (!graphics_init()) { WHBProcShutdown(); return 1; }
    input_init(); frame_clock_reset(); (void) audio_init();
    Game game; game_init(&game); render_game(&game);
    while (WHBProcIsRunning()) {
        InputState input; input_update(&input);
        update_game(&game, &input);
        render_game(&game);
        frame_clock_wait_next();
    }
    progress_save_shutdown();
    audio_stop_all(); audio_shutdown(); graphics_shutdown(); WHBProcShutdown();
    return 0;
}
