/* Final rendering override for authentic PC Follow Me, endings and secret
 * minigames. Gameplay/state handling remains in the existing Wii U logic. */

static void pc_finishing_draw_sprite_centered(const TextureRle *texture,
                                              int center_x,
                                              int center_y,
                                              int max_width,
                                              int max_height)
{
    if (texture == NULL || texture->width == 0u || texture->height == 0u ||
        max_width <= 0 || max_height <= 0) return;

    int width = max_width;
    int height = (int) ((uint32_t) texture->height * (uint32_t) width /
                        texture->width);
    if (height > max_height) {
        height = max_height;
        width = (int) ((uint32_t) texture->width * (uint32_t) height /
                       texture->height);
    }
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     center_x - width / 2,
                     center_y - height / 2,
                     width, height, texture);
}

static const TextureRle *pc_finishing_follow_player_texture(int chapter)
{
    switch (chapter) {
        case 1: return &gPcFollowFreddyTexture;
        case 2: return &gPcFollowBonnieTexture;
        case 3: return &gPcFollowChicaTexture;
        case 4: return &gPcFollowFoxyTexture;
        default: return &gPcFollowGhostTexture;
    }
}

static void pc_finishing_draw_checker_floor(int x, int y, int width, int height)
{
    const int cell = 24;
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, width, height,
                       GRAPHICS_RGB(8, 8, 10));
    for (int row = 0; row * cell < height; ++row) {
        for (int column = 0; column * cell < width; ++column) {
            if (((row + column) & 1) == 0) continue;
            int cw = cell;
            int ch = cell;
            if ((column + 1) * cell > width) cw = width - column * cell;
            if ((row + 1) * cell > height) ch = height - row * cell;
            graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                               x + column * cell, y + row * cell,
                               cw, ch, GRAPHICS_RGB(44, 42, 66));
        }
    }
}

static void pc_finishing_draw_follow_me(const Game *game)
{
    static const int waypoints[4][2] = {
        {206, 360}, {342, 258}, {537, 177}, {721, 101}
    };
    (void) game;

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(2, 3, 8));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 24, 52, 806, 374,
                       GRAPHICS_RGB(20, 20, 27));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 46, 314, 238, 96,
                       GRAPHICS_RGB(55, 50, 61));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 285, 205, 188, 119,
                       GRAPHICS_RGB(47, 43, 54));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 470, 118, 198, 112,
                       GRAPHICS_RGB(52, 46, 57));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 660, 65, 144, 111,
                       GRAPHICS_RGB(48, 42, 51));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 246, 247, 75, 35,
                       GRAPHICS_RGB(44, 40, 50));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 435, 166, 75, 35,
                       GRAPHICS_RGB(44, 40, 50));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 627, 109, 62, 35,
                       GRAPHICS_RGB(44, 40, 50));
    pc_finishing_draw_checker_floor(47, 365, 236, 44);
    pc_finishing_draw_checker_floor(286, 279, 186, 44);
    pc_finishing_draw_checker_floor(471, 185, 196, 44);
    pc_finishing_draw_checker_floor(661, 132, 142, 43);

    for (int x = 62; x < 800; x += 78)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 59, 3, 346,
                           GRAPHICS_RGB(72, 34, 77));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 24, 18, 2,
                       "FOLLOW ME", GRAPHICS_RGB(190, 86, 201));
    char chapter[24];
    snprintf(chapter, sizeof(chapter), "%d / 5", sFinishing.minigame_chapter);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 752, 20, 2, chapter,
                       GRAPHICS_RGB(156, 132, 158));

    if (sFinishing.minigame_event_frames == 0u) {
        const int waypoint = sFinishing.minigame_waypoint < 4
            ? sFinishing.minigame_waypoint : 3;
        pc_finishing_draw_sprite_centered(&gPcFollowShadowFreddyTexture,
                                          waypoints[waypoint][0],
                                          waypoints[waypoint][1] - 24,
                                          66, 82);
        pc_finishing_draw_sprite_centered(
            pc_finishing_follow_player_texture(sFinishing.minigame_chapter),
            sFinishing.minigame_x, sFinishing.minigame_y - 27,
            sFinishing.minigame_chapter == 5 ? 54 : 62, 84);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 628, 445, 2,
                           "FOLLOW ME", GRAPHICS_RGB(189, 77, 197));
    } else {
        const bool flash = ((sFinishing.minigame_event_frames / 7u) & 1u) != 0u;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 610, 73, 194, 150,
                           GRAPHICS_RGB(10, 7, 13));
        pc_finishing_draw_sprite_centered(&gPcFollowPurpleGuyTexture,
                                          673, 142, 62, 98);
        if (sFinishing.minigame_chapter < 5) {
            pc_finishing_draw_sprite_centered(
                pc_finishing_follow_player_texture(sFinishing.minigame_chapter),
                753, 143, 64, 94);
            if (flash) {
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 720, 178, 28, 7,
                                   COLOUR_RED);
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 752, 190, 35, 7,
                                   COLOUR_RED);
            }
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 508, 389, 3,
                               "YOU CANT", COLOUR_RED);
        } else {
            pc_finishing_draw_sprite_centered(&gPcFollowSpringBonnieTexture,
                                              754, 144, 67, 117);
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, 716, 79, 76, 139, 4,
                                flash ? COLOUR_RED : GRAPHICS_RGB(155, 116, 31));
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 492, 389, 3,
                               flash ? "SPRINGLOCK" : "HE FOLLOWED YOU",
                               COLOUR_RED);
        }
    }

    if (sFinishing.minigame_replay)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 24, 449, 1,
                           "B: RETURN TO EXTRAS", COLOUR_WHITE);
}

static void pc_secret_draw_bb(void)
{
    static const int balloons[5][2] = {
        {178, 306}, {320, 154}, {468, 282}, {626, 128}, {754, 326}
    };
    static const uint32_t colours[5] = {
        GRAPHICS_RGB(229, 50, 61), GRAPHICS_RGB(244, 203, 46),
        GRAPHICS_RGB(76, 190, 83), GRAPHICS_RGB(60, 126, 221),
        GRAPHICS_RGB(192, 73, 203)
    };
    secret_draw_header("BBS AIR ADVENTURE",
                       "COLLECT 5 BALLOONS, THEN REACH THE EXIT CLOUD");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       GRAPHICS_RGB(48, 172, 178));
    for (int index = 0; index < 5; ++index)
        if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
            content_draw_balloon(balloons[index][0], balloons[index][1], colours[index]);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 790, 90, 48, 25, COLOUR_WHITE);
    pc_finishing_draw_sprite_centered(&gPcSecretBBPlayerTexture,
                                      sSecret.x, sSecret.y - 12, 58, 68);
}

static void pc_secret_draw_mangle(void)
{
    static const int parts[4][2] = {
        {175, 116}, {352, 342}, {552, 153}, {707, 326}
    };
    secret_draw_header("MANGLES QUEST",
                       "FIND THE FOUR PARTS, THEN REACH THE CAKE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       GRAPHICS_RGB(9, 8, 15));
    for (int y = 116; y < 430; y += 78)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 42, y, 770, 6,
                           GRAPHICS_RGB(221, 32, 42));
    for (int index = 0; index < 4; ++index) {
        if ((sSecret.collected & (uint8_t) (1u << index)) != 0u) continue;
        pc_finishing_draw_sprite_centered(&gPcSecretManglePlayerTexture,
                                          parts[index][0], parts[index][1],
                                          38, 38);
    }
    content_draw_cake(777, 102, (sSecret.frames / 10u) % 2u == 0u);
    pc_finishing_draw_sprite_centered(&gPcSecretManglePlayerTexture,
                                      sSecret.x, sSecret.y - 8, 62, 62);
}

static void pc_secret_draw_chica(void)
{
    static const int cupcakes[4][2] = {
        {125, 116}, {125, 204}, {125, 292}, {125, 380}
    };
    static const int children[4][2] = {
        {360, 117}, {510, 213}, {660, 309}, {772, 397}
    };
    secret_draw_header("CHICAS PARTY",
                       "COLLECT CUPCAKES AND GIVE ONE TO EACH CHILD");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       GRAPHICS_RGB(44, 168, 174));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 70, 72, 115, 350,
                       GRAPHICS_RGB(28, 118, 38));
    for (int index = 0; index < 4; ++index) {
        const uint8_t pickup = (uint8_t) (1u << index);
        const uint8_t delivered = (uint8_t) (1u << (index + 4));
        if ((sSecret.collected & pickup) == 0u)
            content_draw_cupcake(cupcakes[index][0], cupcakes[index][1]);
        const TextureRle *child = (index & 1)
            ? &gPcSecretChicaChildGreenTexture
            : &gPcSecretChicaChildBlueTexture;
        pc_finishing_draw_sprite_centered(child, children[index][0],
                                          children[index][1], 52, 54);
        if ((sSecret.collected & delivered) != 0u)
            content_draw_cupcake(children[index][0] + 28,
                                 children[index][1] + 10);
    }
    pc_finishing_draw_sprite_centered(&gPcSecretChicaPlayerTexture,
                                      sSecret.x, sSecret.y - 13, 58, 72);
    char carried[32];
    snprintf(carried, sizeof(carried), "CUPCAKES: %d", sSecret.carried);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 640, 22, 2, carried,
                       GRAPHICS_RGB(235, 205, 45));
}

static void pc_secret_draw_stage01(void)
{
    static const int children[3][2] = {
        {235, 125}, {446, 310}, {680, 122}
    };
    secret_draw_header("STAGE 01",
                       "FIND THREE CHILDREN, THEN TAKE THE CAKE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       GRAPHICS_RGB(46, 171, 176));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 35, 338, 785, 90,
                       GRAPHICS_RGB(131, 64, 39));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 700, 70, 120, 270,
                       GRAPHICS_RGB(131, 64, 39));
    for (int index = 0; index < 3; ++index)
        if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
            pc_finishing_draw_sprite_centered(&gPcSecretStageChildTexture,
                                              children[index][0], children[index][1],
                                              54, 54);
    content_draw_cake(757, 378, true);
    pc_finishing_draw_sprite_centered(&gPcSecretStagePlayerTexture,
                                      sSecret.x, sSecret.y - 14, 54, 72);
}

static void pc_secret_draw_shadow(void)
{
    static const int targets[4][2] = {
        {718, 109}, {171, 114}, {708, 360}, {171, 360}
    };
    static const uint32_t rooms[4] = {
        GRAPHICS_RGB(43, 13, 55), GRAPHICS_RGB(75, 24, 86),
        GRAPHICS_RGB(27, 10, 38), GRAPHICS_RGB(90, 34, 104)
    };
    secret_draw_header("SHADOW BONNIE",
                       "A OR R: PHASE ROOMS  -  TOUCH EACH SHADOW CAKE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       rooms[sSecret.room]);
    for (int x = 75; x < 830; x += 115)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 95, 8, 310,
                           GRAPHICS_RGB(13, 7, 20));
    const int room = sSecret.room;
    if ((sSecret.collected & (uint8_t) (1u << room)) == 0u)
        content_draw_cake(targets[room][0], targets[room][1], false);
    pc_finishing_draw_sprite_centered(&gPcSecretShadowBonniePlayerTexture,
                                      sSecret.x, sSecret.y - 13, 58, 76);
    char room_text[24];
    snprintf(room_text, sizeof(room_text), "ROOM %d/4", room + 1);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 690, 22, 2, room_text,
                       GRAPHICS_RGB(195, 92, 220));
}

static const TextureRle *pc_secret_happiest_child_texture(int index)
{
    switch (index) {
        case 0: return &gPcSecretHappiestChild1Texture;
        case 1: return &gPcSecretHappiestChild2Texture;
        case 2: return &gPcSecretHappiestChild3Texture;
        default: return &gPcSecretHappiestChild4Texture;
    }
}

static void pc_secret_draw_happiest(void)
{
    static const int children[4][2] = {
        {232, 354}, {374, 354}, {516, 354}, {658, 354}
    };
    static const uint32_t child_colours[4] = {
        GRAPHICS_RGB(91, 165, 79), GRAPHICS_RGB(208, 62, 69),
        GRAPHICS_RGB(72, 119, 202), GRAPHICS_RGB(232, 152, 54)
    };
    secret_draw_header("HAPPIEST DAY",
                       "VISIT EVERY CHILD, THEN JOIN THE CAKE TABLE");
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       GRAPHICS_RGB(48, 170, 175));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 50, 390, 754, 26,
                       GRAPHICS_RGB(38, 38, 38));
    for (int index = 0; index < 4; ++index) {
        pc_finishing_draw_sprite_centered(pc_secret_happiest_child_texture(index),
                                          children[index][0], children[index][1] - 13,
                                          66, 86);
        if ((sSecret.collected & (uint8_t) (1u << index)) != 0u)
            content_draw_balloon(children[index][0], 260, child_colours[index]);
    }
    content_draw_cake(760, 210, sSecret.delivered >= 4);
    pc_finishing_draw_sprite_centered(&gPcSecretHappiestPlayerTexture,
                                      sSecret.x, sSecret.y - 10, 50, 66);
}

static void pc_secret_draw_good_ending(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_tiled_texture_draw(GRAPHICS_TARGET_BOTH, 107, 0, 640, 480,
                          &gPcGoodEndingTexture);
    if (sSecret.completed_frames >= 150u) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 235, 435, 384, 34,
                           COLOUR_BLACK);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 282, 445, 1,
                           "A: RETURN TO EXTRAS", COLOUR_WHITE);
    }
    (void) game;
}

static void pc_secret_draw_active(const Game *game)
{
    if (sSecret.good_ending) {
        pc_secret_draw_good_ending(game);
    } else {
        switch (sSecret.kind) {
            case SECRET_MINIGAME_BB: pc_secret_draw_bb(); break;
            case SECRET_MINIGAME_MANGLE: pc_secret_draw_mangle(); break;
            case SECRET_MINIGAME_CHICA: pc_secret_draw_chica(); break;
            case SECRET_MINIGAME_STAGE01: pc_secret_draw_stage01(); break;
            case SECRET_MINIGAME_SHADOW_BONNIE: pc_secret_draw_shadow(); break;
            case SECRET_MINIGAME_HAPPIEST_DAY: pc_secret_draw_happiest(); break;
            default: break;
        }
        if (sSecret.completed) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 171, 176, 512, 132,
                               GRAPHICS_RGB(4, 4, 4));
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, 171, 176, 512, 132, 4,
                                COLOUR_WHITE);
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 245, 205, 4,
                               "MINIGAME COMPLETE", COLOUR_WHITE);
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 280, 267, 1,
                               "PROGRESS SAVED - A TO RETURN", COLOUR_GREEN);
        }
    }
}

static void pc_finishing_draw_bad_ending(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_tiled_texture_draw(GRAPHICS_TARGET_BOTH, 107, 0, 640, 480,
                          &gPcBadEndingTexture);
    if (game->screen_frames >= 90u) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 172, 342, 510, 92,
                           COLOUR_BLACK);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 242, 356, 2,
                           "NIGHT 6 UNLOCKED", COLOUR_GREEN);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 250, 394, 2,
                           "EXTRAS UNLOCKED", COLOUR_GREEN);
    }
    if (game->screen_frames >= 180u) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 247, 440, 360, 30,
                           COLOUR_BLACK);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 286, 444, 1,
                           "A: RETURN TO TITLE", COLOUR_WHITE);
    }
}

static void pc_finishing_draw_newspaper(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_tiled_texture_draw(GRAPHICS_TARGET_BOTH, 107, 0, 640, 480,
                          &gPcNewspaperEndingTexture);
    if (game->screen_frames >= 180u) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 247, 440, 360, 30,
                           COLOUR_BLACK);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 286, 444, 1,
                           "A: RETURN TO TITLE", COLOUR_WHITE);
    }
}

static bool pc_finishing_render_override(Game *game)
{
    bool handled = false;
    if (sSecret.active) {
        handled = true;
    } else if (game->screen == SCREEN_PLACEHOLDER) {
        handled = sFinishing.mode == FINISHING_FOLLOW_ME ||
                  sFinishing.mode == FINISHING_NIGHT5_ENDING ||
                  sFinishing.mode == FINISHING_NIGHT6_ENDING;
    }
    if (!handled) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    if (sSecret.active) {
        pc_secret_draw_active(game);
    } else if (sFinishing.mode == FINISHING_FOLLOW_ME) {
        pc_finishing_draw_follow_me(game);
    } else if (sFinishing.mode == FINISHING_NIGHT5_ENDING) {
        pc_finishing_draw_bad_ending(game);
    } else {
        pc_finishing_draw_newspaper(game);
    }

    achievement_draw_overlay();
    graphics_present(GRAPHICS_TARGET_BOTH);
    return true;
}
