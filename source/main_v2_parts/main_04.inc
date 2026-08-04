                game->dirty = true;
            }
            if (game->screen_frames >= 180u &&
                (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
                 input_was_pressed(input, GAME_BUTTON_START))) {
                if (game->night < NIGHT_COUNT) {
                    game->unlocked_night = game->night + 1;
                    reset_night(game, game->unlocked_night);
                } else {
                    game->screen = SCREEN_TITLE;
                    game->screen_frames = 0u;
                    game->dirty = true;
                }
            }
            break;

        case SCREEN_GAME_OVER:
            if ((game->screen_frames % 5u) == 0u) {
                game->dirty = true;
            }
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
                input_was_pressed(input, GAME_BUTTON_START)) {
                audio_stop(AUDIO_CUE_SCREAM);
                reset_night(game, game->night);
            } else if (input_was_pressed(input, GAME_BUTTON_BACK)) {
                audio_stop(AUDIO_CUE_SCREAM);
                game->screen = SCREEN_TITLE;
                game->screen_frames = 0u;
                game->dirty = true;
            }
            break;

        case SCREEN_PLACEHOLDER:
            if (input_was_pressed(input, GAME_BUTTON_BACK)) {
                game->screen = SCREEN_TITLE;
                game->screen_frames = 0u;
                game->dirty = true;
            }
            break;
    }
}

static void draw_scanlines(uint32_t target, uint32_t seed)
{
    for (int y = 0; y < GRAPHICS_LOGICAL_HEIGHT; y += 10) {
        const uint32_t shade = 3u + ((seed + (uint32_t) y) % 5u);
        graphics_draw_rect(target, 0, y, GRAPHICS_LOGICAL_WIDTH, 1,
                           GRAPHICS_RGB(shade, shade + 4u, shade));
    }
}

static void draw_warning(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     67, 174, 720, 138,
                     &gWarningTexture);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       285, 390, 2, "A: SKIP",
                       GRAPHICS_RGB(105, 151, 101));
    if ((game->screen_frames / 10u) & 1u) {
        graphics_draw_frame(GRAPHICS_TARGET_BOTH,
                            49, 146, 755, 205, 3,
                            GRAPHICS_RGB(96, 14, 10));
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_title(const Game *game)
{
    static const char *const items[TITLE_ITEM_COUNT] = {
        "NEW GAME", "LOAD GAME", "EXTRAS"
    };
    const int frame = title_frame_index(game);
    const int glitch = frame == 0
        ? (int) ((game->seed >> 8) % 7u) - 3
        : (frame - 2) * 3;

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 410, 480,
                       GRAPHICS_RGB(2, 7, 4));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 47, 7, "F", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 119, 7, "N", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 191, 7, "A", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 263, 7, "F", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 349, 8, "3", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 120, 72, 2,
                       "FIVE NIGHTS AT FREDDYS 3", COLOUR_GREEN);

    char continue_text[32];
    snprintf(continue_text, sizeof(continue_text),
             "CURRENT NIGHT: %d", game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 119, 1,
                       continue_text, GRAPHICS_RGB(74, 111, 73));

    for (int item = 0; item < TITLE_ITEM_COUNT; ++item) {
        const int y = 284 + item * 48;
        if (item == game->title_selection) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 262, 36,
                               GRAPHICS_RGB(31, 72, 37));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 5, 36,
                               GRAPHICS_RGB(129, 207, 121));
        }
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 124, y, 3, items[item],
                           item == game->title_selection
                               ? COLOUR_WHITE
                               : GRAPHICS_RGB(112, 139, 113));
    }

    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 410, 0, 444, 480, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     410 + glitch, 16, 444, 448,
                     kTitleFrames[frame]);
    draw_scanlines(GRAPHICS_TARGET_BOTH, game->seed + game->screen_frames);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_night_intro(const Game *game)
{
    char night_text[24];
    snprintf(night_text, sizeof(night_text), "NIGHT %d", game->night);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 286, 139, 7,
                       night_text, COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 329, 239, 4,
                       "12:00 AM", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 245, 315, 2,
                       night_config(game->night)->subtitle,
                       GRAPHICS_RGB(84, 124, 82));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 306, 397, 1,
                       "A: SKIP INTRO", GRAPHICS_RGB(72, 102, 71));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_springtrap_office(const Game *game)
{
    const SpringtrapOfficeSide side =
        springtrap_ai_office_side(&game->springtrap);
    int x = 0;
    if (side == SPRINGTRAP_OFFICE_WINDOW && game->office_pan > 25) {
        x = 690;
    } else if (side == SPRINGTRAP_OFFICE_LEFT && game->office_pan < -25) {
        x = 95;
    } else {
        return;
    }

    graphics_draw_ellipse(GRAPHICS_TARGET_TV, x, 182, 61, 100,
                          GRAPHICS_RGB(16, 29, 15));
    graphics_draw_ellipse(GRAPHICS_TARGET_TV, x, 165, 47, 72,
                          GRAPHICS_RGB(42, 67, 34));
    graphics_draw_rect(GRAPHICS_TARGET_TV, x - 38, 76, 18, 68,
                       GRAPHICS_RGB(34, 58, 30));
    graphics_draw_rect(GRAPHICS_TARGET_TV, x + 19, 67, 17, 76,
                       GRAPHICS_RGB(34, 58, 30));
    graphics_draw_rect(GRAPHICS_TARGET_TV, x - 28, 157, 17, 9,
                       GRAPHICS_RGB(221, 240, 211));
    graphics_draw_rect(GRAPHICS_TARGET_TV, x + 13, 157, 17, 9,
                       GRAPHICS_RGB(221, 240, 211));
    graphics_draw_ellipse(GRAPHICS_TARGET_TV, x, 225, 45, 35,
                          GRAPHICS_RGB(11, 19, 10));
}

static void draw_danger_message(const Game *game)
{
    if (game->message != MESSAGE_DANGER_SOFT &&
        game->message != MESSAGE_DANGER_LOUD) {
        return;
    }
    const bool loud = game->message == MESSAGE_DANGER_LOUD;
    graphics_draw_frame(GRAPHICS_TARGET_TV,
                        537, 52, 299, 38, 3,
                        loud ? COLOUR_RED : GRAPHICS_RGB(194, 141, 65));
    graphics_draw_text(GRAPHICS_TARGET_TV,
                       562, 65, 2,
                       loud ? "DANGER SIGNAL: LOUD"
                            : "DANGER SIGNAL: SOFT",
                       loud ? COLOUR_RED : GRAPHICS_RGB(194, 141, 65));
}

static void draw_office_tv(const Game *game)
{
    const int texture_x = -133 + game->office_pan;
    graphics_clear(GRAPHICS_TARGET_TV, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_TV,
                     texture_x, 24, 1120, 430,
                     &gOfficeTexture);
    draw_springtrap_office(game);

    for (int y = 30; y < 454; y += 12) {
        graphics_draw_rect(GRAPHICS_TARGET_TV, 0, y, 854, 1,
                           GRAPHICS_RGB(4, 8, 5));
    }

    char night_text[24];
    char clock_text[16];
    snprintf(night_text, sizeof(night_text), "NIGHT %d", game->night);
    snprintf(clock_text, sizeof(clock_text), "%d AM", game->hour);
    graphics_draw_text(GRAPHICS_TARGET_TV, 18, 5, 2,
                       night_text, GRAPHICS_RGB(166, 191, 151));
    graphics_draw_rect(GRAPHICS_TARGET_TV, 718, 4, 118, 36,
                       GRAPHICS_RGB(1, 5, 2));
    graphics_draw_frame(GRAPHICS_TARGET_TV, 718, 4, 118, 36, 2,
                        GRAPHICS_RGB(65, 103, 63));
    graphics_draw_text(GRAPHICS_TARGET_TV, 744, 16, 2,
                       clock_text, COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_TV, 18, 458, 1,
                       "LEFT/RIGHT LOOK  X/Y CAMERAS  MINUS MAINTENANCE",
                       GRAPHICS_RGB(119, 150, 111));

    if (game->ventilation_failed) {
        graphics_draw_frame(GRAPHICS_TARGET_TV,
                            555, 9, 157, 34, 3, COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_TV,
                           572, 20, 1, "VENT ERROR", COLOUR_RED);
    }
    draw_danger_message(game);
}

static void draw_office_gamepad(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(3, 7, 4));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        57, 57, 740, 356, 4,
                        GRAPHICS_RGB(40, 77, 43));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91, 91, 4, "OFFICE CONTROL", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91, 178, 3,
                       "X/Y: CAMERA PANEL", GRAPHICS_RGB(134, 181, 129));
