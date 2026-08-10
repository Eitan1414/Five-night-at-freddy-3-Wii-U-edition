static Game *sCompleteRenderGame = NULL;

static bool complete_cheats_unlocked(void)
{
    return progress_save_is_night_completed(6) &&
           progress_save_good_ending_unlocked();
}

static void complete_set_extras_page(Game *game, ExtrasPage page)
{
    sFinishing.extras_page = page;
    sFinishing.extras_selection = 0;
    sFinishing.extras_index = 0;
    game->screen_frames = 0u;
    game->dirty = true;
}

static bool complete_update_extras(Game *game, const InputState *input)
{
    if (game->screen != SCREEN_PLACEHOLDER ||
        sFinishing.mode != FINISHING_EXTRAS) {
        return false;
    }

    if (sFinishing.extras_page == EXTRAS_PAGE_MAIN) {
        ++game->screen_frames;
        if (sCompleteCheats.locked_notice_frames > 0u)
            --sCompleteCheats.locked_notice_frames;

        const int old = sFinishing.extras_selection;
        if (input_was_pressed(input, GAME_BUTTON_UP))
            sFinishing.extras_selection =
                (sFinishing.extras_selection + 5) % 6;
        if (input_was_pressed(input, GAME_BUTTON_DOWN))
            sFinishing.extras_selection =
                (sFinishing.extras_selection + 1) % 6;
        if (old != sFinishing.extras_selection) content_play_select();

        if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_START)) {
            static const ExtrasPage pages[6] = {
                EXTRAS_PAGE_GALLERY,
                EXTRAS_PAGE_JUMPSCARES,
                EXTRAS_PAGE_MINIGAMES,
                EXTRAS_PAGE_CHEATS,
                EXTRAS_PAGE_ACHIEVEMENTS,
                EXTRAS_PAGE_CREDITS
            };
            if (sFinishing.extras_selection == 3 &&
                !complete_cheats_unlocked()) {
                sCompleteCheats.locked_notice_frames = 180u;
                audio_play(AUDIO_CUE_END, 0.52f, false);
            } else {
                content_play_select();
                complete_set_extras_page(
                    game, pages[sFinishing.extras_selection]);
            }
        } else if (input_was_pressed(input, GAME_BUTTON_BACK)) {
            content_play_select();
            finishing_return_to_title(game);
            content_start_title_music();
        }
        game->dirty = true;
        return true;
    }

    if (sFinishing.extras_page == EXTRAS_PAGE_ACHIEVEMENTS) {
        ++game->screen_frames;
        const int old = sAchievements.menu_selection;
        if (input_was_pressed(input, GAME_BUTTON_UP))
            sAchievements.menu_selection =
                (sAchievements.menu_selection + ACH_NORMAL_COUNT - 1) %
                ACH_NORMAL_COUNT;
        if (input_was_pressed(input, GAME_BUTTON_DOWN))
            sAchievements.menu_selection =
                (sAchievements.menu_selection + 1) % ACH_NORMAL_COUNT;
        if (old != sAchievements.menu_selection) content_play_select();
        if (input_was_pressed(input, GAME_BUTTON_BACK)) {
            content_play_select();
            complete_set_extras_page(game, EXTRAS_PAGE_MAIN);
        }
        game->dirty = true;
        return true;
    }

    if (sFinishing.extras_page == EXTRAS_PAGE_CHEATS) {
        ++game->screen_frames;
        const int old = sCompleteCheats.selection;
        if (input_was_pressed(input, GAME_BUTTON_UP))
            sCompleteCheats.selection =
                (sCompleteCheats.selection + 3) % 4;
        if (input_was_pressed(input, GAME_BUTTON_DOWN))
            sCompleteCheats.selection =
                (sCompleteCheats.selection + 1) % 4;
        if (old != sCompleteCheats.selection) content_play_select();

        if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_START)) {
            switch (sCompleteCheats.selection) {
                case 0:
                    sCompleteCheats.fast_nights =
                        !sCompleteCheats.fast_nights;
                    break;
                case 1:
                    sCompleteCheats.radar = !sCompleteCheats.radar;
                    break;
                case 2:
                    sCompleteCheats.aggressive =
                        !sCompleteCheats.aggressive;
                    break;
                case 3:
                    sCompleteCheats.no_errors =
                        !sCompleteCheats.no_errors;
                    break;
                default:
                    break;
            }
            content_play_select();
            game->dirty = true;
        }
        if (input_was_pressed(input, GAME_BUTTON_BACK)) {
            content_play_select();
            complete_set_extras_page(game, EXTRAS_PAGE_MAIN);
        }
        game->dirty = true;
        return true;
    }

    return false;
}

static void complete_clear_system_errors(Game *game)
{
    game->camera_failed = false;
    game->audio_failed = false;
    game->ventilation_failed = false;
    game->camera_failure_triggered = true;
    game->audio_failure_triggered = true;
    game->ventilation_failure_triggered = true;
    game->ventilation_error_frames = 0u;
    game->idle_frames = 0u;
    game->player_blinded = false;
    game->breathing_started = false;
    audio_stop(AUDIO_CUE_ALARM);
    audio_stop(AUDIO_CUE_BREATHING);
}

static void complete_apply_aggressive_ai(Game *game)
{
    const NightConfig *config = night_config(game->night);
    if (!config->springtrap_enabled) return;

    const SpringtrapOfficeSide attack_side =
        springtrap_ai_office_side(&game->springtrap);
    const SpringtrapEvent event = springtrap_ai_update(
        &game->springtrap,
        game->night,
        game->hour == 12 ? 0 : game->hour,
        springtrap_interval(game),
        game->sealed_vent,
        springtrap_directly_observed(game),
        game->player_blinded);
    process_springtrap_event(game, event, attack_side);
}

static void complete_apply_gameplay_cheats(Game *game,
                                           GameScreen before_screen)
{
    if (game->screen == SCREEN_OFFICE && sCompleteCheats.fast_nights) {
        ++game->hour_frames;
        if (game->hour_frames >= FRAMES_PER_HOUR)
            advance_hour(game);
    }

    if (game->screen == SCREEN_OFFICE && sCompleteCheats.no_errors)
        complete_clear_system_errors(game);

    if (game->screen == SCREEN_OFFICE && sCompleteCheats.aggressive)
        complete_apply_aggressive_ai(game);

    if (before_screen == SCREEN_OFFICE &&
        game->screen == SCREEN_VICTORY &&
        game->night == 6 &&
        sCompleteCheats.aggressive &&
        !sCompleteCheats.fast_nights &&
        !sCompleteCheats.radar &&
        !sCompleteCheats.no_errors) {
        progress_save_complete_aggressive_nightmare();
        achievement_unlock(game, ACH_NIGHT_6_AGGRESSIVE);
    }
}

static void complete_draw_extras_main(void)
{
    static const char *const items[6] = {
        "ANIMATRONICS",
        "JUMPSCARES",
        "SECRET MINIGAMES",
        "CHEATS",
        "ACHIEVEMENTS",
        "CREDITS"
    };

    finishing_draw_header("EXTRAS");
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 126, 65, 602, 365, 4,
                        GRAPHICS_RGB(43, 91, 48));
    for (int item = 0; item < 6; ++item) {
        const int y = 84 + item * 56;
        if (item == sFinishing.extras_selection)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                               157, y - 10, 540, 38,
                               GRAPHICS_RGB(24, 61, 29));
        uint32_t colour = item == sFinishing.extras_selection
            ? COLOUR_WHITE : GRAPHICS_RGB(107, 145, 104);
        if (item == 3 && !complete_cheats_unlocked())
            colour = GRAPHICS_RGB(67, 76, 67);

        if (item == 4) {
            graphics_draw_fnaf3_menu_text(GRAPHICS_TARGET_BOTH,
                                          190, y - 2, 2,
                                          items[item], colour);
        } else {
            graphics_draw_text(GRAPHICS_TARGET_BOTH,
                               190, y,
                               item == 2 ? 3 : 4,
                               items[item], colour);
        }
        if (item == 3 && !complete_cheats_unlocked())
            graphics_draw_text(GRAPHICS_TARGET_BOTH,
                               587, y + 6, 1, "LOCKED",
                               GRAPHICS_RGB(107, 63, 58));
    }

    if (sCompleteCheats.locked_notice_frames > 0u)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 166, 437, 1,
                           "COMPLETE NIGHT 6 AND THE GOOD ENDING",
                           COLOUR_RED);
    else
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 304, 449, 1,
                           "A: OPEN   B: TITLE",
                           GRAPHICS_RGB(121, 161, 117));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void complete_draw_cheats(void)
{
    static const char *const names[4] = {
        "FAST NIGHTS", "RADAR", "AGGRESSIVE", "NO ERRORS"
    };
    const bool enabled[4] = {
        sCompleteCheats.fast_nights,
        sCompleteCheats.radar,
        sCompleteCheats.aggressive,
        sCompleteCheats.no_errors
    };

    finishing_draw_header("EXTRAS / CHEATS");
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 137, 80, 580, 302, 4,
                        GRAPHICS_RGB(43, 91, 48));
    for (int index = 0; index < 4; ++index) {
        const int y = 119 + index * 62;
        if (index == sCompleteCheats.selection)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                               171, y - 12, 512, 42,
                               GRAPHICS_RGB(24, 61, 29));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 205, y, 3,
                           names[index],
                           index == sCompleteCheats.selection
                               ? COLOUR_WHITE
                               : GRAPHICS_RGB(107, 145, 104));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 583, y + 3, 2,
                           enabled[index] ? "ON" : "OFF",
                           enabled[index] ? COLOUR_GREEN
                                          : GRAPHICS_RGB(105, 86, 80));
    }

    graphics_draw_text(GRAPHICS_TARGET_BOTH, 134, 403, 1,
                       "4TH STAR: COMPLETE NIGHT 6 WITH AGGRESSIVE ONLY",
                       GRAPHICS_RGB(164, 151, 94));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 221, 445, 1,
                       "UP/DOWN: SELECT   A: TOGGLE   B: BACK",
                       GRAPHICS_RGB(121, 161, 117));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static bool complete_render_extras(Game *game)
{
    if (game->screen != SCREEN_PLACEHOLDER ||
        sFinishing.mode != FINISHING_EXTRAS) {
        return false;
    }
    if (sFinishing.extras_page == EXTRAS_PAGE_MAIN) {
        complete_draw_extras_main();
        return true;
    }
    if (sFinishing.extras_page == EXTRAS_PAGE_ACHIEVEMENTS) {
        achievement_draw_menu();
        return true;
    }
    if (sFinishing.extras_page == EXTRAS_PAGE_CHEATS) {
        complete_draw_cheats();
        return true;
    }
    return false;
}

static void complete_draw_radar(void)
{
    if (!sCompleteCheats.radar || sCompleteRenderGame == NULL) return;
    const Game *game = sCompleteRenderGame;
    if (game->screen != SCREEN_OFFICE || !game->camera_open ||
        game->vent_map_open) {
        return;
    }

    int camera = -1;
    if (springtrap_ai_is_on_camera(&game->springtrap,
                                   springtrap_ai_camera(&game->springtrap))) {
        camera = springtrap_ai_camera(&game->springtrap);
    } else if (springtrap_ai_is_in_vent(&game->springtrap)) {
        camera = springtrap_vent_source_camera(
            springtrap_ai_current_vent(&game->springtrap));
    }

    if (camera >= 0 && camera < CAMERA_COUNT) {
        const MapPoint point = kCameraMapPoints[camera];
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH,
                              point.x, point.y, 10, 10, COLOUR_RED);
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH,
                              point.x, point.y, 5, 5, COLOUR_WHITE);
    } else if (springtrap_ai_office_side(&game->springtrap)
               != SPRINGTRAP_OFFICE_NONE) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 25, 405, 190, 32,
                           GRAPHICS_RGB(4, 8, 5));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 42, 416, 1,
                           "RADAR: OFFICE", COLOUR_RED);
    }
}

static void complete_graphics_present(uint32_t target)
{
    finishing_draw_phone_overlay();
    complete_draw_radar();
    achievement_draw_overlay();
    graphics_present(target);
}
