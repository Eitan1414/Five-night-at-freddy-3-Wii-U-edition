typedef enum SecretMinigameKind {
    SECRET_MINIGAME_BB = 0,
    SECRET_MINIGAME_MANGLE,
    SECRET_MINIGAME_CHICA,
    SECRET_MINIGAME_STAGE01,
    SECRET_MINIGAME_SHADOW_BONNIE,
    SECRET_MINIGAME_HAPPIEST_DAY,
    SECRET_MINIGAME_COUNT
} SecretMinigameKind;

typedef struct SecretMinigameState {
    bool active;
    bool completed;
    bool good_ending;
    int selection;
    SecretMinigameKind kind;
    int x;
    int y;
    int room;
    int carried;
    int delivered;
    uint8_t collected;
    uint32_t frames;
    uint32_t completed_frames;
} SecretMinigameState;

static SecretMinigameState sSecret;
static bool sTitleMusicPlaying = false;

static const char *const kSecretMinigameNames[SECRET_MINIGAME_COUNT] = {
    "BBS AIR ADVENTURE",
    "MANGLES QUEST",
    "CHICAS PARTY",
    "STAGE 01",
    "SHADOW BONNIE",
    "HAPPIEST DAY"
};

static void content_start_title_music(void)
{
    if (sTitleMusicPlaying) return;
    audio_restart(AUDIO_CUE_TITLE_MUSIC, 0.54f, true);
    sTitleMusicPlaying = true;
}

static void content_stop_title_music(void)
{
    if (!sTitleMusicPlaying) return;
    audio_stop(AUDIO_CUE_TITLE_MUSIC);
    sTitleMusicPlaying = false;
}

static void content_play_select(void)
{
    audio_restart(AUDIO_CUE_SELECT, 0.82f, false);
}

static bool secret_near(int x1, int y1, int x2, int y2, int radius)
{
    return finishing_abs(x1 - x2) <= radius &&
           finishing_abs(y1 - y2) <= radius;
}

static void secret_move_player(const InputState *input, int speed)
{
    if (input_is_held(input, GAME_BUTTON_LEFT)) sSecret.x -= speed;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) sSecret.x += speed;
    if (input_is_held(input, GAME_BUTTON_UP)) sSecret.y -= speed;
    if (input_is_held(input, GAME_BUTTON_DOWN)) sSecret.y += speed;
    if (sSecret.x < 42) sSecret.x = 42;
    if (sSecret.x > 812) sSecret.x = 812;
    if (sSecret.y < 78) sSecret.y = 78;
    if (sSecret.y > 420) sSecret.y = 420;
}

static void secret_start(Game *game, int selection)
{
    if (selection < 0) selection = 0;
    if (selection >= SECRET_MINIGAME_COUNT) selection = SECRET_MINIGAME_COUNT - 1;
    memset(&sSecret, 0, sizeof(sSecret));
    sSecret.active = true;
    sSecret.selection = selection;
    sSecret.kind = (SecretMinigameKind) selection;
    sSecret.x = 84;
    sSecret.y = 374;
    sSecret.room = 0;
    content_stop_title_music();
    audio_stop_all();
    game->screen_frames = 0u;
    game->dirty = true;
}

static void secret_return_to_menu(Game *game)
{
    const int selection = sSecret.selection;
    memset(&sSecret, 0, sizeof(sSecret));
    sSecret.selection = selection;
    sFinishing.mode = FINISHING_EXTRAS;
    sFinishing.extras_page = EXTRAS_PAGE_MINIGAMES;
    game->screen = SCREEN_PLACEHOLDER;
    game->screen_frames = 0u;
    game->dirty = true;
    content_start_title_music();
}

static void secret_complete(Game *game)
{
    if (sSecret.completed) return;
    sSecret.completed = true;
    sSecret.completed_frames = 0u;
    progress_save_complete_secret_minigame((int) sSecret.kind + 1);
    sSecret.good_ending = progress_save_good_ending_unlocked() &&
                          sSecret.kind == SECRET_MINIGAME_HAPPIEST_DAY;
    audio_play(AUDIO_CUE_END, 0.76f, false);
    game->dirty = true;
}

static void secret_collect_points(Game *game,
                                  const int points[][2],
                                  int count,
                                  int radius)
{
    for (int index = 0; index < count; ++index) {
        const uint8_t bit = (uint8_t) (1u << index);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!secret_near(sSecret.x, sSecret.y,
                         points[index][0], points[index][1], radius)) continue;
        sSecret.collected |= bit;
        content_play_select();
        game->dirty = true;
    }
}

static void secret_update_bb(Game *game, const InputState *input)
{
    static const int balloons[5][2] = {
        {178, 306}, {320, 154}, {468, 282}, {626, 128}, {754, 326}
    };
    secret_move_player(input, 4);
    secret_collect_points(game, balloons, 5, 26);
    if ((sSecret.collected & 0x1Fu) == 0x1Fu &&
        secret_near(sSecret.x, sSecret.y, 790, 90, 35)) {
        secret_complete(game);
    }
}

static void secret_update_mangle(Game *game, const InputState *input)
{
    static const int parts[4][2] = {
        {175, 116}, {352, 342}, {552, 153}, {707, 326}
    };
    secret_move_player(input, 4);
    secret_collect_points(game, parts, 4, 25);
    if ((sSecret.collected & 0x0Fu) == 0x0Fu &&
        secret_near(sSecret.x, sSecret.y, 777, 102, 36)) {
        secret_complete(game);
    }
}

static void secret_update_chica(Game *game, const InputState *input)
{
    static const int cupcakes[4][2] = {
        {125, 116}, {125, 204}, {125, 292}, {125, 380}
    };
    static const int children[4][2] = {
        {360, 117}, {510, 213}, {660, 309}, {772, 397}
    };
    secret_move_player(input, 4);
    for (int index = 0; index < 4; ++index) {
        const uint8_t bit = (uint8_t) (1u << index);
        if ((sSecret.collected & bit) == 0u &&
            secret_near(sSecret.x, sSecret.y,
                        cupcakes[index][0], cupcakes[index][1], 24)) {
            sSecret.collected |= bit;
            ++sSecret.carried;
            content_play_select();
        }
        const uint8_t delivered_bit = (uint8_t) (1u << (index + 4));
        if (sSecret.carried > 0 && (sSecret.collected & delivered_bit) == 0u &&
            secret_near(sSecret.x, sSecret.y,
                        children[index][0], children[index][1], 30)) {
            sSecret.collected |= delivered_bit;
            --sSecret.carried;
            ++sSecret.delivered;
            audio_play(AUDIO_CUE_END, 0.42f, false);
        }
    }
    if (sSecret.delivered >= 4) secret_complete(game);
    game->dirty = true;
}

static void secret_update_stage01(Game *game, const InputState *input)
{
    static const int children[3][2] = {
        {235, 125}, {446, 310}, {680, 122}
    };
    secret_move_player(input, 4);
    secret_collect_points(game, children, 3, 28);
    if ((sSecret.collected & 0x07u) == 0x07u &&
        secret_near(sSecret.x, sSecret.y, 757, 378, 38)) {
        secret_complete(game);
    }
}

static void secret_update_shadow(Game *game, const InputState *input)
{
    static const int targets[4][2] = {
        {718, 109}, {171, 114}, {708, 360}, {171, 360}
    };
    secret_move_player(input, 5);
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_MODE)) {
        sSecret.room = (sSecret.room + 1) % 4;
        sSecret.x = sSecret.room == 1 || sSecret.room == 3 ? 730 : 105;
        sSecret.y = sSecret.room >= 2 ? 350 : 120;
        content_play_select();
    }
    const int room = sSecret.room;
    const uint8_t bit = (uint8_t) (1u << room);
    if ((sSecret.collected & bit) == 0u &&
        secret_near(sSecret.x, sSecret.y,
                    targets[room][0], targets[room][1], 32)) {
        sSecret.collected |= bit;
        content_play_select();
    }
    if ((sSecret.collected & 0x0Fu) == 0x0Fu) secret_complete(game);
    game->dirty = true;
}

static void secret_update_happiest(Game *game, const InputState *input)
{
    static const int children[4][2] = {
        {232, 354}, {374, 354}, {516, 354}, {658, 354}
    };
    secret_move_player(input, 4);
    for (int index = 0; index < 4; ++index) {
        const uint8_t bit = (uint8_t) (1u << index);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!secret_near(sSecret.x, sSecret.y,
                         children[index][0], children[index][1], 30)) continue;
        sSecret.collected |= bit;
        ++sSecret.delivered;
        audio_play(AUDIO_CUE_END, 0.42f, false);
    }
    if (sSecret.delivered >= 4 &&
        secret_near(sSecret.x, sSecret.y, 760, 210, 42)) {
        secret_complete(game);
    }
    game->dirty = true;
}

static void secret_update_active(Game *game, const InputState *input)
{
    ++sSecret.frames;
    ++game->screen_frames;
    if (input_was_pressed(input, GAME_BUTTON_BACK) && !sSecret.good_ending) {
        secret_return_to_menu(game);
        return;
    }
    if (sSecret.completed) {
        ++sSecret.completed_frames;
        game->dirty = true;
        if (sSecret.completed_frames >= 150u &&
            (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
             input_was_pressed(input, GAME_BUTTON_START) ||
             input_was_pressed(input, GAME_BUTTON_BACK))) {
            secret_return_to_menu(game);
        }
        return;
    }
    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: secret_update_bb(game, input); break;
        case SECRET_MINIGAME_MANGLE: secret_update_mangle(game, input); break;
        case SECRET_MINIGAME_CHICA: secret_update_chica(game, input); break;
        case SECRET_MINIGAME_STAGE01: secret_update_stage01(game, input); break;
        case SECRET_MINIGAME_SHADOW_BONNIE: secret_update_shadow(game, input); break;
        case SECRET_MINIGAME_HAPPIEST_DAY: secret_update_happiest(game, input); break;
        default: break;
    }
    game->dirty = true;
}

static void secret_update_menu(Game *game, const InputState *input)
{
    ++game->screen_frames;
    int old = sSecret.selection;
    if (input_was_pressed(input, GAME_BUTTON_UP))
        sSecret.selection = (sSecret.selection + SECRET_MINIGAME_COUNT - 1) % SECRET_MINIGAME_COUNT;
    if (input_was_pressed(input, GAME_BUTTON_DOWN))
        sSecret.selection = (sSecret.selection + 1) % SECRET_MINIGAME_COUNT;
    if (input_was_pressed(input, GAME_BUTTON_LEFT))
        sSecret.selection = (sSecret.selection + SECRET_MINIGAME_COUNT - 3) % SECRET_MINIGAME_COUNT;
    if (input_was_pressed(input, GAME_BUTTON_RIGHT))
        sSecret.selection = (sSecret.selection + 3) % SECRET_MINIGAME_COUNT;
    if (old != sSecret.selection) content_play_select();
    if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
        input_was_pressed(input, GAME_BUTTON_START)) {
        content_play_select();
        secret_start(game, sSecret.selection);
        return;
    }
    if (input_was_pressed(input, GAME_BUTTON_BACK)) {
        content_play_select();
        finishing_change_extras_page(game, EXTRAS_PAGE_MAIN);
        return;
    }
    game->dirty = true;
}

static void content_draw_pixel_person(int x, int y, uint32_t body,
                                      uint32_t face, bool ears)
{
    const uint32_t target = GRAPHICS_TARGET_BOTH;
    if (ears) {
        graphics_draw_rect(target, x - 10, y - 27, 7, 9, body);
        graphics_draw_rect(target, x + 3, y - 27, 7, 9, body);
    }
    graphics_draw_rect(target, x - 10, y - 20, 20, 17, body);
    graphics_draw_rect(target, x - 12, y - 3, 24, 24, body);
    graphics_draw_rect(target, x - 10, y + 21, 8, 13, body);
    graphics_draw_rect(target, x + 2, y + 21, 8, 13, body);
    graphics_draw_rect(target, x - 5, y - 15, 3, 3, face);
    graphics_draw_rect(target, x + 3, y - 15, 3, 3, face);
}

static void content_draw_balloon(int x, int y, uint32_t colour)
{
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, x, y, 12, 16, colour);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 1, y + 16, 3, 19, COLOUR_WHITE);
}

static void content_draw_cupcake(int x, int y)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 11, y - 2, 22, 15, GRAPHICS_RGB(204, 77, 120));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 8, y - 10, 16, 9, GRAPHICS_RGB(235, 183, 102));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 2, y - 17, 4, 7, COLOUR_RED);
}

static void content_draw_cake(int x, int y, bool lit)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 30, y - 8, 60, 27, GRAPHICS_RGB(233, 127, 159));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 25, y - 22, 50, 16, GRAPHICS_RGB(245, 199, 110));
    for (int offset = -18; offset <= 18; offset += 12) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + offset, y - 31, 3, 10, COLOUR_WHITE);
        if (lit) graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + offset - 2, y - 36, 7, 5, GRAPHICS_RGB(250, 220, 66));
    }
}

static void secret_draw_header(const char *name, const char *objective)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 58, GRAPHICS_RGB(20, 20, 20));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 24, 18, 3, name, COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 24, 445, 1, objective, GRAPHICS_RGB(180, 180, 180));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 676, 445, 1, "B: EXIT", GRAPHICS_RGB(180, 180, 180));
}

static void secret_draw_bb(void)
{
    static const int balloons[5][2] = {
        {178, 306}, {320, 154}, {468, 282}, {626, 128}, {754, 326}
    };
    static const uint32_t colours[5] = {
        GRAPHICS_RGB(229, 50, 61), GRAPHICS_RGB(244, 203, 46),
        GRAPHICS_RGB(76, 190, 83), GRAPHICS_RGB(60, 126, 221),
        GRAPHICS_RGB(192, 73, 203)
    };
    secret_draw_header("BBS AIR ADVENTURE", "COLLECT 5 BALLOONS, THEN REACH THE EXIT CLOUD");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382, GRAPHICS_RGB(48, 172, 178));
    for (int index = 0; index < 5; ++index)
        if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
            content_draw_balloon(balloons[index][0], balloons[index][1], colours[index]);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 790, 90, 48, 25, COLOUR_WHITE);
    content_draw_pixel_person(sSecret.x, sSecret.y, GRAPHICS_RGB(57, 71, 164),
                              COLOUR_WHITE, false);
}

static void secret_draw_mangle(void)
{
    static const int parts[4][2] = {
        {175, 116}, {352, 342}, {552, 153}, {707, 326}
    };
    secret_draw_header("MANGLES QUEST", "FIND THE FOUR PARTS, THEN REACH THE CAKE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382, GRAPHICS_RGB(11, 10, 18));
    for (int y = 116; y < 430; y += 78)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 42, y, 770, 5, GRAPHICS_RGB(166, 30, 36));
    for (int index = 0; index < 4; ++index) {
        if ((sSecret.collected & (uint8_t) (1u << index)) != 0u) continue;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, parts[index][0] - 14,
                           parts[index][1] - 6, 28, 12, GRAPHICS_RGB(225, 225, 225));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, parts[index][0] - 3,
                           parts[index][1] - 16, 6, 32, GRAPHICS_RGB(225, 225, 225));
    }
    content_draw_cake(777, 102, (sSecret.frames / 10u) % 2u == 0u);
    content_draw_pixel_person(sSecret.x, sSecret.y, GRAPHICS_RGB(242, 239, 239),
                              COLOUR_RED, true);
}

