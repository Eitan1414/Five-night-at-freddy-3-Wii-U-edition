    if (next_camera != game->selected_camera) {
        game->selected_camera = next_camera;
        restart_camera_static(); game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM)) trigger_audio_lure(game);
}

static void handle_maintenance_input(Game *game, const InputState *input)
{
    if (game->repairing) return;
    if (input_was_pressed(input, GAME_BUTTON_BACK) || input_was_pressed(input, GAME_BUTTON_SELECT)) {
        game->maintenance_open = false; game->dirty = true; return;
    }
    if (input_was_pressed(input, GAME_BUTTON_UP)) {
        game->maintenance_selection = (game->maintenance_selection + MAINTENANCE_ITEM_COUNT - 1) % MAINTENANCE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
        game->maintenance_selection = (game->maintenance_selection + 1) % MAINTENANCE_ITEM_COUNT;
        game->dirty = true;
    }
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) || input_was_pressed(input, GAME_BUTTON_START)) start_repair(game);
}

static void handle_office_input(Game *game, const InputState *input)
{
    if (phantoms_blocks_input(&game->phantoms)) return;
    if (game->camera_open) { handle_camera_input(game, input); return; }
    if (game->maintenance_open) { handle_maintenance_input(game, input); return; }
    if (input_was_pressed(input, GAME_BUTTON_BACK)) {
        audio_stop_all(); game->screen = SCREEN_TITLE; game->screen_frames = 0u; game->dirty = true; return;
    }
    if (input_was_pressed(input, GAME_BUTTON_PANEL)) { open_cameras(game); return; }
    if (input_was_pressed(input, GAME_BUTTON_SELECT)) { open_maintenance(game); return; }
    int pan = game->office_pan;
    if (input_is_held(input, GAME_BUTTON_LEFT)) pan -= 5;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) pan += 5;
    if (pan < -140) pan = -140;
    if (pan > 140) pan = 140;
    if (pan != game->office_pan) { game->office_pan = pan; game->dirty = true; }
    if (input_is_held(input, GAME_BUTTON_START) && input_was_pressed(input, GAME_BUTTON_RIGHT)) advance_hour(game);
}

static void update_game(Game *game, const InputState *input)
{
    ++game->screen_frames;
    switch (game->screen) {
        case SCREEN_WARNING:
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) || input_was_pressed(input, GAME_BUTTON_START) ||
                game->screen_frames >= WARNING_FRAMES) {
                game->screen = SCREEN_TITLE; game->screen_frames = 0u; game->dirty = true;
            } else if ((game->screen_frames % 6u) == 0u) game->dirty = true;
            break;
        case SCREEN_TITLE:
            handle_title_input(game, input);
            if ((game->screen_frames % 9u) == 0u) game->dirty = true;
            break;
        case SCREEN_NIGHT_INTRO:
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) || input_was_pressed(input, GAME_BUTTON_START) ||
                game->screen_frames >= NIGHT_INTRO_FRAMES) {
                game->screen = SCREEN_OFFICE; game->screen_frames = 0u; game->dirty = true;
            }
            break;
        case SCREEN_OFFICE:
            handle_office_input(game, input);
            if (game->screen == SCREEN_OFFICE) update_active_game(game, input);
            break;
        case SCREEN_VICTORY:
            if ((game->screen_frames % 6u) == 0u) game->dirty = true;
            if (game->screen_frames >= 180u &&
                (input_was_pressed(input, GAME_BUTTON_CONFIRM) || input_was_pressed(input, GAME_BUTTON_START))) {
                if (game->night < NIGHT_COUNT) {
                    reset_night(game, game->night + 1);
                } else {
                    game->screen = SCREEN_TITLE; game->screen_frames = 0u; game->dirty = true;
                }
            }
            break;
        case SCREEN_SPRINGTRAP_JUMPSCARE: {
            const JumpscareSequence *sequence = game->springtrap_jumpscare_right
                ? &gSpringtrapJumpscareRight
                : &gSpringtrapJumpscareLeft;
            game->dirty = true;
            if (game->screen_frames >=
                sequence->frame_count * sequence->ticks_per_frame) {
                begin_game_over(game);
            }
            break;
        }
        case SCREEN_GAME_OVER:
            if ((game->screen_frames % 5u) == 0u) game->dirty = true;
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) || input_was_pressed(input, GAME_BUTTON_START)) {
                audio_stop(AUDIO_CUE_SCREAM); reset_night(game, game->night);
            } else if (input_was_pressed(input, GAME_BUTTON_BACK)) {
                audio_stop(AUDIO_CUE_SCREAM); game->screen = SCREEN_TITLE; game->screen_frames = 0u; game->dirty = true;
            }
            break;
        case SCREEN_PLACEHOLDER:
            if (input_was_pressed(input, GAME_BUTTON_BACK)) {
                game->screen = SCREEN_TITLE; game->screen_frames = 0u; game->dirty = true;
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
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 67, 174, 720, 138, &gWarningTexture);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 285, 390, 2, "A: SKIP", GRAPHICS_RGB(105, 151, 101));
    if ((game->screen_frames / 10u) & 1u)
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 49, 146, 755, 205, 3, GRAPHICS_RGB(96, 14, 10));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_title(const Game *game)
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
                       progress_save_load_status_is_error()
                           ? COLOUR_RED : GRAPHICS_RGB(74, 111, 73));
    for (int item = 0; item < TITLE_ITEM_COUNT; ++item) {
        const int y = 284 + item * 48;
        if (item == game->title_selection) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 262, 36, GRAPHICS_RGB(31, 72, 37));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 106, y - 10, 5, 36, GRAPHICS_RGB(129, 207, 121));
        }
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 124, y, 3, items[item],
                           item == game->title_selection ? COLOUR_WHITE : GRAPHICS_RGB(112, 139, 113));
    }
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 410, 0, 444, 480, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 410 + glitch, 16, 444, 448, kTitleFrames[frame]);
    draw_scanlines(GRAPHICS_TARGET_BOTH, game->seed + game->screen_frames);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_night_intro(const Game *game)
{
    char night_text[24];
    snprintf(night_text, sizeof(night_text), "NIGHT %d", game->night);
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 286, 139, 7, night_text, COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 329, 239, 4, "12:00 AM", COLOUR_GREEN);
