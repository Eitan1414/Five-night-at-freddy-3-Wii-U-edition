/*
 * Rendering contract for the authentic Follow Me pass.
 *
 * Kept separate from main_follow_me_pc.inc so original sprite extraction can
 * be iterated without touching collision or progression logic.  This file is
 * not wired into the active build yet.
 */

#ifndef FNAF3_FOLLOW_ME_PC_RENDER_INC
#define FNAF3_FOLLOW_ME_PC_RENDER_INC

typedef enum FollowPcSprite {
    FOLLOW_PC_SPR_FREDDY_DOWN_0 = 0,
    FOLLOW_PC_SPR_FREDDY_DOWN_1,
    FOLLOW_PC_SPR_FREDDY_LEFT_0,
    FOLLOW_PC_SPR_FREDDY_LEFT_1,
    FOLLOW_PC_SPR_FREDDY_RIGHT_0,
    FOLLOW_PC_SPR_FREDDY_RIGHT_1,
    FOLLOW_PC_SPR_FREDDY_UP_0,
    FOLLOW_PC_SPR_FREDDY_UP_1,

    FOLLOW_PC_SPR_BONNIE_DOWN_0,
    FOLLOW_PC_SPR_BONNIE_DOWN_1,
    FOLLOW_PC_SPR_BONNIE_LEFT_0,
    FOLLOW_PC_SPR_BONNIE_LEFT_1,
    FOLLOW_PC_SPR_BONNIE_RIGHT_0,
    FOLLOW_PC_SPR_BONNIE_RIGHT_1,
    FOLLOW_PC_SPR_BONNIE_UP_0,
    FOLLOW_PC_SPR_BONNIE_UP_1,

    FOLLOW_PC_SPR_CHICA_DOWN_0,
    FOLLOW_PC_SPR_CHICA_DOWN_1,
    FOLLOW_PC_SPR_CHICA_LEFT_0,
    FOLLOW_PC_SPR_CHICA_LEFT_1,
    FOLLOW_PC_SPR_CHICA_RIGHT_0,
    FOLLOW_PC_SPR_CHICA_RIGHT_1,
    FOLLOW_PC_SPR_CHICA_UP_0,
    FOLLOW_PC_SPR_CHICA_UP_1,

    FOLLOW_PC_SPR_FOXY_DOWN_0,
    FOLLOW_PC_SPR_FOXY_DOWN_1,
    FOLLOW_PC_SPR_FOXY_LEFT_0,
    FOLLOW_PC_SPR_FOXY_LEFT_1,
    FOLLOW_PC_SPR_FOXY_RIGHT_0,
    FOLLOW_PC_SPR_FOXY_RIGHT_1,
    FOLLOW_PC_SPR_FOXY_UP_0,
    FOLLOW_PC_SPR_FOXY_UP_1,

    FOLLOW_PC_SPR_CHILD_DOWN_0,
    FOLLOW_PC_SPR_CHILD_DOWN_1,
    FOLLOW_PC_SPR_CHILD_LEFT_0,
    FOLLOW_PC_SPR_CHILD_LEFT_1,
    FOLLOW_PC_SPR_CHILD_RIGHT_0,
    FOLLOW_PC_SPR_CHILD_RIGHT_1,
    FOLLOW_PC_SPR_CHILD_UP_0,
    FOLLOW_PC_SPR_CHILD_UP_1,

    FOLLOW_PC_SPR_SHADOW_RIGHT,
    FOLLOW_PC_SPR_SHADOW_UP,
    FOLLOW_PC_SPR_SHADOW_DOWN,
    FOLLOW_PC_SPR_PURPLE_GUY,
    FOLLOW_PC_SPR_SPRING_BONNIE,
    FOLLOW_PC_SPR_SPRING_BONNIE_FAIL_0,
    FOLLOW_PC_SPR_SPRING_BONNIE_FAIL_1,
    FOLLOW_PC_SPR_SPRING_BONNIE_FAIL_2,

    FOLLOW_PC_SPR_FREDDY_PARTS,
    FOLLOW_PC_SPR_BONNIE_PARTS,
    FOLLOW_PC_SPR_CHICA_PARTS,
    FOLLOW_PC_SPR_FOXY_PARTS,

    FOLLOW_PC_SPR_COUNT
} FollowPcSprite;

typedef struct FollowPcRenderOps {
    void (*clear)(void *user, uint32_t colour);
    void (*rect)(void *user, int x, int y, int w, int h, uint32_t colour);
    void (*sprite)(void *user, FollowPcSprite sprite, int x, int y);
    void (*text)(void *user, int x, int y, const char *text, uint32_t colour);
    void (*present)(void *user);
    void *user;
} FollowPcRenderOps;

static FollowPcSprite follow_pc_actor_sprite(FollowPcChapter chapter,
                                              FollowPcFacing facing,
                                              int walk_frame)
{
    static const FollowPcSprite table[FOLLOW_PC_CHAPTER_COUNT][4][2] = {
        {
            {FOLLOW_PC_SPR_FREDDY_DOWN_0, FOLLOW_PC_SPR_FREDDY_DOWN_1},
            {FOLLOW_PC_SPR_FREDDY_LEFT_0, FOLLOW_PC_SPR_FREDDY_LEFT_1},
            {FOLLOW_PC_SPR_FREDDY_RIGHT_0, FOLLOW_PC_SPR_FREDDY_RIGHT_1},
            {FOLLOW_PC_SPR_FREDDY_UP_0, FOLLOW_PC_SPR_FREDDY_UP_1}
        },
        {
            {FOLLOW_PC_SPR_BONNIE_DOWN_0, FOLLOW_PC_SPR_BONNIE_DOWN_1},
            {FOLLOW_PC_SPR_BONNIE_LEFT_0, FOLLOW_PC_SPR_BONNIE_LEFT_1},
            {FOLLOW_PC_SPR_BONNIE_RIGHT_0, FOLLOW_PC_SPR_BONNIE_RIGHT_1},
            {FOLLOW_PC_SPR_BONNIE_UP_0, FOLLOW_PC_SPR_BONNIE_UP_1}
        },
        {
            {FOLLOW_PC_SPR_CHICA_DOWN_0, FOLLOW_PC_SPR_CHICA_DOWN_1},
            {FOLLOW_PC_SPR_CHICA_LEFT_0, FOLLOW_PC_SPR_CHICA_LEFT_1},
            {FOLLOW_PC_SPR_CHICA_RIGHT_0, FOLLOW_PC_SPR_CHICA_RIGHT_1},
            {FOLLOW_PC_SPR_CHICA_UP_0, FOLLOW_PC_SPR_CHICA_UP_1}
        },
        {
            {FOLLOW_PC_SPR_FOXY_DOWN_0, FOLLOW_PC_SPR_FOXY_DOWN_1},
            {FOLLOW_PC_SPR_FOXY_LEFT_0, FOLLOW_PC_SPR_FOXY_LEFT_1},
            {FOLLOW_PC_SPR_FOXY_RIGHT_0, FOLLOW_PC_SPR_FOXY_RIGHT_1},
            {FOLLOW_PC_SPR_FOXY_UP_0, FOLLOW_PC_SPR_FOXY_UP_1}
        },
        {
            {FOLLOW_PC_SPR_CHILD_DOWN_0, FOLLOW_PC_SPR_CHILD_DOWN_1},
            {FOLLOW_PC_SPR_CHILD_LEFT_0, FOLLOW_PC_SPR_CHILD_LEFT_1},
            {FOLLOW_PC_SPR_CHILD_RIGHT_0, FOLLOW_PC_SPR_CHILD_RIGHT_1},
            {FOLLOW_PC_SPR_CHILD_UP_0, FOLLOW_PC_SPR_CHILD_UP_1}
        }
    };

    int chapter_index = (int) chapter;
    int facing_index = (int) facing;
    if (chapter_index < 0 || chapter_index >= FOLLOW_PC_CHAPTER_COUNT)
        chapter_index = 0;
    if (facing_index < 0 || facing_index > 3) facing_index = 0;
    return table[chapter_index][facing_index][walk_frame & 1];
}

static FollowPcSprite follow_pc_parts_sprite(FollowPcChapter chapter)
{
    switch (chapter) {
        case FOLLOW_PC_FREDDY: return FOLLOW_PC_SPR_FREDDY_PARTS;
        case FOLLOW_PC_BONNIE: return FOLLOW_PC_SPR_BONNIE_PARTS;
        case FOLLOW_PC_CHICA: return FOLLOW_PC_SPR_CHICA_PARTS;
        case FOLLOW_PC_FOXY: return FOLLOW_PC_SPR_FOXY_PARTS;
        default: return FOLLOW_PC_SPR_FREDDY_PARTS;
    }
}

static FollowPcSprite follow_pc_shadow_sprite(FollowPcPoint current,
                                               FollowPcPoint next)
{
    const int dx = next.x - current.x;
    const int dy = next.y - current.y;
    if (follow_pc_abs(dy) > follow_pc_abs(dx))
        return dy < 0 ? FOLLOW_PC_SPR_SHADOW_UP : FOLLOW_PC_SPR_SHADOW_DOWN;
    return FOLLOW_PC_SPR_SHADOW_RIGHT;
}

static FollowPcSprite follow_pc_springlock_frame(uint32_t phase_frames)
{
    if (phase_frames < 45u) return FOLLOW_PC_SPR_SPRING_BONNIE;
    if (phase_frames < 85u) return FOLLOW_PC_SPR_SPRING_BONNIE_FAIL_0;
    if (phase_frames < 125u) return FOLLOW_PC_SPR_SPRING_BONNIE_FAIL_1;
    return FOLLOW_PC_SPR_SPRING_BONNIE_FAIL_2;
}

static void follow_pc_draw_map(const FollowPcRenderOps *ops)
{
    if (!ops || !ops->rect) return;

    /* PC minigame palette: black background, purple room boundaries and the
     * rain-soaked arcade look.  Geometry stays decoupled from sprite art. */
    const uint32_t wall_colour = 0x7B2E8BFFu;
    const uint32_t floor_colour = 0x171017FFu;
    if (ops->clear) ops->clear(ops->user, 0x000000FFu);
    ops->rect(ops->user, 16, 74, 822, 390, floor_colour);

    const int count = (int) (sizeof(kFollowPcWalls) / sizeof(kFollowPcWalls[0]));
    for (int i = 0; i < count; ++i) {
        const FollowPcRect *wall = &kFollowPcWalls[i];
        ops->rect(ops->user, wall->x, wall->y, wall->w, wall->h, wall_colour);
    }
}

static void follow_pc_render(const FollowPcState *state,
                             const FollowPcRenderOps *ops)
{
    if (!state || !ops || !state->active) return;

    follow_pc_draw_map(ops);

    if (ops->text && state->chapter != FOLLOW_PC_FINAL_CHILD &&
        state->phase == FOLLOW_PC_PHASE_FOLLOW) {
        ops->text(ops->user, 355, 18, "FOLLOW ME", 0xA030B0FFu);
    }

    if (state->shadow_visible && ops->sprite) {
        const FollowPcPoint shadow = follow_pc_shadow_position(state);
        FollowPcPoint previous = shadow;
        if (state->shadow_waypoint >= 0) {
            int index = state->shadow_waypoint;
            const int route_count =
                (int) (sizeof(kFollowPcShadowRoute) /
                       sizeof(kFollowPcShadowRoute[0]));
            if (index >= route_count) index = route_count - 1;
            previous = kFollowPcShadowRoute[index];
        }
        ops->sprite(ops->user,
                    follow_pc_shadow_sprite(previous, shadow),
                    shadow.x, shadow.y);
    }

    if (state->purple_visible && ops->sprite &&
        state->phase != FOLLOW_PC_PHASE_SPRINGLOCK) {
        ops->sprite(ops->user, FOLLOW_PC_SPR_PURPLE_GUY,
                    kFollowPcPurplePosition.x, kFollowPcPurplePosition.y);
    }

    if (ops->sprite) {
        if (state->phase == FOLLOW_PC_PHASE_SPRINGLOCK) {
            ops->sprite(ops->user,
                        follow_pc_springlock_frame(state->phase_frames),
                        kFollowPcPurplePosition.x,
                        kFollowPcPurplePosition.y);
        } else if (state->player_dismantled &&
                   state->chapter != FOLLOW_PC_FINAL_CHILD) {
            ops->sprite(ops->user, follow_pc_parts_sprite(state->chapter),
                        state->x, state->y);
        } else {
            ops->sprite(ops->user,
                        follow_pc_actor_sprite(state->chapter,
                                               state->facing,
                                               state->walk_frame),
                        state->x, state->y);
        }
    }

    if (ops->present) ops->present(ops->user);
}

#endif /* FNAF3_FOLLOW_ME_PC_RENDER_INC */
