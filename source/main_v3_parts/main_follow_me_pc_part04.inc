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
    static const int xs[4] = {310, 390, 470, 550};
    for (int index = 0; index < 4; ++index)
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_GHOST_0 + (index & 1)), xs[index], 416, 3);
}

static FollowMePcSprite follow_me_pc_spring_frame(void)
{
    uint32_t frame = sFollowMePc.phase_frames / 24u;
    if (frame > 8u) frame = 8u;
    return (FollowMePcSprite) (FOLLOW_ME_PC_SPRING_0 + (int) frame);
}

static void follow_me_pc_draw(const Game *game)
{
    follow_me_pc_draw_room(game);
    follow_me_pc_draw_remains();

    char chapter[24];
    snprintf(chapter, sizeof(chapter), "FOLLOW ME %d/5", sFollowMePc.chapter);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 18, 25, 2, chapter,
                       GRAPHICS_RGB(177, 72, 185));

    if (sFollowMePc.chapter < 5 && sFollowMePc.phase == FOLLOW_PHASE_GUIDE &&
        sFollowMePc.room != FOLLOW_ROOM_SHOW_STAGE) {
        const int shadow_frame = (int) ((sFollowMePc.frames / 9u) & 1u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_SHADOW_0 + shadow_frame),
            sFollowMePc.shadow_x, sFollowMePc.shadow_y, 3);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 33, 444, 2,
                           "FOLLOW ME", GRAPHICS_RGB(179, 72, 188));
    }

    if (sFollowMePc.phase == FOLLOW_PHASE_ERR) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 39, 444, 3,
                           "ERR", COLOUR_RED);
    }

    if (sFollowMePc.chapter == 5 && sFollowMePc.room == FOLLOW_ROOM_SAFE_ROOM)
        follow_me_pc_draw_safe_room_ghosts();

    if (sFollowMePc.phase == FOLLOW_PHASE_HUNT) {
        const int frame = (int) ((sFollowMePc.frames / 7u) & 1u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_PURPLE_WALK_0 + frame),
            sFollowMePc.purple_x, sFollowMePc.purple_y, 3);
    } else if (sFollowMePc.phase == FOLLOW_PHASE_GHOST_CHASE ||
               sFollowMePc.phase == FOLLOW_PHASE_SUIT) {
        const int frame = (int) ((sFollowMePc.frames / 6u) % 6u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_PURPLE_PANIC_0 + frame),
            sFollowMePc.purple_x, sFollowMePc.purple_y, 3);
        if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ROOM)
            follow_me_pc_draw_sprite(FOLLOW_ME_PC_SPRING_0, 742, 205, 3);
    } else if (sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK) {
        follow_me_pc_draw_sprite(follow_me_pc_spring_frame(), 742, 235, 3);
        if (((sFollowMePc.phase_frames / 7u) & 1u) != 0u)
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, 683, 257, 116, 11,
                               GRAPHICS_RGB(143, 8, 13));
    }

    if (sFollowMePc.phase == FOLLOW_PHASE_DISMANTLED) {
        const FollowMePcSprite wreck = (FollowMePcSprite)
            (FOLLOW_ME_PC_WRECK_0 + sFollowMePc.chapter - 1);
        follow_me_pc_draw_sprite(wreck, sFollowMePc.player_x,
                                 sFollowMePc.player_y + 12, 2);
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
