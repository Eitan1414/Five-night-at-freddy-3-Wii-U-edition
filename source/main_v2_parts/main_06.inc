    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 5, 3));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        38, 31, 778, 412, 4,
                        GRAPHICS_RGB(54, 112, 59));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       81, 58, 4, "MAINTENANCE PANEL", COLOUR_GREEN);

    for (int item = 0; item < MAINTENANCE_ITEM_COUNT; ++item) {
        const int y = 126 + item * 55;
        const bool selected = item == game->maintenance_selection;
        if (selected) {
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               76, y - 12, 701, 40,
                               GRAPHICS_RGB(24, 60, 29));
        }
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           101, y, 3, labels[item],
                           selected ? COLOUR_WHITE
                                    : GRAPHICS_RGB(111, 148, 108));
        if (item < MAINT_REBOOT_ALL) {
            const bool failed = maintenance_failed(game, item);
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               602, y, 2,
                               failed ? "ERROR" : "ONLINE",
                               failed ? COLOUR_RED : COLOUR_GREEN);
        }
    }

    if (game->repairing) {
        const uint32_t duration = repair_duration(game);
        const int progress = (int) ((game->repair_frames * 626u) / duration);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           103, 406, 626, 13, GRAPHICS_RGB(9, 20, 11));
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           103, 406, progress, 13, GRAPHICS_RGB(89, 184, 85));
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           303, 375, 2, "REBOOTING...", COLOUR_GREEN);
    } else {
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           103, 407, 1,
                           "UP/DOWN SELECT  A REBOOT  B/MINUS CLOSE",
                           GRAPHICS_RGB(110, 151, 107));
    }
}

static void draw_ventilation_overlay(const Game *game)
{
    if (!game->ventilation_failed) {
        return;
    }
    const uint32_t phase =
        game->ventilation_error_frames % VENT_BLACKOUT_CYCLE_FRAMES;
    if (phase < VENT_DIM_START_FRAMES) {
        return;
    }
    if (phase >= VENT_BLACK_START_FRAMES) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           0, 0, 854, 480, COLOUR_BLACK);
        return;
    }

    const uint32_t progress = phase - VENT_DIM_START_FRAMES;
    int spacing = 10;
    if (progress > 120u) {
        spacing = 3;
    } else if (progress > 60u) {
        spacing = 6;
    }
    for (int y = 0; y < 480; y += spacing) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           0, y, 854, spacing / 2 + 1, COLOUR_BLACK);
    }
}

static void draw_office(const Game *game)
{
    draw_office_tv(game);
    if (game->maintenance_open) {
        draw_maintenance(game);
    } else if (game->camera_open) {
        if (game->vent_map_open) {
            draw_vent_map(game);
        } else {
            draw_camera_feed(game);
        }
    } else {
        draw_office_gamepad(game);
    }
    draw_ventilation_overlay(game);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_victory(const Game *game)
{
    const bool six = game->screen_frames >= 90u;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       six ? 301 : 275, 145, 8,
                       six ? "6:00" : "5:59",
                       six ? GRAPHICS_RGB(224, 222, 93) : COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       577, 193, 4, "AM", COLOUR_GREEN);
    if (game->screen_frames >= 180u) {
        char text[32];
        snprintf(text, sizeof(text), "NIGHT %d COMPLETE", game->night);
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           248, 305, 3, text, COLOUR_GREEN);
        graphics_draw_text(GRAPHICS_TARGET_BOTH,
                           236, 379, 2,
                           game->night < NIGHT_COUNT
                               ? "A: START NEXT NIGHT"
                               : "A: RETURN TO TITLE",
                           GRAPHICS_RGB(104, 151, 101));
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_game_over(const Game *game)
{
    const uint32_t pulse = ((game->screen_frames / 5u) & 1u)
        ? COLOUR_RED : GRAPHICS_RGB(93, 10, 7);
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(4, 0, 0));
    for (int y = 0; y < 480; y += 9) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y, 854, 2, pulse);
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       183, 135, 7, "GAME OVER", pulse);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       227, 269, 3, "SPRINGTRAP GOT YOU",
                       GRAPHICS_RGB(213, 170, 155));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       257, 370, 2, "A: RETRY   B: MENU",
                       GRAPHICS_RGB(169, 118, 108));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_placeholder(void)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 5, 3));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH,
                        61, 73, 732, 334, 3,
                        GRAPHICS_RGB(36, 91, 44));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       194, 165, 5, "NOT PORTED YET", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       238, 335, 2, "B: RETURN", COLOUR_WHITE);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void render_game(Game *game)
{
    if (!game->dirty) {
        return;
    }
    game->dirty = false;
    switch (game->screen) {
        case SCREEN_WARNING: draw_warning(game); break;
        case SCREEN_TITLE: draw_title(game); break;
        case SCREEN_NIGHT_INTRO: draw_night_intro(game); break;
        case SCREEN_OFFICE: draw_office(game); break;
        case SCREEN_VICTORY: draw_victory(game); break;
        case SCREEN_GAME_OVER: draw_game_over(game); break;
        case SCREEN_PLACEHOLDER: draw_placeholder(); break;
    }
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    WHBProcInit();
    if (!graphics_init()) {
        WHBProcShutdown();
        return 1;
    }

    input_init();
    frame_clock_reset();
    (void) audio_init();

    Game game;
    game_init(&game);
    render_game(&game);

    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);
        update_game(&game, &input);
        render_game(&game);
        frame_clock_wait_next();
    }

    audio_stop_all();
    audio_shutdown();
    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
