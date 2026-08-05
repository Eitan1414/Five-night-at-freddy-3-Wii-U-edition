static void secret_draw_chica(void)
{
    static const int cupcakes[4][2] = {
        {125, 116}, {125, 204}, {125, 292}, {125, 380}
    };
    static const int children[4][2] = {
        {360, 117}, {510, 213}, {660, 309}, {772, 397}
    };
    secret_draw_header("CHICAS PARTY", "COLLECT CUPCAKES AND GIVE ONE TO EACH CHILD");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382, GRAPHICS_RGB(44, 168, 174));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 70, 72, 115, 350, GRAPHICS_RGB(42, 105, 49));
    for (int index = 0; index < 4; ++index) {
        const uint8_t pickup = (uint8_t) (1u << index);
        const uint8_t delivered = (uint8_t) (1u << (index + 4));
        if ((sSecret.collected & pickup) == 0u)
            content_draw_cupcake(cupcakes[index][0], cupcakes[index][1]);
        content_draw_pixel_person(children[index][0], children[index][1],
                                  (sSecret.collected & delivered) != 0u
                                      ? GRAPHICS_RGB(85, 175, 89)
                                      : GRAPHICS_RGB(45, 58, 84),
                                  COLOUR_WHITE, false);
    }
    content_draw_pixel_person(sSecret.x, sSecret.y, GRAPHICS_RGB(235, 205, 45),
                              COLOUR_WHITE, false);
    char carried[32];
    snprintf(carried, sizeof(carried), "CUPCAKES: %d", sSecret.carried);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 640, 22, 2, carried, GRAPHICS_RGB(235, 205, 45));
}

static void secret_draw_stage01(void)
{
    static const int children[3][2] = {
        {235, 125}, {446, 310}, {680, 122}
    };
    secret_draw_header("STAGE 01", "FIND THREE CHILDREN, THEN TAKE THE CAKE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382, GRAPHICS_RGB(46, 171, 176));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 35, 338, 785, 90, GRAPHICS_RGB(131, 64, 39));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 700, 70, 120, 270, GRAPHICS_RGB(131, 64, 39));
    for (int index = 0; index < 3; ++index)
        if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
            content_draw_pixel_person(children[index][0], children[index][1],
                                      GRAPHICS_RGB(41, 68, 58), COLOUR_WHITE, false);
    content_draw_cake(757, 378, true);
    content_draw_pixel_person(sSecret.x, sSecret.y, GRAPHICS_RGB(206, 156, 31),
                              COLOUR_WHITE, true);
}

static void secret_draw_shadow(void)
{
    static const int targets[4][2] = {
        {718, 109}, {171, 114}, {708, 360}, {171, 360}
    };
    static const uint32_t rooms[4] = {
        GRAPHICS_RGB(43, 13, 55), GRAPHICS_RGB(75, 24, 86),
        GRAPHICS_RGB(27, 10, 38), GRAPHICS_RGB(90, 34, 104)
    };
    secret_draw_header("SHADOW BONNIE", "A OR R: PHASE ROOMS  -  TOUCH EACH SHADOW CAKE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382, rooms[sSecret.room]);
    for (int x = 75; x < 830; x += 115)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 95, 8, 310, GRAPHICS_RGB(13, 7, 20));
    const int room = sSecret.room;
    if ((sSecret.collected & (uint8_t) (1u << room)) == 0u)
        content_draw_cake(targets[room][0], targets[room][1], false);
    content_draw_pixel_person(sSecret.x, sSecret.y, GRAPHICS_RGB(28, 4, 39),
                              GRAPHICS_RGB(172, 64, 207), true);
    char room_text[24];
    snprintf(room_text, sizeof(room_text), "ROOM %d/4", room + 1);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 690, 22, 2, room_text,
                       GRAPHICS_RGB(195, 92, 220));
}

static void secret_draw_happiest(void)
{
    static const int children[4][2] = {
        {232, 354}, {374, 354}, {516, 354}, {658, 354}
    };
    static const uint32_t child_colours[4] = {
        GRAPHICS_RGB(91, 165, 79), GRAPHICS_RGB(208, 62, 69),
        GRAPHICS_RGB(72, 119, 202), GRAPHICS_RGB(232, 152, 54)
    };
    secret_draw_header("HAPPIEST DAY", "VISIT EVERY CHILD, THEN JOIN THE CAKE TABLE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382, GRAPHICS_RGB(48, 170, 175));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 50, 390, 754, 26, GRAPHICS_RGB(38, 38, 38));
    for (int index = 0; index < 4; ++index) {
        content_draw_pixel_person(children[index][0], children[index][1],
                                  child_colours[index], COLOUR_WHITE, true);
        if ((sSecret.collected & (uint8_t) (1u << index)) != 0u)
            content_draw_balloon(children[index][0], 280, child_colours[index]);
    }
    content_draw_cake(760, 210, sSecret.delivered >= 4);
    content_draw_pixel_person(sSecret.x, sSecret.y, GRAPHICS_RGB(214, 214, 214),
                              COLOUR_WHITE, false);
}

static void secret_draw_good_ending(void)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(2, 1, 3));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 246, 55, 5, "GOOD ENDING", COLOUR_WHITE);
    static const int xs[4] = {152, 337, 522, 707};
    for (int index = 0; index < 4; ++index) {
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, xs[index], 214, 70, 84,
                              GRAPHICS_RGB(27, 27, 31));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, xs[index] - 35, 201, 22, 8,
                           GRAPHICS_RGB(5, 5, 7));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, xs[index] + 13, 201, 22, 8,
                           GRAPHICS_RGB(5, 5, 7));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, xs[index] - 40, 246, 80, 7,
                           GRAPHICS_RGB(47, 47, 52));
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 232, 340, 3,
                       "THE CHILDREN ARE FREE", GRAPHICS_RGB(170, 170, 180));
    if (sSecret.completed_frames >= 150u)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 282, 432, 1,
                           "A: RETURN TO EXTRAS", COLOUR_WHITE);
}

static void secret_draw_active(void)
{
    if (sSecret.good_ending) {
        secret_draw_good_ending();
        graphics_present(GRAPHICS_TARGET_BOTH);
        return;
    }
    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: secret_draw_bb(); break;
        case SECRET_MINIGAME_MANGLE: secret_draw_mangle(); break;
        case SECRET_MINIGAME_CHICA: secret_draw_chica(); break;
        case SECRET_MINIGAME_STAGE01: secret_draw_stage01(); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: secret_draw_shadow(); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: secret_draw_happiest(); break;
        default: break;
    }
    if (sSecret.completed) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 171, 176, 512, 132, GRAPHICS_RGB(4, 4, 4));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 171, 176, 512, 132, 4, COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 245, 205, 4,
                           "MINIGAME COMPLETE", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 280, 267, 1,
                           "PROGRESS SAVED - A TO RETURN", COLOUR_GREEN);
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void secret_draw_menu(void)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(2, 8, 5));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 58, GRAPHICS_RGB(8, 25, 12));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 28, 19, 3,
                       "EXTRAS / SECRET MINIGAMES", COLOUR_GREEN);
    for (int index = 0; index < SECRET_MINIGAME_COUNT; ++index) {
        const int column = index / 3;
        const int row = index % 3;
        const int x = 56 + column * 402;
        const int y = 92 + row * 103;
        if (index == sSecret.selection) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 342, 72,
                               GRAPHICS_RGB(30, 70, 35));
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, y, 342, 72, 3,
                                GRAPHICS_RGB(128, 202, 122));
        } else {
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, y, 342, 72, 2,
                                GRAPHICS_RGB(52, 90, 55));
        }
        graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 17, y + 19, 2,
                           kSecretMinigameNames[index],
                           index == sSecret.selection ? COLOUR_WHITE : GRAPHICS_RGB(123, 151, 122));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 292, y + 45, 1,
                           progress_save_is_secret_minigame_completed(index + 1)
                               ? "DONE" : "NEW",
                           progress_save_is_secret_minigame_completed(index + 1)
                               ? COLOUR_GREEN : GRAPHICS_RGB(184, 137, 72));
    }
    if (progress_save_good_ending_unlocked())
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 257, 409, 2,
                           "GOOD ENDING UNLOCKED", COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 260, 450, 1,
                       "D-PAD: SELECT   A: PLAY   B: BACK", GRAPHICS_RGB(121, 161, 117));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void content_draw_extras_main(void)
{
    static const char *const items[4] = {
        "ANIMATRONICS", "JUMPSCARES", "SECRET MINIGAMES", "CREDITS"
    };
    finishing_draw_header("EXTRAS");
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 126, 82, 602, 326, 4,
                        GRAPHICS_RGB(43, 91, 48));
    for (int item = 0; item < 4; ++item) {
        const int y = 123 + item * 67;
        if (item == sFinishing.extras_selection)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 166, y - 13, 522, 44,
                               GRAPHICS_RGB(24, 61, 29));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 205, y, item == 2 ? 3 : 4,
                           items[item],
                           item == sFinishing.extras_selection
                               ? COLOUR_WHITE : GRAPHICS_RGB(107, 145, 104));
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 304, 439, 1,
                       "A: OPEN   B: TITLE", GRAPHICS_RGB(121, 161, 117));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void update_game(Game *game, const InputState *input)
{
    if (sSecret.active) {
        secret_update_active(game, input);
        return;
    }
    if (game->screen == SCREEN_PLACEHOLDER &&
        sFinishing.mode == FINISHING_EXTRAS &&
        sFinishing.extras_page == EXTRAS_PAGE_MINIGAMES) {
        secret_update_menu(game, input);
        return;
    }

    const GameScreen before_screen = game->screen;
    const uint32_t before_frames = game->screen_frames;
    const int before_title_selection = game->title_selection;
    const bool before_camera = game->camera_open;
    const bool before_maintenance = game->maintenance_open;
    const int before_maintenance_selection = game->maintenance_selection;
    const FinishingMode before_mode = sFinishing.mode;
    const ExtrasPage before_page = sFinishing.extras_page;
    const int before_extras_selection = sFinishing.extras_selection;
    const int before_extras_index = sFinishing.extras_index;

    fnaf3_finishing_update_game(game, input);

    if (before_title_selection != game->title_selection ||
        before_maintenance_selection != game->maintenance_selection ||
        before_extras_selection != sFinishing.extras_selection ||
        before_extras_index != sFinishing.extras_index ||
        (before_page != sFinishing.extras_page &&
         sFinishing.extras_page != EXTRAS_PAGE_MINIGAMES)) {
        content_play_select();
    }

    if (!before_camera && game->camera_open)
        audio_play(AUDIO_CUE_CAMERA_OPEN, 0.82f, false);
    else if (before_camera && !game->camera_open)
        audio_play(AUDIO_CUE_CAMERA_CLOSE, 0.82f, false);

    if (!before_maintenance && game->maintenance_open)
        audio_play(AUDIO_CUE_MAINTENANCE_OPEN, 0.82f, false);
    else if (before_maintenance && !game->maintenance_open)
        audio_play(AUDIO_CUE_MAINTENANCE_CLOSE, 0.82f, false);

    if (before_screen != game->screen) {
        if (before_screen == SCREEN_TITLE) content_stop_title_music();
        if (before_screen == SCREEN_GAME_OVER)
            audio_stop(AUDIO_CUE_GAME_OVER_AMBIENCE);

        if (game->screen == SCREEN_TITLE) {
            audio_stop(AUDIO_CUE_GAME_OVER_AMBIENCE);
            content_start_title_music();
        } else if (game->screen == SCREEN_NIGHT_INTRO) {
            audio_play(AUDIO_CUE_START_DAY, 0.82f, false);
        } else if (game->screen == SCREEN_GAME_OVER) {
            content_stop_title_music();
            audio_restart(AUDIO_CUE_GAME_OVER_AMBIENCE, 0.68f, true);
        }
    }

    if (before_mode != FINISHING_EXTRAS && sFinishing.mode == FINISHING_EXTRAS)
        content_start_title_music();

    if (before_screen == SCREEN_VICTORY && game->screen == SCREEN_VICTORY &&
        before_frames < 180u && game->screen_frames >= 180u) {
        audio_play(AUDIO_CUE_END, 0.82f, false);
    }

    if (before_page != EXTRAS_PAGE_MINIGAMES &&
        sFinishing.extras_page == EXTRAS_PAGE_MINIGAMES) {
        memset(&sSecret, 0, sizeof(sSecret));
        game->dirty = true;
    }
}

static void render_game(Game *game)
{
    if (!game->dirty) return;
    if (sSecret.active) {
        game->dirty = false;
        secret_draw_active();
        return;
    }
    if (game->screen == SCREEN_PLACEHOLDER &&
        sFinishing.mode == FINISHING_EXTRAS) {
        if (sFinishing.extras_page == EXTRAS_PAGE_MINIGAMES) {
            game->dirty = false;
            secret_draw_menu();
            return;
        }
        if (sFinishing.extras_page == EXTRAS_PAGE_MAIN) {
            game->dirty = false;
            content_draw_extras_main();
            return;
        }
    }
    fnaf3_finishing_render_game(game);
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
    memset(&sSecret, 0, sizeof(sSecret));
    sTitleMusicPlaying = false;
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
