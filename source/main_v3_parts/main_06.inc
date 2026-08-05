    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, six ? 301 : 275, 145, 8,
                       six ? "6:00" : "5:59", six ? GRAPHICS_RGB(224, 222, 93) : COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 577, 193, 4, "AM", COLOUR_GREEN);
    if (game->screen_frames >= 180u) {
        char text[32]; snprintf(text, sizeof(text), "NIGHT %d COMPLETE", game->night);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 248, 305, 3, text, COLOUR_GREEN);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 304, 346, 1,
                           autosave_result_text(), autosave_result_colour());
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 236, 379, 2,
                           game->night < NIGHT_COUNT ? "A: START NEXT NIGHT" : "A: RETURN TO TITLE",
                           GRAPHICS_RGB(104, 151, 101));
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_game_over(const Game *game)
{
    const uint32_t pulse = ((game->screen_frames / 5u) & 1u) ? COLOUR_RED : GRAPHICS_RGB(93, 10, 7);
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(4, 0, 0));
    for (int y = 0; y < 480; y += 9) graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y, 854, 2, pulse);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 183, 135, 7, "GAME OVER", pulse);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 227, 269, 3, "SPRINGTRAP GOT YOU", GRAPHICS_RGB(213, 170, 155));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 257, 370, 2, "A: RETRY   B: MENU", GRAPHICS_RGB(169, 118, 108));
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
    save_system_stop();
    audio_stop_all(); audio_shutdown(); graphics_shutdown(); WHBProcShutdown();
    return 0;
}
