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

    static const int xs[4] = {155, 285, 570, 700};
    for (int index = 0; index < 4; ++index)
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_GHOST_2 + (index & 1)), xs[index], 424, 3);
}

static void follow_me_pc_draw_rain(uint32_t frame)
{
    /* Original PC image-bank object. The exact drop-fall creation logic is
     * handled in the next environment pass; these are the 12 MFA anchors. */
    static const int pc_x[12] = {
        44, 126, 208, 290, 372, 454,
        536, 618, 700, 782, 870, 946
    };
    static const int pc_y0[12] = {
        -25, -25, -25, -25, -25, -25,
        -25, -25, -25, -25, -27, -23
    };

    for (int index = 0; index < 12; ++index) {
        const uint32_t phase = (frame * 4u + (uint32_t) index * 61u) % 850u;
        const int pc_y = pc_y0[index] + (int) phase;
        follow_me_pc_draw_mfa(&gFollowMfaRaindrop, pc_x[index], pc_y);
    }
}

static void follow_me_pc_draw_springlock_blood(void)
{
    static const int pc_positions[4][2] = {
        {316, 308}, {711, 308}, {316, 468}, {711, 468}
    };
    for (int index = 0; index < 4; ++index)
        follow_me_pc_draw_mfa(&gFollowMfaBlood,
                              pc_positions[index][0], pc_positions[index][1]);
}

static FollowMePcSprite follow_me_pc_spring_frame(void)
{
    uint32_t frame = sFollowMePc.phase_frames / 28u;
    if (frame > 8u) frame = 8u;
    return (FollowMePcSprite) (FOLLOW_ME_PC_SPRING_0 + (int) frame);
}

static FollowMePcSprite follow_me_pc_mfa_shadow_sprite(void)
{
    /* Recovered two-frame pairs for handles 347/348/349. */
    int base = FOLLOW_ME_PC_SHADOW_0;
    if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_DOWN)
        base = FOLLOW_ME_PC_SHADOW_2;
    else if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_UP)
        base = FOLLOW_ME_PC_SHADOW_4;
    return (FollowMePcSprite) (base + (int) ((sFollowMePc.frames / 9u) & 1u));
}

static void follow_me_pc_draw(const Game *game)
{
    follow_me_pc_draw_room(game);
    follow_me_pc_draw_remains();
    follow_me_pc_draw_rain(sFollowMePc.frames);

    /* Event 8 places controls at source (502,158); event 63 hides at 5000 ms. */
    if (sFollowMePc.frames < 300u)
        follow_me_pc_draw_mfa(&gFollowMfaControls, 502, 158);

    /* Shadow Freddy and Active 351 are independent Active objects; they keep
     * updating if the player has already triggered the ERR overlay. */
    if (sFollowMePc.chapter < 5 &&
        sFollowMfaShadowKind != FOLLOW_MFA_SHADOW_NONE) {
        follow_me_pc_draw_sprite(follow_me_pc_mfa_shadow_sprite(),
                                 follow_me_pc_grid_x(sFollowMfaShadowSourceX),
                                 follow_me_pc_grid_y(sFollowMfaShadowSourceY),
                                 3);
    }

    if (sFollowMePc.chapter < 5 && sFollowMfaFollowTextTicks > 0u)
        follow_me_pc_draw_mfa(&gFollowMfaFollowText, 10, 660);

    /* Event 75 starts ERR at value 200; events 76/77 hide it at zero and
     * subtract one every game tick. */
    if (sFollowMePc.phase == FOLLOW_PHASE_ERR &&
        sFollowMePc.phase_frames < 200u)
        follow_me_pc_draw_mfa(&gFollowMfaErr, 222, 618);

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
            ((sFollowMePc.phase_frames / 7u) & 1u) != 0u)
            follow_me_pc_draw_springlock_blood();
    }

    if (sFollowMePc.phase == FOLLOW_PHASE_DISMANTLED) {
        const FollowMePcSprite wreck = (FollowMePcSprite)
            (FOLLOW_ME_PC_WRECK_0 + sFollowMePc.chapter - 1);
        follow_me_pc_draw_sprite(wreck, sFollowMePc.player_x,
                                 sFollowMePc.player_y + 12, 2);
    } else if (sFollowMePc.chapter == 5) {
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
