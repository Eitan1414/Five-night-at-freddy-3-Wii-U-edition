typedef struct SecretTriggerState {
    int night;
    uint8_t cupcake_mask;
    uint8_t mangle_step;
    uint8_t stage_step;
    bool return_to_office;
} SecretTriggerState;

static SecretTriggerState sSecretTriggers;

static const uint32_t kTriggerDirectionMask =
    GAME_BUTTON_UP | GAME_BUTTON_DOWN |
    GAME_BUTTON_LEFT | GAME_BUTTON_RIGHT;

static void complete_consume(InputState *input, uint32_t buttons)
{
    if (input == NULL) return;
    input->held &= ~buttons;
    input->pressed &= ~buttons;
    input->released &= ~buttons;
}

static void complete_reset_trigger_sequences(int night)
{
    if (sSecretTriggers.night == night) return;
    memset(&sSecretTriggers, 0, sizeof(sSecretTriggers));
    sSecretTriggers.night = night;
}

static void complete_start_hidden_minigame(Game *game, int selection)
{
    game->camera_open = false;
    game->maintenance_open = false;
    game->vent_map_open = false;
    sSecretTriggers.return_to_office = true;
    secret_start(game, selection);
}

static void complete_return_from_hidden_minigame(Game *game)
{
    const int selection = sSecret.selection;
    memset(&sSecret, 0, sizeof(sSecret));
    sSecret.selection = selection;
    sSecretTriggers.return_to_office = false;
    game->screen = SCREEN_OFFICE;
    game->screen_frames = 0u;
    game->camera_open = false;
    game->maintenance_open = false;
    game->vent_map_open = false;
    game->dirty = true;
    audio_stop_all();
}

static bool complete_handle_hidden_return(Game *game,
                                          const InputState *input)
{
    if (!sSecret.active || !sSecretTriggers.return_to_office) return false;

    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending) {
        complete_return_from_hidden_minigame(game);
        return true;
    }

    if (sSecret.completed && sSecret.completed_frames >= 150u &&
        (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
         input_was_pressed(input, GAME_BUTTON_START) ||
         input_was_pressed(input, GAME_BUTTON_BACK))) {
        complete_return_from_hidden_minigame(game);
        return true;
    }
    return false;
}

static bool complete_try_mangle_trigger(Game *game,
                                        const InputState *input,
                                        InputState *filtered)
{
    if (!game->camera_open || game->vent_map_open ||
        game->selected_camera != 6 || game->night < 2 ||
        !input_is_held(input, GAME_BUTTON_PANEL)) {
        sSecretTriggers.mangle_step = 0u;
        return false;
    }

    complete_consume(filtered, GAME_BUTTON_PANEL | kTriggerDirectionMask);
    if ((input->pressed & kTriggerDirectionMask) == 0u) return true;

    static const uint32_t corners[4] = {
        GAME_BUTTON_UP | GAME_BUTTON_LEFT,
        GAME_BUTTON_DOWN | GAME_BUTTON_LEFT,
        GAME_BUTTON_UP | GAME_BUTTON_RIGHT,
        GAME_BUTTON_DOWN | GAME_BUTTON_RIGHT
    };
    const uint32_t directions = input->held & kTriggerDirectionMask;
    if (directions == corners[sSecretTriggers.mangle_step]) {
        ++sSecretTriggers.mangle_step;
        content_play_select();
        if (sSecretTriggers.mangle_step >= 4u) {
            sSecretTriggers.mangle_step = 0u;
            complete_start_hidden_minigame(game, SECRET_MINIGAME_MANGLE);
        }
    } else {
        sSecretTriggers.mangle_step = 0u;
    }
    return true;
}

static bool complete_try_camera_trigger(Game *game,
                                        const InputState *input,
                                        InputState *filtered)
{
    if (!game->camera_open || game->camera_failed) return false;

    if (complete_try_mangle_trigger(game, input, filtered))
        return sSecret.active;

    if (!input_was_pressed(input, GAME_BUTTON_CONFIRM)) return false;

    const bool seal_held = input_is_held(input, GAME_BUTTON_SEAL);
    const bool mode_held = input_is_held(input, GAME_BUTTON_MODE);

    if (game->selected_camera == 7 && seal_held) {
        complete_consume(filtered, GAME_BUTTON_CONFIRM | GAME_BUTTON_SEAL);
        complete_start_hidden_minigame(game, SECRET_MINIGAME_BB);
        return true;
    }

    if (game->selected_camera == 2 && mode_held) {
        complete_consume(filtered, GAME_BUTTON_CONFIRM | GAME_BUTTON_MODE);
        complete_start_hidden_minigame(game, SECRET_MINIGAME_HAPPIEST_DAY);
        return true;
    }

    if (game->night >= 3 && seal_held) {
        static const int cupcake_cameras[4] = {1, 2, 3, 5};
        for (int index = 0; index < 4; ++index) {
            if (game->selected_camera != cupcake_cameras[index]) continue;
            complete_consume(filtered, GAME_BUTTON_CONFIRM | GAME_BUTTON_SEAL);
            const uint8_t bit = (uint8_t) (1u << index);
            if ((sSecretTriggers.cupcake_mask & bit) == 0u) {
                sSecretTriggers.cupcake_mask |= bit;
                content_play_select();
            }
            if ((sSecretTriggers.cupcake_mask & 0x0Fu) == 0x0Fu) {
                sSecretTriggers.cupcake_mask = 0u;
                complete_start_hidden_minigame(game, SECRET_MINIGAME_CHICA);
            }
            return true;
        }
    }

    return false;
}

static bool complete_try_stage01_trigger(Game *game,
                                         const InputState *input,
                                         InputState *filtered)
{
    if (game->camera_open || game->maintenance_open || game->night < 4 ||
        game->office_pan > -80 ||
        !input_is_held(input, GAME_BUTTON_SEAL) ||
        !input_was_pressed(input, GAME_BUTTON_CONFIRM)) {
        return false;
    }

    complete_consume(filtered, GAME_BUTTON_CONFIRM | GAME_BUTTON_SEAL |
                               kTriggerDirectionMask);

    static const uint32_t code[6] = {
        GAME_BUTTON_UP | GAME_BUTTON_RIGHT,
        GAME_BUTTON_DOWN | GAME_BUTTON_RIGHT,
        0u,
        GAME_BUTTON_UP,
        GAME_BUTTON_LEFT,
        GAME_BUTTON_DOWN
    };
    const uint32_t directions = input->held & kTriggerDirectionMask;
    if (directions == code[sSecretTriggers.stage_step]) {
        ++sSecretTriggers.stage_step;
        content_play_select();
        if (sSecretTriggers.stage_step >= 6u) {
            sSecretTriggers.stage_step = 0u;
            complete_start_hidden_minigame(game, SECRET_MINIGAME_STAGE01);
        }
    } else {
        sSecretTriggers.stage_step = 0u;
    }
    return true;
}

static bool complete_try_shadow_trigger(Game *game,
                                        const InputState *input,
                                        InputState *filtered)
{
    if (game->camera_open || game->maintenance_open || game->night < 5 ||
        game->office_pan < 90 ||
        !input_is_held(input, GAME_BUTTON_SEAL) ||
        !input_was_pressed(input, GAME_BUTTON_CONFIRM)) {
        return false;
    }

    complete_consume(filtered, GAME_BUTTON_CONFIRM | GAME_BUTTON_SEAL);
    complete_start_hidden_minigame(game, SECRET_MINIGAME_SHADOW_BONNIE);
    return true;
}

static bool complete_try_hidden_trigger(Game *game,
                                        const InputState *input,
                                        InputState *filtered)
{
    if (game->screen != SCREEN_OFFICE || sSecret.active ||
        phantoms_blocks_input(&game->phantoms)) {
        return false;
    }

    complete_reset_trigger_sequences(game->night);
    if (complete_try_camera_trigger(game, input, filtered)) return true;
    if (complete_try_stage01_trigger(game, input, filtered)) return true;
    if (complete_try_shadow_trigger(game, input, filtered)) return true;
    return false;
}

static void update_game(Game *game, const InputState *input)
{
    if (complete_update_extras(game, input)) return;
    if (complete_handle_hidden_return(game, input)) return;

    InputState filtered = *input;
    if (complete_try_hidden_trigger(game, input, &filtered) && sSecret.active)
        return;

    const GameScreen before_screen = game->screen;
    fnaf3_content_update_game(game, &filtered);
    complete_apply_gameplay_cheats(game, before_screen);
}

static void complete_draw_original_bad_ending(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 107, 0, 640, 480,
                     &gBadEndingOriginalTexture);

    if (game->screen_frames >= 90u) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 172, 342, 510, 92,
                           GRAPHICS_RGB(0, 0, 0));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 263, 360, 3,
                           "NIGHT 6 UNLOCKED", COLOUR_GREEN);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 279, 398, 3,
                           "EXTRAS UNLOCKED", COLOUR_GREEN);
    }
    if (game->screen_frames >= 180u) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 247, 440, 360, 30,
                           GRAPHICS_RGB(0, 0, 0));
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 286, 449, 1,
                           "A: RETURN TO TITLE", COLOUR_WHITE);
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void render_game(Game *game)
{
    if (!game->dirty) return;
    sCompleteRenderGame = game;

    if (complete_render_extras(game)) {
        game->dirty = false;
        sCompleteRenderGame = NULL;
        return;
    }
    if (game->screen == SCREEN_PLACEHOLDER &&
        sFinishing.mode == FINISHING_NIGHT5_ENDING) {
        game->dirty = false;
        complete_draw_original_bad_ending(game);
        sCompleteRenderGame = NULL;
        return;
    }

    fnaf3_content_render_game(game);
    sCompleteRenderGame = NULL;
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
    memset(&sFinishing, 0, sizeof(sFinishing));
    memset(&sSecret, 0, sizeof(sSecret));
    memset(&sSecretTriggers, 0, sizeof(sSecretTriggers));
    memset(&sCompleteCheats, 0, sizeof(sCompleteCheats));
    sTitleMusicPlaying = false;

    Game game;
    game_init(&game);

    /* game_init mounts the SD card through progress_save_init. Audio must be
       initialized afterwards so optional SD audio overrides can be loaded. */
    (void) audio_init();

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
    progress_save_shutdown();
    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
