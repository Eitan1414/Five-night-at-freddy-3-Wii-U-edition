static const uint32_t kPhoneDurations[NIGHT_COUNT] = {
    12200u, 9620u, 5704u, 4592u, 4060u, 3140u
};

static AudioCue finishing_phone_cue(int night)
{
    if (night < 1) night = 1;
    if (night > NIGHT_COUNT) night = NIGHT_COUNT;
    return (AudioCue) (AUDIO_CUE_PHONE_NIGHT_1 + night - 1);
}

static int finishing_abs(int value)
{
    return value < 0 ? -value : value;
}

static void finishing_stop_phone(void)
{
    if (sFinishing.phone_night >= 1 && sFinishing.phone_night <= NIGHT_COUNT)
        audio_stop(finishing_phone_cue(sFinishing.phone_night));
    sFinishing.phone_active = false;
    sFinishing.phone_frames = 0u;
    sFinishing.phone_duration = 0u;
}

static void finishing_start_phone(const Game *game)
{
    finishing_stop_phone();
    if (game == NULL || game->night < 1 || game->night > NIGHT_COUNT) return;
    sFinishing.phone_night = game->night;
    sFinishing.phone_active = true;
    sFinishing.phone_muted = false;
    sFinishing.phone_frames = 0u;
    sFinishing.phone_duration = kPhoneDurations[game->night - 1];
    audio_play(finishing_phone_cue(game->night), 0.76f, false);
}

static void finishing_draw_phone_overlay(void)
{
    if (!sFinishing.phone_active) return;
    const uint32_t target = GRAPHICS_TARGET_BOTH;
    const uint32_t border = sFinishing.phone_muted
        ? GRAPHICS_RGB(103, 107, 101) : GRAPHICS_RGB(145, 196, 126);
    graphics_draw_rect(target, 548, 51, 282, 78, GRAPHICS_RGB(2, 8, 4));
    graphics_draw_frame(target, 548, 51, 282, 78, 3, border);
    graphics_draw_text(target, 570, 70, 2,
                       sFinishing.phone_night <= 2 ? "PHONE DUDE" : "PHONE GUY",
                       border);
    graphics_draw_text(target, 570, 101, 1,
                       sFinishing.phone_muted ? "CALL MUTED" : "R: MUTE CALL",
                       sFinishing.phone_muted ? GRAPHICS_RGB(118, 121, 117) : COLOUR_WHITE);
}

static void finishing_graphics_present(uint32_t target)
{
    finishing_draw_phone_overlay();
    graphics_present(target);
}

static void finishing_return_to_title(Game *game)
{
    finishing_stop_phone();
    sFinishing.mode = FINISHING_NONE;
    sFinishing.extras_page = EXTRAS_PAGE_MAIN;
    game->screen = SCREEN_TITLE;
    game->screen_frames = 0u;
    game->title_selection = 0;
    game->dirty = true;
}

static void finishing_open_extras(Game *game)
{
    audio_stop_all();
    finishing_stop_phone();
    sFinishing.mode = FINISHING_EXTRAS;
    sFinishing.extras_page = EXTRAS_PAGE_MAIN;
    sFinishing.extras_selection = 0;
    sFinishing.extras_index = 0;
    game->screen = SCREEN_PLACEHOLDER;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void finishing_start_minigame(Game *game, int chapter, bool replay)
{
    if (chapter < 1) chapter = 1;
    if (chapter > 5) chapter = 5;
    audio_stop_all();
    finishing_stop_phone();
    sFinishing.mode = FINISHING_FOLLOW_ME;
    sFinishing.minigame_chapter = chapter;
    sFinishing.minigame_x = 86;
    sFinishing.minigame_y = 382;
    sFinishing.minigame_waypoint = 0;
    sFinishing.minigame_event_frames = 0u;
    sFinishing.minigame_replay = replay;
    game->screen = SCREEN_PLACEHOLDER;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void finishing_start_ending(Game *game, FinishingMode mode)
{
    audio_stop_all();
    finishing_stop_phone();
    sFinishing.mode = mode;
    game->screen = SCREEN_PLACEHOLDER;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void finishing_handle_title(Game *game, const InputState *input)
{
    ++game->screen_frames;
    if (sFinishing.title_notice_frames > 0u) --sFinishing.title_notice_frames;
    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        game->title_selection = (game->title_selection + TITLE_ITEM_COUNT - 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        game->title_selection = (game->title_selection + 1) % TITLE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        if (game->title_selection == 0) {
            sFinishing.mode = FINISHING_NONE;
            finishing_stop_phone();
            reset_night(game, 1);
        } else if (game->title_selection == 1) {
            sFinishing.mode = FINISHING_NONE;
            finishing_stop_phone();
            reset_night(game, progress_save_highest_unlocked_night());
        } else if (progress_save_extras_unlocked()) {
            finishing_open_extras(game);
        } else {
            sFinishing.title_notice_frames = 180u;
            game->dirty = true;
        }
    }
    if ((game->screen_frames % 9u) == 0u || sFinishing.title_notice_frames > 0u)
        game->dirty = true;
}

static void finishing_update_victory(Game *game, const InputState *input)
{
    ++game->screen_frames;
    if ((game->screen_frames % 6u) == 0u) game->dirty = true;
    if (game->screen_frames < 180u) return;
    if (!input_was_pressed(input, GAME_BUTTON_CONFIRM) &&
        !input_was_pressed(input, GAME_BUTTON_START)) return;

    if (game->night <= 5) finishing_start_minigame(game, game->night, false);
    else finishing_start_ending(game, FINISHING_NIGHT6_ENDING);
}

static void finishing_change_extras_page(Game *game, ExtrasPage page)
{
    sFinishing.extras_page = page;
    sFinishing.extras_selection = 0;
    sFinishing.extras_index = 0;
    game->screen_frames = 0u;
    game->dirty = true;
}

static void finishing_update_extras(Game *game, const InputState *input)
{
    if (sFinishing.extras_page == EXTRAS_PAGE_MAIN) {
        if (input_was_pressed(input, GAME_BUTTON_UP)) {
            sFinishing.extras_selection = (sFinishing.extras_selection + 3) % 4;
            game->dirty = true;
        }
        if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
            sFinishing.extras_selection = (sFinishing.extras_selection + 1) % 4;
            game->dirty = true;
        }
        if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(input, GAME_BUTTON_START)) {
            static const ExtrasPage pages[4] = {
                EXTRAS_PAGE_GALLERY, EXTRAS_PAGE_JUMPSCARES,
                EXTRAS_PAGE_MINIGAMES, EXTRAS_PAGE_CREDITS
            };
            finishing_change_extras_page(game, pages[sFinishing.extras_selection]);
        } else if (input_was_pressed(input, GAME_BUTTON_BACK)) {
            finishing_return_to_title(game);
        }
        return;
    }

    if (input_was_pressed(input, GAME_BUTTON_BACK)) {
        finishing_change_extras_page(game, EXTRAS_PAGE_MAIN);
        return;
    }

    int count = 1;
    if (sFinishing.extras_page == EXTRAS_PAGE_GALLERY) count = 7;
    else if (sFinishing.extras_page == EXTRAS_PAGE_JUMPSCARES) count = 6;
    else if (sFinishing.extras_page == EXTRAS_PAGE_MINIGAMES) count = 5;

    if (input_was_pressed(input, GAME_BUTTON_LEFT)) {
        sFinishing.extras_index = (sFinishing.extras_index + count - 1) % count;
        game->screen_frames = 0u;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_RIGHT)) {
        sFinishing.extras_index = (sFinishing.extras_index + 1) % count;
        game->screen_frames = 0u;
        game->dirty = true;
    }
    if (sFinishing.extras_page == EXTRAS_PAGE_MINIGAMES &&
        (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
         input_was_pressed(input, GAME_BUTTON_START))) {
        const int chapter = sFinishing.extras_index + 1;
        if (progress_save_is_night_completed(chapter))
            finishing_start_minigame(game, chapter, true);
    }
    if ((game->screen_frames % 6u) == 0u) game->dirty = true;
}

static void finishing_complete_minigame(Game *game)
{
    const int chapter = sFinishing.minigame_chapter;
    if (sFinishing.minigame_replay) {
        finishing_open_extras(game);
        sFinishing.extras_page = EXTRAS_PAGE_MINIGAMES;
        sFinishing.extras_index = chapter - 1;
        return;
    }
    if (chapter < 5) {
        sFinishing.mode = FINISHING_NONE;
        reset_night(game, chapter + 1);
    } else {
        finishing_start_ending(game, FINISHING_NIGHT5_ENDING);
    }
}

static void finishing_update_minigame(Game *game, const InputState *input)
{
    static const int waypoints[4][2] = {
        {206, 360}, {342, 258}, {537, 177}, {721, 101}
    };

    if (sFinishing.minigame_replay && input_was_pressed(input, GAME_BUTTON_BACK)) {
        finishing_open_extras(game);
        sFinishing.extras_page = EXTRAS_PAGE_MINIGAMES;
        sFinishing.extras_index = sFinishing.minigame_chapter - 1;
        return;
    }

    if (sFinishing.minigame_event_frames > 0u) {
        ++sFinishing.minigame_event_frames;
        game->dirty = true;
        if (sFinishing.minigame_event_frames >= 210u)
            finishing_complete_minigame(game);
        return;
    }

    if (input_is_held(input, GAME_BUTTON_LEFT)) sFinishing.minigame_x -= 4;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) sFinishing.minigame_x += 4;
    if (input_is_held(input, GAME_BUTTON_UP)) sFinishing.minigame_y -= 4;
    if (input_is_held(input, GAME_BUTTON_DOWN)) sFinishing.minigame_y += 4;
    if (sFinishing.minigame_x < 45) sFinishing.minigame_x = 45;
    if (sFinishing.minigame_x > 807) sFinishing.minigame_x = 807;
    if (sFinishing.minigame_y < 72) sFinishing.minigame_y = 72;
    if (sFinishing.minigame_y > 426) sFinishing.minigame_y = 426;

    const int waypoint = sFinishing.minigame_waypoint;
    if (waypoint < 4 &&
        finishing_abs(sFinishing.minigame_x - waypoints[waypoint][0]) < 28 &&
        finishing_abs(sFinishing.minigame_y - waypoints[waypoint][1]) < 28) {
        ++sFinishing.minigame_waypoint;
        if (sFinishing.minigame_waypoint >= 4)
            sFinishing.minigame_event_frames = 1u;
    }
    game->dirty = true;
}

static void finishing_update_ending(Game *game, const InputState *input)
{
    if ((game->screen_frames % 5u) == 0u) game->dirty = true;
    if (game->screen_frames >= 180u &&
        (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
         input_was_pressed(input, GAME_BUTTON_START) ||
         input_was_pressed(input, GAME_BUTTON_BACK))) {
        finishing_return_to_title(game);
    }
}

static void update_game(Game *game, const InputState *input)
{
    if (game->screen == SCREEN_TITLE) {
        finishing_handle_title(game, input);
        return;
    }
    if (game->screen == SCREEN_VICTORY) {
        finishing_update_victory(game, input);
        return;
    }
    if (game->screen == SCREEN_PLACEHOLDER && sFinishing.mode != FINISHING_NONE) {
        ++game->screen_frames;
        if (sFinishing.mode == FINISHING_EXTRAS)
            finishing_update_extras(game, input);
        else if (sFinishing.mode == FINISHING_FOLLOW_ME)
            finishing_update_minigame(game, input);
        else
            finishing_update_ending(game, input);
        return;
    }

    InputState filtered = *input;
    if (game->screen == SCREEN_OFFICE && sFinishing.phone_active &&
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        audio_stop(finishing_phone_cue(sFinishing.phone_night));
        sFinishing.phone_active = false;
        sFinishing.phone_muted = true;
        filtered.pressed &= ~(uint32_t) GAME_BUTTON_MODE;
        filtered.held &= ~(uint32_t) GAME_BUTTON_MODE;
        game->dirty = true;
    }

    const GameScreen before = game->screen;
    fnaf3_legacy_update_game(game, &filtered);

    if (before == SCREEN_NIGHT_INTRO && game->screen == SCREEN_OFFICE)
        finishing_start_phone(game);

    if (game->screen == SCREEN_OFFICE && sFinishing.phone_active) {
        ++sFinishing.phone_frames;
        if (sFinishing.phone_frames >= sFinishing.phone_duration)
            finishing_stop_phone();
        else if ((sFinishing.phone_frames % 30u) == 0u)
            game->dirty = true;
    }

    if (before == SCREEN_OFFICE && game->screen == SCREEN_VICTORY) {
        finishing_stop_phone();
        audio_play(AUDIO_CUE_SIX_AM, 0.92f, false);
    } else if (before == SCREEN_OFFICE && game->screen != SCREEN_OFFICE) {
        finishing_stop_phone();
    }
}

static void finishing_draw_title(const Game *game)
{
    static const char *const items[TITLE_ITEM_COUNT] = {"NEW GAME", "LOAD GAME", "EXTRAS"};
    const int frame = title_frame_index(game);
    const int glitch = frame == 0 ? (int) ((game->seed >> 8) % 7u) - 3 : (frame - 2) * 3;
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 410, 480, GRAPHICS_RGB(2, 7, 4));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 47, 7, "F", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 119, 7, "N", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 191, 7, "A", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 263, 7, "F", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 349, 8, "3", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 120, 72, 2, "FIVE NIGHTS AT FREDDYS 3", COLOUR_GREEN);
    char continue_text[32];
    snprintf(continue_text, sizeof(continue_text), "CURRENT NIGHT: %d", game->unlocked_night);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 119, 1, continue_text, GRAPHICS_RGB(74, 111, 73));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 145, 1,
                       progress_save_load_status_text(),
                       progress_save_load_status_is_error() ? COLOUR_RED : GRAPHICS_RGB(74, 111, 73));
    if (progress_save_is_night_completed(5))
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 320, 40, 4, "*", GRAPHICS_RGB(225, 219, 103));
    if (progress_save_is_night_completed(6))
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 352, 40, 4, "*", GRAPHICS_RGB(225, 219, 103));
    for (int item = 0; item < TITLE_ITEM_COUNT; ++item) {
        const int y = 284 + item * 48;
        if (item == game->title_selection) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 262, 36, GRAPHICS_RGB(31, 72, 37));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 5, 36, GRAPHICS_RGB(129, 207, 121));
        }
        uint32_t colour = item == game->title_selection ? COLOUR_WHITE : GRAPHICS_RGB(112, 139, 113);
        if (item == 2 && !progress_save_extras_unlocked()) colour = GRAPHICS_RGB(70, 78, 70);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 124, y, 3, items[item], colour);
        if (item == 2 && !progress_save_extras_unlocked())
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 300, y + 5, 1, "LOCKED", GRAPHICS_RGB(91, 67, 62));
    }
    if (sFinishing.title_notice_frames > 0u)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 112, 445, 1,
                           "COMPLETE NIGHT 5 TO UNLOCK EXTRAS", COLOUR_RED);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 410, 0, 444, 480, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 410 + glitch, 16, 444, 448, kTitleFrames[frame]);
    draw_scanlines(GRAPHICS_TARGET_BOTH, game->seed + game->screen_frames);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void finishing_draw_victory(const Game *game)
{
    const bool six = game->screen_frames >= 90u;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, six ? 301 : 275, 145, 8,
                       six ? "6:00" : "5:59",
                       six ? GRAPHICS_RGB(224, 222, 93) : COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 577, 193, 4, "AM", COLOUR_GREEN);
    if (game->screen_frames >= 180u) {
        char text[32];
        snprintf(text, sizeof(text), "NIGHT %d COMPLETE", game->night);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 248, 305, 3, text, COLOUR_GREEN);
        if (progress_save_write_attempted())
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 304, 346, 1,
                               progress_save_write_status_text(),
                               progress_save_last_write_ok() ? COLOUR_GREEN : COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 226, 391, 2,
                           game->night <= 5 ? "A: FOLLOW THE SHADOW" : "A: VIEW FINAL ENDING",
                           GRAPHICS_RGB(104, 151, 101));
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void finishing_draw_header(const char *title)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 5, 3));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 54, GRAPHICS_RGB(7, 20, 10));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 35, 20, 3, title, COLOUR_GREEN);
}

static void finishing_draw_extras_main(void)
{
    static const char *const items[4] = {
        "ANIMATRONICS", "JUMPSCARES", "FOLLOW ME", "CREDITS"
    };
    finishing_draw_header("EXTRAS");
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 126, 82, 602, 326, 4, GRAPHICS_RGB(43, 91, 48));
    for (int item = 0; item < 4; ++item) {
        const int y = 123 + item * 67;
        if (item == sFinishing.extras_selection)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 166, y - 13, 522, 44, GRAPHICS_RGB(24, 61, 29));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 205, y, 4, items[item],
                           item == sFinishing.extras_selection ? COLOUR_WHITE : GRAPHICS_RGB(107, 145, 104));
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 304, 439, 1, "A: OPEN   B: TITLE", GRAPHICS_RGB(121, 161, 117));
}

static void finishing_draw_texture_fit(const TextureRle *texture,
                                       int x, int y, int width, int height)
{
    if (texture == NULL || texture->width == 0u || texture->height == 0u) return;
    int draw_width = width;
    int draw_height = (int) ((uint32_t) texture->height * (uint32_t) width / texture->width);
    if (draw_height > height) {
        draw_height = height;
        draw_width = (int) ((uint32_t) texture->width * (uint32_t) height / texture->height);
    }
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     x + (width - draw_width) / 2,
                     y + (height - draw_height) / 2,
                     draw_width, draw_height, texture);
}

static void finishing_draw_gallery(const Game *game)
{
    static const char *const names[7] = {
        "SPRINGTRAP", "PHANTOM FOXY", "PHANTOM FREDDY", "PHANTOM CHICA",
        "PHANTOM BB", "PHANTOM MANGLE", "PHANTOM PUPPET"
    };
    const TextureRle *texture = NULL;
    switch (sFinishing.extras_index) {
        case 0: texture = kTitleFrames[(game->screen_frames / 10u) % TITLE_FRAME_COUNT]; break;
        case 1: texture = &gPhantomFoxyOfficeTexture; break;
        case 2: texture = &gPhantomFreddyOfficeTexture; break;
        case 3: texture = &gPhantomChicaOfficeTexture; break;
        case 4: texture = &gPhantomBBOfficeTexture; break;
        case 5: texture = &gPhantomMangleOfficeTexture; break;
        default: texture = &gPhantomPuppetOfficeTexture; break;
    }
    finishing_draw_header("EXTRAS / ANIMATRONICS");
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 109, 73, 636, 325, 3, GRAPHICS_RGB(45, 88, 48));
    finishing_draw_texture_fit(texture, 135, 88, 584, 285);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 177, 365, 500, 37, GRAPHICS_RGB(3, 10, 5));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 260, 379, 3,
                       names[sFinishing.extras_index], COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 235, 438, 1,
                       "LEFT/RIGHT: CHARACTER   B: BACK", GRAPHICS_RGB(121, 161, 117));
}

static const JumpscareSequence *finishing_extra_jumpscare(int index)
{
    switch (index) {
        case 0: return &gSpringtrapJumpscareLeft;
        case 1: return &gSpringtrapJumpscareRight;
        case 2: return &gPhantomBBRealJumpscare;
        case 3: return &gPhantomChicaRealJumpscare;
        case 4: return &gPhantomFreddyRealJumpscare;
        default: return &gPhantomFoxyRealJumpscare;
    }
}

static void finishing_draw_jumpscare_extra(const Game *game)
{
    static const char *const names[6] = {
        "SPRINGTRAP LEFT", "SPRINGTRAP RIGHT", "PHANTOM BB",
        "PHANTOM CHICA", "PHANTOM FREDDY", "PHANTOM FOXY"
    };
    const JumpscareSequence *sequence = finishing_extra_jumpscare(sFinishing.extras_index);
    uint32_t duration = sequence->frame_count * sequence->ticks_per_frame;
    if (duration == 0u) duration = 1u;
    draw_jumpscare_sequence(GRAPHICS_TARGET_BOTH, sequence, game->screen_frames % duration);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 55, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 28, 20, 2,
                       names[sFinishing.extras_index], COLOUR_GREEN);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 426, 854, 54, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 217, 447, 1,
                       "LEFT/RIGHT: JUMPSCARE   B: BACK", COLOUR_WHITE);
}

static void finishing_draw_minigame_menu(void)
{
    const int chapter = sFinishing.extras_index + 1;
    finishing_draw_header("EXTRAS / FOLLOW ME");
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 144, 96, 566, 269, 4, GRAPHICS_RGB(49, 90, 52));
    char label[32];
    snprintf(label, sizeof(label), "FOLLOW ME - CHAPTER %d", chapter);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 212, 147, 4, label,
                       progress_save_is_night_completed(chapter) ? COLOUR_WHITE : GRAPHICS_RGB(77, 82, 77));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 274, 232, 3,
                       progress_save_is_night_completed(chapter) ? "UNLOCKED" : "LOCKED",
                       progress_save_is_night_completed(chapter) ? COLOUR_GREEN : COLOUR_RED);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 177, 316, 2,
                       progress_save_is_night_completed(chapter) ? "A: REPLAY" : "COMPLETE THE MATCHING NIGHT",
                       GRAPHICS_RGB(124, 160, 119));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 226, 438, 1,
                       "LEFT/RIGHT: CHAPTER   B: BACK", GRAPHICS_RGB(121, 161, 117));
}

static void finishing_draw_credits(void)
{
    finishing_draw_header("EXTRAS / CREDITS");
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 182, 105, 3, "ORIGINAL GAME", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 464, 105, 3, "SCOTT CAWTHON", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 182, 169, 3, "PSX PORT", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 464, 169, 3, "SOEIZ", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 182, 233, 3, "WII U EDITION", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 464, 233, 3, "EITAN1414", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 182, 297, 3, "ENGINE", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 464, 297, 3, "WUT + SDL2/GX2", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 299, 438, 1, "B: BACK", GRAPHICS_RGB(121, 161, 117));
}

static uint32_t finishing_player_colour(int chapter)
{
    static const uint32_t colours[5] = {
        GRAPHICS_RGB(126, 73, 45), GRAPHICS_RGB(93, 79, 151),
        GRAPHICS_RGB(183, 154, 52), GRAPHICS_RGB(151, 51, 47),
        GRAPHICS_RGB(215, 221, 215)
    };
    if (chapter < 1) chapter = 1;
    if (chapter > 5) chapter = 5;
    return colours[chapter - 1];
}

static void finishing_draw_pixel_character(int x, int y, uint32_t colour, bool shadow)
{
    const uint32_t target = GRAPHICS_TARGET_BOTH;
    graphics_draw_rect(target, x - 8, y - 20, 16, 13, colour);
    graphics_draw_rect(target, x - 11, y - 7, 22, 22, colour);
    graphics_draw_rect(target, x - 13, y + 15, 9, 13, colour);
    graphics_draw_rect(target, x + 4, y + 15, 9, 13, colour);
    graphics_draw_rect(target, x - 12, y - 26, 7, 8, colour);
    graphics_draw_rect(target, x + 5, y - 26, 7, 8, colour);
    if (!shadow) {
        graphics_draw_rect(target, x - 5, y - 15, 3, 3, COLOUR_WHITE);
        graphics_draw_rect(target, x + 3, y - 15, 3, 3, COLOUR_WHITE);
    }
}

static void finishing_draw_minigame(const Game *game)
{
    static const int waypoints[4][2] = {
        {206, 360}, {342, 258}, {537, 177}, {721, 101}
    };
    (void) game;
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(15, 8, 21));
    for (int x = 32; x < 822; x += 64)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 58, 3, 389, GRAPHICS_RGB(45, 30, 55));
    for (int y = 58; y < 448; y += 48)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 32, y, 790, 3, GRAPHICS_RGB(45, 30, 55));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 58, 332, 210, 76, GRAPHICS_RGB(29, 18, 35));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 292, 215, 165, 94, GRAPHICS_RGB(29, 18, 35));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 478, 128, 176, 102, GRAPHICS_RGB(29, 18, 35));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 678, 70, 118, 91, GRAPHICS_RGB(29, 18, 35));
    char chapter[32];
    snprintf(chapter, sizeof(chapter), "FOLLOW ME  %d/5", sFinishing.minigame_chapter);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 24, 20, 2, chapter, GRAPHICS_RGB(187, 80, 188));

    if (sFinishing.minigame_event_frames == 0u) {
        const int waypoint = sFinishing.minigame_waypoint < 4
            ? sFinishing.minigame_waypoint : 3;
        finishing_draw_pixel_character(waypoints[waypoint][0], waypoints[waypoint][1],
                                       GRAPHICS_RGB(112, 44, 148), true);
        finishing_draw_pixel_character(sFinishing.minigame_x, sFinishing.minigame_y,
                                       finishing_player_colour(sFinishing.minigame_chapter), false);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 624, 445, 2, "FOLLOW ME", GRAPHICS_RGB(183, 71, 190));
    } else {
        const uint32_t flash = (sFinishing.minigame_event_frames / 8u) & 1u;
        finishing_draw_pixel_character(690, 125, GRAPHICS_RGB(114, 31, 132), false);
        if (sFinishing.minigame_chapter < 5) {
            finishing_draw_pixel_character(742, 125,
                                           flash ? GRAPHICS_RGB(62, 34, 36)
                                                 : finishing_player_colour(sFinishing.minigame_chapter),
                                           false);
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 720, 159, 12, 7, finishing_player_colour(sFinishing.minigame_chapter));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 750, 166, 14, 7, finishing_player_colour(sFinishing.minigame_chapter));
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 507, 390, 3, "YOU CANT", COLOUR_RED);
        } else {
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, 718, 82, 62, 101, 5,
                                flash ? COLOUR_RED : GRAPHICS_RGB(131, 102, 45));
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 507, 390, 3,
                               flash ? "SPRINGLOCK" : "HE FOLLOWED YOU", COLOUR_RED);
        }
    }
    if (sFinishing.minigame_replay)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 24, 449, 1, "B: RETURN TO EXTRAS", COLOUR_WHITE);
}

static void finishing_draw_night5_ending(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(3, 1, 2));
    const uint32_t glow = ((game->screen_frames / 9u) & 1u)
        ? GRAPHICS_RGB(222, 104, 91) : GRAPHICS_RGB(142, 49, 43);
    static const int xs[4] = {146, 331, 516, 701};
    for (int index = 0; index < 4; ++index) {
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, xs[index], 190, 73, 87, GRAPHICS_RGB(38, 23, 23));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, xs[index] - 37, 177, 24, 10, glow);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, xs[index] + 13, 177, 24, 10, glow);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, xs[index] - 42, 228, 84, 8, GRAPHICS_RGB(70, 46, 43));
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 249, 52, 5, "BAD ENDING", COLOUR_RED);
    if (game->screen_frames >= 90u) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 263, 322, 3, "NIGHT 6 UNLOCKED", COLOUR_GREEN);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 279, 365, 3, "EXTRAS UNLOCKED", COLOUR_GREEN);
    }
    if (game->screen_frames >= 180u)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 286, 437, 1, "A: RETURN TO TITLE", COLOUR_WHITE);
}

static void finishing_draw_night6_ending(const Game *game)
{
    const uint32_t flicker = ((game->screen_frames / 4u) & 1u)
        ? GRAPHICS_RGB(187, 49, 19) : GRAPHICS_RGB(231, 91, 27);
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(22, 5, 2));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 93, 46, 668, 373, GRAPHICS_RGB(205, 197, 166));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 93, 46, 668, 373, 5, GRAPHICS_RGB(52, 39, 30));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 139, 78, 4, "FAZBEARS FRIGHT BURNS DOWN", GRAPHICS_RGB(35, 27, 22));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 131, 135, 592, 5, GRAPHICS_RGB(67, 55, 44));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 138, 170, 335, 180, GRAPHICS_RGB(48, 43, 39));
    for (int x = 148; x < 464; x += 39) {
        const int height = 42 + (int) ((game->screen_frames + (uint32_t) x) % 83u);
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, x, 337 - height / 2, 24, height, flicker);
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 500, 181, 2, "LOCAL ATTRACTION", GRAPHICS_RGB(52, 42, 34));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 500, 216, 2, "DESTROYED OVERNIGHT", GRAPHICS_RGB(52, 42, 34));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 500, 270, 1, "SALVAGED ITEMS WILL", GRAPHICS_RGB(70, 57, 45));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 500, 295, 1, "BE SOLD AT AUCTION", GRAPHICS_RGB(70, 57, 45));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 259, 380, 2, "NIGHT 6 COMPLETE", GRAPHICS_RGB(65, 53, 42));
    if (game->screen_frames >= 180u)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 286, 443, 1, "A: RETURN TO TITLE", COLOUR_WHITE);
}

static void finishing_draw_extras(const Game *game)
{
    switch (sFinishing.extras_page) {
        case EXTRAS_PAGE_MAIN: finishing_draw_extras_main(); break;
        case EXTRAS_PAGE_GALLERY: finishing_draw_gallery(game); break;
        case EXTRAS_PAGE_JUMPSCARES: finishing_draw_jumpscare_extra(game); break;
        case EXTRAS_PAGE_MINIGAMES: finishing_draw_minigame_menu(); break;
        case EXTRAS_PAGE_CREDITS: finishing_draw_credits(); break;
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void render_game(Game *game)
{
    if (!game->dirty) return;
    game->dirty = false;
    if (game->screen == SCREEN_TITLE) {
        finishing_draw_title(game);
        return;
    }
    if (game->screen == SCREEN_VICTORY) {
        finishing_draw_victory(game);
        return;
    }
    if (game->screen == SCREEN_PLACEHOLDER && sFinishing.mode != FINISHING_NONE) {
        if (sFinishing.mode == FINISHING_EXTRAS) finishing_draw_extras(game);
        else if (sFinishing.mode == FINISHING_FOLLOW_ME) {
            finishing_draw_minigame(game);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else if (sFinishing.mode == FINISHING_NIGHT5_ENDING) {
            finishing_draw_night5_ending(game);
            graphics_present(GRAPHICS_TARGET_BOTH);
        } else {
            finishing_draw_night6_ending(game);
            graphics_present(GRAPHICS_TARGET_BOTH);
        }
        return;
    }
    game->dirty = true;
    fnaf3_legacy_render_game(game);
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
    memset(&sFinishing, 0, sizeof(sFinishing));
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
    progress_save_shutdown();
    audio_stop_all();
    audio_shutdown();
    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
