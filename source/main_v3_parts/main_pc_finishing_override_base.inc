/* Final rendering override for authentic PC Follow Me, endings and secret
 * minigames. Gameplay/state handling remains in the existing Wii U logic.
 *
 * This pass deliberately keeps the Wii U minigame collision/state model but
 * replaces the old placeholder teal rooms with scenery that follows the
 * original PC minigames: black voids, scanline/striped platforms, balloons,
 * party tables, moon/stars, stage geometry and the Follow Me checker map. */

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
    const int cell = 12;
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, width, height,
                       GRAPHICS_RGB(6, 6, 9));
    for (int row = 0; row * cell < height; ++row) {
        for (int column = 0; column * cell < width; ++column) {
            if (((row + column) & 1) == 0) continue;
            int cw = cell;
            int ch = cell;
            if ((column + 1) * cell > width) cw = width - column * cell;
            if ((row + 1) * cell > height) ch = height - row * cell;
            graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                               x + column * cell, y + row * cell,
                               cw, ch, GRAPHICS_RGB(29, 29, 36));
        }
    }
}

static void pc_pixel_striped_rect(int x, int y, int width, int height,
                                  uint32_t base, uint32_t stripe)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, width, height, base);
    for (int yy = y + 2; yy < y + height; yy += 6) {
        int h = 2;
        if (yy + h > y + height) h = y + height - yy;
        if (h > 0)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, yy, width, h, stripe);
    }
}

static void pc_pixel_vertical_striped_rect(int x, int y, int width, int height,
                                           uint32_t base, uint32_t stripe)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, width, height, base);
    for (int xx = x + 2; xx < x + width; xx += 6) {
        int w = 2;
        if (xx + w > x + width) w = x + width - xx;
        if (w > 0)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, xx, y, w, height, stripe);
    }
}

static void pc_pixel_star(int x, int y, uint32_t colour)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 1, y - 4, 3, 9, colour);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 4, y - 1, 9, 3, colour);
}

static void pc_pixel_balloon_platform(int x, int y, uint32_t colour)
{
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, x, y, 29, 22, colour);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 15, y - 9, 9, 5,
                       COLOUR_WHITE);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 2, y + 21, 5, 6, colour);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y + 27, 2, 20,
                       GRAPHICS_RGB(220, 220, 220));
}

static void pc_pixel_exit_door(int x, int y, bool open)
{
    const uint32_t edge = open ? COLOUR_WHITE : GRAPHICS_RGB(116, 116, 116);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 46, 62,
                       GRAPHICS_RGB(3, 3, 5));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, y, 46, 62, 4, edge);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, x + 6, y + 23, 1,
                       "EXIT", edge);
}

static void pc_pixel_cake_stack(int x, int y)
{
    const uint32_t pink = GRAPHICS_RGB(228, 92, 126);
    const uint32_t cream = GRAPHICS_RGB(241, 211, 86);
    const uint32_t red = GRAPHICS_RGB(232, 54, 57);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 48, y - 19, 96, 19, pink);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 44, y - 14, 88, 5, cream);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 36, y - 38, 72, 18, pink);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 32, y - 33, 64, 5, cream);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 24, y - 56, 48, 17, pink);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 20, y - 51, 40, 5, cream);
    for (int dx = -14; dx <= 14; dx += 14) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + dx - 1, y - 67, 3, 10,
                           COLOUR_WHITE);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + dx - 3, y - 72, 7, 5,
                           red);
    }
}

static void pc_secret_begin(const char *title, uint32_t title_colour,
                            const char *objective)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 22, 17, 3, title, title_colour);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 20, 51, 814, 2,
                       GRAPHICS_RGB(42, 42, 46));
    if (objective != NULL)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 22, 449, 1,
                           objective, GRAPHICS_RGB(137, 137, 145));
}

static void pc_follow_draw_room(int x, int y, int width, int height)
{
    const uint32_t wall = GRAPHICS_RGB(47, 63, 154);
    pc_finishing_draw_checker_floor(x + 6, y + 6, width - 12, height - 12);
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, y, width, height, 6, wall);
}

static void pc_finishing_draw_follow_me(const Game *game)
{
    static const int waypoints[4][2] = {
        {206, 360}, {342, 258}, {537, 177}, {721, 101}
    };
    (void) game;

    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 22, 17, 3,
                       "FOLLOW ME", GRAPHICS_RGB(188, 73, 202));
    char chapter[24];
    snprintf(chapter, sizeof(chapter), "%d / 5", sFinishing.minigame_chapter);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 764, 20, 2, chapter,
                       GRAPHICS_RGB(135, 111, 147));

    /* FNaF 1-style pizzeria layout: blue walls, dark checker floors and the
     * sealed safe-room at the upper right.  Rooms are arranged around the
     * existing Wii U waypoints so the old progression remains untouched. */
    pc_follow_draw_room(48, 316, 236, 100);
    pc_follow_draw_room(280, 218, 196, 112);
    pc_follow_draw_room(468, 133, 202, 106);
    pc_follow_draw_room(656, 58, 152, 122);
    pc_follow_draw_room(48, 188, 178, 92);
    pc_follow_draw_room(558, 292, 250, 122);

    pc_finishing_draw_checker_floor(219, 249, 67, 34);
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 216, 246, 73, 40, 5,
                        GRAPHICS_RGB(47, 63, 154));
    pc_finishing_draw_checker_floor(431, 177, 44, 72);
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 428, 174, 50, 78, 5,
                        GRAPHICS_RGB(47, 63, 154));
    pc_finishing_draw_checker_floor(626, 98, 39, 78);
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 623, 95, 45, 84, 5,
                        GRAPHICS_RGB(47, 63, 154));

    /* Stage / tables / arcade-like wall details from the PC intermissions. */
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 69, 201, 136, 26,
                       GRAPHICS_RGB(84, 31, 86));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 82, 207, 22, 15,
                       GRAPHICS_RGB(120, 83, 34));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 126, 207, 22, 15,
                       GRAPHICS_RGB(120, 83, 34));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 170, 207, 22, 15,
                       GRAPHICS_RGB(120, 83, 34));
    for (int x = 584; x <= 764; x += 45) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 328, 26, 41,
                           GRAPHICS_RGB(25, 29, 45));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, x, 328, 26, 41, 3,
                            GRAPHICS_RGB(82, 55, 101));
    }
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 676, 69, 1,
                       "SAFE ROOM", GRAPHICS_RGB(107, 107, 122));

    if (sFinishing.minigame_event_frames == 0u) {
        const int waypoint = sFinishing.minigame_waypoint < 4
            ? sFinishing.minigame_waypoint : 3;
        pc_finishing_draw_sprite_centered(&gPcFollowShadowFreddyTexture,
                                          waypoints[waypoint][0],
                                          waypoints[waypoint][1] - 23,
                                          66, 82);
        pc_finishing_draw_sprite_centered(
            pc_finishing_follow_player_texture(sFinishing.minigame_chapter),
            sFinishing.minigame_x, sFinishing.minigame_y - 25,
            sFinishing.minigame_chapter == 5 ? 54 : 62, 84);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 675, 445, 2,
                           "FOLLOW ME", GRAPHICS_RGB(188, 73, 202));
    } else {
        const bool flash = ((sFinishing.minigame_event_frames / 7u) & 1u) != 0u;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 665, 68, 133, 100,
                           GRAPHICS_RGB(3, 2, 5));
        pc_finishing_draw_sprite_centered(&gPcFollowPurpleGuyTexture,
                                          700, 123, 58, 91);
        if (sFinishing.minigame_chapter < 5) {
            pc_finishing_draw_sprite_centered(
                pc_finishing_follow_player_texture(sFinishing.minigame_chapter),
                760, 125, 60, 88);
            if (flash) {
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 729, 159, 24, 6,
                                   COLOUR_RED);
                graphics_draw_rect(GRAPHICS_TARGET_BOTH, 758, 151, 25, 6,
                                   COLOUR_RED);
            }
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 595, 389, 3,
                               "YOU CANT", COLOUR_RED);
        } else {
            pc_finishing_draw_sprite_centered(&gPcFollowSpringBonnieTexture,
                                              758, 124, 64, 108);
            graphics_draw_frame(GRAPHICS_TARGET_BOTH, 726, 70, 67, 101, 4,
                                flash ? COLOUR_RED : GRAPHICS_RGB(155, 116, 31));
            graphics_draw_text(GRAPHICS_TARGET_BOTH, 529, 389, 3,
                               flash ? "SPRINGLOCK" : "HE FOLLOWED YOU",
                               COLOUR_RED);
        }
    }

    if (sFinishing.minigame_replay)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 22, 449, 1,
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
    const uint32_t green_dark = GRAPHICS_RGB(12, 79, 25);
    const uint32_t green_bright = GRAPHICS_RGB(29, 150, 43);

    pc_secret_begin("BBS AIR ADVENTURE", COLOUR_WHITE,
                    "COLLECT 5 BALLOONS - EXIT APPEARS AT THE TOP RIGHT");

    /* PC-like floating green striped architecture over a black void. */
    pc_pixel_striped_rect(48, 372, 245, 20, green_dark, green_bright);
    pc_pixel_vertical_striped_rect(262, 264, 20, 109, green_dark, green_bright);
    pc_pixel_striped_rect(262, 251, 213, 20, green_dark, green_bright);
    pc_pixel_vertical_striped_rect(449, 176, 20, 77, green_dark, green_bright);
    pc_pixel_striped_rect(448, 164, 178, 20, green_dark, green_bright);
    pc_pixel_vertical_striped_rect(603, 116, 20, 50, green_dark, green_bright);
    pc_pixel_striped_rect(603, 104, 181, 20, green_dark, green_bright);
    pc_pixel_striped_rect(513, 351, 287, 20, green_dark, green_bright);
    pc_pixel_vertical_striped_rect(513, 300, 20, 54, green_dark, green_bright);

    /* Pale clouds and the hidden, slightly impossible geometry are a big
     * part of the original Air Adventure look. */
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 112, 184, 46, 18,
                          GRAPHICS_RGB(124, 124, 132));
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 160, 181, 37, 15,
                          GRAPHICS_RGB(124, 124, 132));
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 703, 262, 43, 16,
                          GRAPHICS_RGB(124, 124, 132));

    for (int index = 0; index < 5; ++index)
        if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
            content_draw_balloon(balloons[index][0], balloons[index][1], colours[index]);

    const bool exit_open = (sSecret.collected & 0x1Fu) == 0x1Fu;
    pc_pixel_exit_door(778, 63, exit_open);
    pc_finishing_draw_sprite_centered(&gPcSecretBBPlayerTexture,
                                      sSecret.x, sSecret.y - 12, 58, 68);
}

static void pc_secret_draw_mangle(void)
{
    static const int parts[4][2] = {
        {175, 116}, {352, 342}, {552, 153}, {707, 326}
    };
    const uint32_t moon_blue = GRAPHICS_RGB(55, 108, 218);

    pc_secret_begin("MANGLES QUEST", COLOUR_WHITE,
                    "FIND THE FOUR PARTS - THEN REACH THE CAKE");

    /* The PC minigame's signature crescent moon and sparse stars. */
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 205, 178, 118, 114, moon_blue);
    graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 247, 145, 100, 92, COLOUR_BLACK);
    for (int yy = 89; yy <= 264; yy += 9)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 91, yy, 92, 3,
                           GRAPHICS_RGB(29, 62, 146));
    pc_pixel_star(82, 114, GRAPHICS_RGB(66, 112, 213));
    pc_pixel_star(414, 191, GRAPHICS_RGB(66, 112, 213));
    pc_pixel_star(494, 84, GRAPHICS_RGB(205, 129, 215));
    pc_pixel_star(46, 185, GRAPHICS_RGB(205, 129, 215));

    /* Secret-route cake on a red balloon, matching the recognizable PC
     * success scene while remaining at the Wii U completion coordinate. */
    pc_pixel_balloon_platform(775, 142, GRAPHICS_RGB(228, 48, 54));
    pc_pixel_cake_stack(775, 119);

    for (int index = 0; index < 4; ++index) {
        if ((sSecret.collected & (uint8_t) (1u << index)) != 0u) continue;
        const int x = parts[index][0];
        const int y = parts[index][1];
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 9, y - 7, 18, 14,
                           index & 1 ? GRAPHICS_RGB(225, 80, 166)
                                     : GRAPHICS_RGB(92, 127, 232));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x - 4, y - 12, 8, 5,
                           COLOUR_WHITE);
    }

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
    const uint32_t green_dark = GRAPHICS_RGB(12, 87, 25);
    const uint32_t green_bright = GRAPHICS_RGB(31, 158, 49);
    const uint32_t yellow = GRAPHICS_RGB(242, 229, 82);

    pc_secret_begin("CHICAS PARTY", yellow,
                    "COLLECT CUPCAKES AND GIVE ONE TO EACH CHILD");

    /* Large birthday-cake alcove and the green striped path visible in the
     * original PC Chica's Party. */
    pc_pixel_vertical_striped_rect(84, 72, 18, 296, green_dark, green_bright);
    pc_pixel_striped_rect(84, 349, 213, 18, green_dark, green_bright);
    pc_pixel_vertical_striped_rect(280, 246, 18, 105, green_dark, green_bright);
    pc_pixel_striped_rect(280, 233, 225, 18, green_dark, green_bright);
    pc_pixel_vertical_striped_rect(488, 151, 18, 84, green_dark, green_bright);
    pc_pixel_striped_rect(488, 138, 201, 18, green_dark, green_bright);
    pc_pixel_vertical_striped_rect(672, 103, 18, 37, green_dark, green_bright);

    pc_pixel_cake_stack(208, 155);
    pc_finishing_draw_sprite_centered(&gPcSecretChicaChildBlueTexture,
                                      139, 166, 45, 48);

    /* Yellow window bars on the right of the PC sheet/screen. */
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 722, 73, 105, 151, 5, yellow);
    for (int x = 744; x < 818; x += 25)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, 78, 5, 141, yellow);
    for (int y = 101; y < 213; y += 27)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 727, y, 95, 4, yellow);

    pc_pixel_balloon_platform(581, 330, GRAPHICS_RGB(226, 51, 57));
    pc_pixel_balloon_platform(721, 286, GRAPHICS_RGB(226, 51, 57));

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
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 666, 22, 2, carried, yellow);
}

static void pc_secret_draw_stage01(void)
{
    static const int children[3][2] = {
        {235, 125}, {446, 310}, {680, 122}
    };
    const uint32_t gold_dark = GRAPHICS_RGB(119, 83, 23);
    const uint32_t gold = GRAPHICS_RGB(224, 180, 47);
    const uint32_t purple = GRAPHICS_RGB(89, 42, 111);

    pc_secret_begin("STAGE 01", COLOUR_WHITE,
                    "FIND THREE CHILDREN - THEN TAKE THE CAKE");

    /* Fredbear's stage: gold striped boards, purple backdrop and a long
     * lower walkway.  This replaces the previous flat turquoise room. */
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 61, 84, 342, 145, purple);
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 61, 84, 342, 145, 6, gold);
    pc_pixel_striped_rect(68, 207, 329, 20, gold_dark, gold);
    pc_pixel_vertical_striped_rect(82, 102, 20, 105, gold_dark, gold);
    pc_pixel_vertical_striped_rect(361, 102, 20, 105, gold_dark, gold);
    pc_pixel_striped_rect(58, 367, 741, 21, gold_dark, gold);
    pc_pixel_vertical_striped_rect(423, 227, 20, 142, gold_dark, gold);
    pc_pixel_striped_rect(423, 259, 220, 18, gold_dark, gold);

    /* Spring Bonnie remains on the stage as scenery while Fredbear is the
     * authentic Stage 01 player sprite. */
    pc_finishing_draw_sprite_centered(&gPcFollowSpringBonnieTexture,
                                      313, 171, 57, 92);
    pc_pixel_exit_door(744, 303, true);

    for (int index = 0; index < 3; ++index)
        if ((sSecret.collected & (uint8_t) (1u << index)) == 0u)
            pc_finishing_draw_sprite_centered(&gPcSecretStageChildTexture,
                                              children[index][0], children[index][1],
                                              54, 54);
    content_draw_cake(757, 378, true);
    pc_finishing_draw_sprite_centered(&gPcSecretStagePlayerTexture,
                                      sSecret.x, sSecret.y - 14, 54, 72);
}

static void pc_secret_draw_shadow_room(int room)
{
    const uint32_t dark = GRAPHICS_RGB(20, 4, 30);
    const uint32_t mid = GRAPHICS_RGB(74, 20, 95);
    const uint32_t bright = GRAPHICS_RGB(137, 49, 171);

    if (room == 0) {
        pc_pixel_striped_rect(50, 356, 258, 17, dark, mid);
        pc_pixel_vertical_striped_rect(286, 226, 17, 132, dark, mid);
        pc_pixel_striped_rect(286, 213, 218, 17, dark, mid);
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 680, 166, 48, 18, mid);
    } else if (room == 1) {
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 204, 184, 116, 110, mid);
        graphics_draw_ellipse(GRAPHICS_TARGET_BOTH, 246, 151, 97, 89,
                              COLOUR_BLACK);
        pc_pixel_star(473, 125, bright);
        pc_pixel_star(680, 228, mid);
    } else if (room == 2) {
        pc_pixel_vertical_striped_rect(84, 80, 18, 275, dark, mid);
        pc_pixel_striped_rect(84, 338, 252, 18, dark, mid);
        pc_pixel_cake_stack(225, 151);
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 710, 80, 110, 140, 5,
                            bright);
    } else {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 68, 89, 336, 133, dark);
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 68, 89, 336, 133, 6,
                            bright);
        pc_pixel_striped_rect(58, 365, 735, 18, dark, mid);
        pc_pixel_vertical_striped_rect(420, 221, 18, 146, dark, mid);
    }

    /* Glitch scanlines and displaced fragments sell the Shadow Bonnie room
     * switching without changing the existing room-switch gameplay. */
    for (int y = 63; y < 432; y += 16)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           (y * 7) % 97, y, 854 - ((y * 7) % 97), 2,
                           GRAPHICS_RGB(35, 8, 48));
    for (int i = 0; i < 8; ++i) {
        int x = 55 + ((i * 109 + room * 47) % 735);
        int y = 80 + ((i * 71 + room * 31) % 325);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 13 + (i & 3) * 7, 4,
                           i & 1 ? mid : bright);
    }
}

static void pc_secret_draw_shadow(void)
{
    static const int targets[4][2] = {
        {718, 109}, {171, 114}, {708, 360}, {171, 360}
    };
    pc_secret_begin("SHADOW BONNIE", GRAPHICS_RGB(185, 74, 222),
                    "A OR R: PHASE BETWEEN THE FOUR GLITCH ROOMS");
    pc_secret_draw_shadow_room(sSecret.room);

    const int room = sSecret.room;
    if ((sSecret.collected & (uint8_t) (1u << room)) == 0u)
        content_draw_cake(targets[room][0], targets[room][1], false);
    pc_finishing_draw_sprite_centered(&gPcSecretShadowBonniePlayerTexture,
                                      sSecret.x, sSecret.y - 13, 58, 76);
    char room_text[24];
    snprintf(room_text, sizeof(room_text), "ROOM %d/4", room + 1);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 708, 22, 2, room_text,
                       GRAPHICS_RGB(195, 92, 220));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 620, 420, 1,
                       "RWQFSFASXC", GRAPHICS_RGB(100, 36, 127));
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

static void pc_pixel_party_table(int x, int y, uint32_t trim)
{
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 176, 13,
                       GRAPHICS_RGB(49, 49, 56));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 176, 4, trim);
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + 12, y + 13, 8, 43,
                       GRAPHICS_RGB(49, 49, 56));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, x + 156, y + 13, 8, 43,
                       GRAPHICS_RGB(49, 49, 56));
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

    pc_secret_begin("HAPPIEST DAY", COLOUR_WHITE,
                    "VISIT EVERY CHILD - THEN JOIN THE CAKE TABLE");

    /* Long black party room with several tables, cakes and floating balloons,
     * rather than the old teal rectangle. */
    pc_pixel_party_table(70, 294, GRAPHICS_RGB(71, 118, 206));
    pc_pixel_party_table(309, 294, GRAPHICS_RGB(82, 163, 87));
    pc_pixel_party_table(548, 294, GRAPHICS_RGB(199, 65, 75));
    pc_pixel_cake_stack(158, 292);
    pc_pixel_cake_stack(397, 292);
    pc_pixel_cake_stack(636, 292);

    content_draw_balloon(107, 118, GRAPHICS_RGB(70, 125, 215));
    content_draw_balloon(182, 91, GRAPHICS_RGB(238, 215, 64));
    content_draw_balloon(266, 130, GRAPHICS_RGB(72, 125, 215));
    content_draw_balloon(349, 101, GRAPHICS_RGB(221, 54, 61));
    content_draw_balloon(438, 129, GRAPHICS_RGB(71, 125, 215));
    content_draw_balloon(525, 107, GRAPHICS_RGB(75, 178, 89));
    content_draw_balloon(611, 128, GRAPHICS_RGB(194, 79, 207));
    content_draw_balloon(708, 94, GRAPHICS_RGB(239, 213, 61));

    pc_pixel_striped_rect(47, 390, 758, 20,
                          GRAPHICS_RGB(25, 25, 31),
                          GRAPHICS_RGB(74, 74, 82));

    for (int index = 0; index < 4; ++index) {
        pc_finishing_draw_sprite_centered(pc_secret_happiest_child_texture(index),
                                          children[index][0], children[index][1] - 13,
                                          66, 86);
        if ((sSecret.collected & (uint8_t) (1u << index)) != 0u)
            content_draw_balloon(children[index][0], 239, child_colours[index]);
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
