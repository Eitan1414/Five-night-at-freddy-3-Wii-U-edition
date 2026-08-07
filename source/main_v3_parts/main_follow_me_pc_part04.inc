static void follow_me_pc_draw_remains(void)
{
    if (sFollowMePc.room != FOLLOW_ROOM_SAFE_ENTRANCE) return;
    static const FollowMePcSprite remains[4] = {
        FOLLOW_ME_PC_WRECK_0, FOLLOW_ME_PC_WRECK_1,
        FOLLOW_ME_PC_WRECK_2, FOLLOW_ME_PC_WRECK_3
    };
    static const int xs[4] = {165, 315, 465, 580};
    const int count = sFollowMePc.chapter <= 4
        ? sFollowMePc.chapter - 1 : 4;
    for (int index = 0; index < count; ++index)
        follow_me_pc_draw_sprite(remains[index], xs[index], 375, 2);
}

static void follow_me_pc_draw_safe_room_ghosts(void)
{
    if (sFollowMePc.room != FOLLOW_ROOM_SAFE_ROOM) return;

    /* In the PC ending the four released spirits form a barrier across the
     * lower part of the room while the fifth spirit (the player) approaches
     * Purple Guy. Use the upward-facing ghost pair because they are looking
     * toward him. */
    static const int xs[4] = {155, 285, 570, 700};
    for (int index = 0; index < 4; ++index)
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_GHOST_2 + (index & 1)), xs[index], 424, 3);
}

static void follow_me_pc_draw_rain(uint32_t frame)
{
    /* Follow Me takes place in the abandoned, leaking restaurant. Keep the
     * rain deterministic and moving downward instead of regenerating random
     * streaks every frame. */
    static const int xs[18] = {
        44, 91, 138, 194, 241, 296, 344, 391, 448,
        503, 552, 608, 653, 704, 748, 786, 817, 838
    };

    for (int index = 0; index < 18; ++index) {
        const uint32_t speed = 3u + (uint32_t) (index % 3);
        const int y = 66 + (int) ((frame * speed +
            (uint32_t) index * 47u) % 350u);
        const int length = 5 + (index % 4) * 2;
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, xs[index], y, 2, length,
                           GRAPHICS_RGB(82, 99, 113));
    }

    /* The final suit failure is wetter and visually denser. */
    if (sFollowMePc.phase == FOLLOW_PHASE_SUIT ||
        sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK) {
        for (int index = 0; index < 9; ++index) {
            const int x = 70 + index * 88;
            const int y = 64 + (int) ((frame * 5u +
                (uint32_t) index * 71u) % 355u);
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, 2, 11,
                               GRAPHICS_RGB(101, 117, 132));
        }
    }
}

static FollowMePcSprite follow_me_pc_spring_frame(void)
{
    uint32_t frame = sFollowMePc.phase_frames / 28u;
    if (frame > 8u) frame = 8u;
    return (FollowMePcSprite) (FOLLOW_ME_PC_SPRING_0 + (int) frame);
}

static void follow_me_pc_draw(const Game *game)
{
    follow_me_pc_draw_room(game);
    follow_me_pc_draw_remains();
    follow_me_pc_draw_rain(sFollowMePc.frames);

    if (sFollowMePc.frames < 180u && sFollowMePc.phase == FOLLOW_PHASE_GUIDE)
        follow_me_pc_draw_env(FOLLOW_ME_ENV_WASD, 20, 372, 1);

    if (sFollowMePc.chapter < 5 && sFollowMePc.phase == FOLLOW_PHASE_GUIDE &&
        sFollowMePc.room != FOLLOW_ROOM_SHOW_STAGE) {
        const int shadow_frame = (int) ((sFollowMePc.frames / 9u) & 1u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_SHADOW_0 + shadow_frame),
            sFollowMePc.shadow_x, sFollowMePc.shadow_y, 3);
        follow_me_pc_draw_env(FOLLOW_ME_ENV_FOLLOW_TEXT, 30, 438, 1);
    }

    if (sFollowMePc.phase == FOLLOW_PHASE_ERR)
        follow_me_pc_draw_env(FOLLOW_ME_ENV_ERR, 38, 438, 1);

    if (sFollowMePc.chapter == 5 && sFollowMePc.room == FOLLOW_ROOM_SAFE_ROOM)
        follow_me_pc_draw_safe_room_ghosts();

    if (sFollowMePc.phase == FOLLOW_PHASE_HUNT) {
        const int frame = (int) ((sFollowMePc.frames / 7u) & 1u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_PURPLE_WALK_0 + frame),
            sFollowMePc.purple_x, sFollowMePc.purple_y, 3);
    } else if (sFollowMePc.phase == FOLLOW_PHASE_GHOST_CHASE) {
        const int frame = (int) ((sFollowMePc.frames / 6u) % 6u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_PURPLE_PANIC_0 + frame),
            sFollowMePc.purple_x, sFollowMePc.purple_y, 3);
        follow_me_pc_draw_sprite(FOLLOW_ME_PC_SPRING_0, 742, 235, 3);
    } else if (sFollowMePc.phase == FOLLOW_PHASE_SUIT) {
        if (follow_me_pc_near(sFollowMePc.purple_x, sFollowMePc.purple_y,
                              742, 235, 12)) {
            /* Once Purple Guy is inside Spring Bonnie, do not draw his sprite
             * over the suit. Hold the intact suit before the failure. */
            follow_me_pc_draw_sprite(FOLLOW_ME_PC_SPRING_0, 742, 235, 3);
        } else {
            const int frame = (int) ((sFollowMePc.frames / 6u) % 6u);
            follow_me_pc_draw_sprite((FollowMePcSprite)
                (FOLLOW_ME_PC_PURPLE_PANIC_0 + frame),
                sFollowMePc.purple_x, sFollowMePc.purple_y, 3);
            follow_me_pc_draw_sprite(FOLLOW_ME_PC_SPRING_0, 742, 235, 3);
        }
    } else if (sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK) {
        follow_me_pc_draw_sprite(follow_me_pc_spring_frame(), 742, 235, 3);
        if (sFollowMePc.phase_frames >= 112u &&
            ((sFollowMePc.phase_frames / 7u) & 1u) != 0u) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 711, 193, 8, 10,
                               GRAPHICS_RGB(143, 8, 13));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 754, 216, 10, 7,
                               GRAPHICS_RGB(143, 8, 13));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 728, 244, 26, 6,
                               GRAPHICS_RGB(143, 8, 13));
        }
    }

    if (sFollowMePc.phase == FOLLOW_PHASE_DISMANTLED) {
        const FollowMePcSprite wreck = (FollowMePcSprite)
            (FOLLOW_ME_PC_WRECK_0 + sFollowMePc.chapter - 1);
        follow_me_pc_draw_sprite(wreck, sFollowMePc.player_x,
                                 sFollowMePc.player_y + 12, 2);
    } else if (sFollowMePc.chapter == 5) {
        /* Keep the fifth spirit visible for the whole confrontation. */
        follow_me_pc_draw_sprite(follow_me_pc_player_sprite(),
                                 sFollowMePc.player_x,
                                 sFollowMePc.player_y, 3);
    } else if (sFollowMePc.phase != FOLLOW_PHASE_SPRINGLOCK &&
               sFollowMePc.phase != FOLLOW_PHASE_SUIT) {
        follow_me_pc_draw_sprite(follow_me_pc_player_sprite(),
                                 sFollowMePc.player_x,
                                 sFollowMePc.player_y, 3);
    }

    if (sFinishing.minigame_replay)
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 640, 446, 1,
                           "B: EXTRAS", COLOUR_WHITE);

    graphics_present(GRAPHICS_TARGET_BOTH);
}
