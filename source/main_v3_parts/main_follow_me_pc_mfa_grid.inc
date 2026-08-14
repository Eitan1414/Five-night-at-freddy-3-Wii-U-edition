/* MFA-derived Follow Me room grid and edge transitions.
 * Source: fivenights3-94.mfa, frame `cutscenes` (1024x768). */

static int sFollowMfaVertical = 2;
static int sFollowMfaHorizontal = 3;
static int sFollowMfaEntryDirection = 2;

/* Shadow Freddy is created by events 95..104. The three source objects are
 * `shadow right`, `shadow down` and `shadow up`. */
static int sFollowMfaShadowKind;
static int sFollowMfaShadowSourceX;
static int sFollowMfaShadowSourceY;
static uint32_t sFollowMfaShadowMoveClock;
static uint32_t sFollowMfaFollowTextTicks;

enum {
    FOLLOW_MFA_WALL_UP    = 1u << 0,
    FOLLOW_MFA_WALL_DOWN  = 1u << 1,
    FOLLOW_MFA_WALL_LEFT  = 1u << 2,
    FOLLOW_MFA_WALL_RIGHT = 1u << 3,
};

enum {
    FOLLOW_MFA_SHADOW_NONE = 0,
    FOLLOW_MFA_SHADOW_RIGHT,
    FOLLOW_MFA_SHADOW_DOWN,
    FOLLOW_MFA_SHADOW_UP,
};

static int follow_me_pc_grid_scale(int value)
{
    if (value >= 0) return (value * 5 + 4) / 8;
    return -(((-value) * 5 + 4) / 8);
}

static int follow_me_pc_grid_x(int source_x)
{
    return 107 + follow_me_pc_grid_scale(source_x);
}

static int follow_me_pc_grid_y(int source_y)
{
    return follow_me_pc_grid_scale(source_y);
}

static unsigned follow_me_pc_mfa_walls(int vertical, int horizontal)
{
    if (vertical == 1 && horizontal == 5)
        return FOLLOW_MFA_WALL_UP | FOLLOW_MFA_WALL_LEFT |
               FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 2 && horizontal == 1)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_UP |
               FOLLOW_MFA_WALL_LEFT;
    if (vertical == 2 && horizontal >= 2 && horizontal <= 4)
        return FOLLOW_MFA_WALL_UP;
    if (vertical == 2 && horizontal == 5)
        return FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 3 && horizontal == 2)
        return FOLLOW_MFA_WALL_LEFT;
    if (vertical == 3 && horizontal == 3)
        return FOLLOW_MFA_WALL_DOWN;
    if (vertical == 3 && horizontal == 4)
        return FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 3 && horizontal == 5)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_RIGHT |
               FOLLOW_MFA_WALL_LEFT;
    if (vertical == 4 && (horizontal == 2 || horizontal == 4))
        return FOLLOW_MFA_WALL_LEFT | FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 5 && horizontal == 2)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_LEFT;
    if (vertical == 5 && horizontal == 3)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_UP;
    if (vertical == 5 && horizontal == 4)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_RIGHT;
    return 0u;
}

static bool follow_me_pc_mfa_guide_for_cell(int *kind, int *x, int *y)
{
    if (sFollowMePc.chapter >= 5) return false;

    if (sFollowMfaVertical == 2 && sFollowMfaHorizontal == 4) {
        *kind = FOLLOW_MFA_SHADOW_RIGHT; *x = 816; *y = 376; return true;
    }
    if (((sFollowMfaVertical == 3 && sFollowMfaHorizontal == 3) ||
         (sFollowMfaVertical == 3 && sFollowMfaHorizontal == 2)) &&
        sFollowMfaEntryDirection != 4) {
        *kind = FOLLOW_MFA_SHADOW_RIGHT; *x = 816; *y = 376; return true;
    }
    if (sFollowMfaVertical == 2 && sFollowMfaHorizontal == 5) {
        *kind = FOLLOW_MFA_SHADOW_UP; *x = 520; *y = 202; return true;
    }
    if (((sFollowMfaVertical == 3 && sFollowMfaHorizontal == 4) ||
         (sFollowMfaVertical == 4 && sFollowMfaHorizontal == 4) ||
         (sFollowMfaVertical == 5 && sFollowMfaHorizontal == 4)) &&
        sFollowMfaEntryDirection != 3) {
        *kind = FOLLOW_MFA_SHADOW_UP; *x = 520; *y = 202; return true;
    }
    if (((sFollowMfaVertical == 5 && sFollowMfaHorizontal == 3) ||
         (sFollowMfaVertical == 5 && sFollowMfaHorizontal == 2)) &&
        sFollowMfaEntryDirection != 4) {
        *kind = FOLLOW_MFA_SHADOW_RIGHT; *x = 816; *y = 376; return true;
    }
    if (sFollowMfaVertical == 4 && sFollowMfaHorizontal == 2 &&
        sFollowMfaEntryDirection != 1) {
        *kind = FOLLOW_MFA_SHADOW_DOWN; *x = 514; *y = 564; return true;
    }
    return false;
}

static void follow_me_pc_mfa_spawn_guide(void)
{
    int kind = FOLLOW_MFA_SHADOW_NONE;
    int x = 0;
    int y = 0;
    if (!follow_me_pc_mfa_guide_for_cell(&kind, &x, &y)) return;

    sFollowMfaShadowKind = kind;
    sFollowMfaShadowSourceX = x;
    sFollowMfaShadowSourceY = y;
    sFollowMfaShadowMoveClock = 0u;
    sFollowMfaFollowTextTicks = 200u;
}

static void follow_me_pc_mfa_step_guide(void)
{
    if (sFollowMfaFollowTextTicks > 0u)
        --sFollowMfaFollowTextTicks;

    if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_NONE) return;

    /* Event 88: 30 source pixels every 290 ms. */
    sFollowMfaShadowMoveClock += 1000u;
    while (sFollowMfaShadowMoveClock >= 17400u) {
        sFollowMfaShadowMoveClock -= 17400u;
        if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_RIGHT)
            sFollowMfaShadowSourceX += 30;
        else if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_DOWN)
            sFollowMfaShadowSourceY += 30;
        else if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_UP)
            sFollowMfaShadowSourceY -= 30;
    }

    if (sFollowMfaShadowSourceX > 1055 ||
        sFollowMfaShadowSourceY < -80 ||
        sFollowMfaShadowSourceY > 850)
        sFollowMfaShadowKind = FOLLOW_MFA_SHADOW_NONE;
}

static void follow_me_pc_reset(int chapter)
{
    follow_me_pc_reset_legacy(chapter);

    sFollowMfaVertical = 2;
    sFollowMfaHorizontal = 3;
    sFollowMfaEntryDirection = 2;
    sFollowMfaShadowKind = FOLLOW_MFA_SHADOW_NONE;
    sFollowMfaShadowMoveClock = 0u;
    sFollowMfaFollowTextTicks = 0u;

    int source_x = 500;
    int source_y = 272;
    if (chapter == 2) {
        source_x = 342;
        source_y = 212;
    } else if (chapter == 3) {
        source_x = 658;
        source_y = 212;
    } else if (chapter == 4) {
        sFollowMfaVertical = 3;
        sFollowMfaHorizontal = 2;
        source_x = 268;
        source_y = 420;
    } else if (chapter >= 5) {
        source_x = 496;
        source_y = 494;
    }

    sFollowMePc.player_x = follow_me_pc_grid_x(source_x);
    sFollowMePc.player_y = follow_me_pc_grid_y(source_y);
    sFollowMePc.room = FOLLOW_ROOM_SHOW_STAGE;
    follow_me_pc_mfa_spawn_guide();
}

static void follow_me_pc_move_player(const InputState *input)
{
    const int speed = 4;
    int dx = 0;
    int dy = 0;

    /* These Active objects keep running while ERR is displayed. */
    follow_me_pc_mfa_step_guide();

    if (input_is_held(input, GAME_BUTTON_LEFT)) dx -= speed;
    if (input_is_held(input, GAME_BUTTON_RIGHT)) dx += speed;
    if (input_is_held(input, GAME_BUTTON_UP)) dy -= speed;
    if (input_is_held(input, GAME_BUTTON_DOWN)) dy += speed;

    sFollowMePc.moving = dx != 0 || dy != 0;
    if (sFollowMePc.moving) ++sFollowMePc.move_frames;

    if (dx < 0) sFollowMePc.direction = FOLLOW_DIR_LEFT;
    else if (dx > 0) sFollowMePc.direction = FOLLOW_DIR_RIGHT;
    else if (dy < 0) sFollowMePc.direction = FOLLOW_DIR_UP;
    else if (dy > 0) sFollowMePc.direction = FOLLOW_DIR_DOWN;

    const unsigned walls = follow_me_pc_mfa_walls(
        sFollowMfaVertical, sFollowMfaHorizontal);
    int next_x = sFollowMePc.player_x + dx;
    int next_y = sFollowMePc.player_y + dy;
    bool changed_cell = false;

    const int left_edge = follow_me_pc_grid_x(-1);
    const int right_edge = follow_me_pc_grid_x(1023);
    const int top_edge = follow_me_pc_grid_y(-1);
    const int bottom_edge = follow_me_pc_grid_y(771);

    if (next_x <= left_edge) {
        if ((walls & FOLLOW_MFA_WALL_LEFT) == 0u) {
            --sFollowMfaHorizontal;
            sFollowMfaEntryDirection = 4;
            next_x = follow_me_pc_grid_x(879);
            changed_cell = true;
        } else {
            next_x = left_edge + 1;
        }
    } else if (next_x >= right_edge) {
        if ((walls & FOLLOW_MFA_WALL_RIGHT) == 0u) {
            ++sFollowMfaHorizontal;
            sFollowMfaEntryDirection = 2;
            next_x = follow_me_pc_grid_x(142);
            changed_cell = true;
        } else {
            next_x = right_edge - 1;
        }
    }

    bool handled_top = false;
    if (next_y <= top_edge &&
        sFollowMfaVertical == 2 && sFollowMfaHorizontal == 5 &&
        sFollowMePc.chapter < 5) {
        /* Event 75: collision with the up barrier in (2,5) bounces the hit box
         * down by 60 PC pixels and sets ERR alterable value to 200. It does
         * not enter cell (1,5) during chapters 1..4. */
        next_y = sFollowMePc.player_y + follow_me_pc_grid_scale(60);
        sFollowMePc.phase = FOLLOW_PHASE_ERR;
        sFollowMePc.phase_frames = 0u;
        handled_top = true;
    }

    if (!handled_top && next_y <= top_edge) {
        if ((walls & FOLLOW_MFA_WALL_UP) == 0u) {
            --sFollowMfaVertical;
            sFollowMfaEntryDirection = 1;
            next_y = follow_me_pc_grid_y(664);
            changed_cell = true;
        } else {
            next_y = top_edge + 1;
        }
    } else if (next_y >= bottom_edge) {
        if ((walls & FOLLOW_MFA_WALL_DOWN) == 0u) {
            ++sFollowMfaVertical;
            sFollowMfaEntryDirection = 3;
            next_y = follow_me_pc_grid_y(122);
            changed_cell = true;
        } else {
            next_y = bottom_edge - 1;
        }
    }

    sFollowMePc.player_x = next_x;
    sFollowMePc.player_y = next_y;

    if (changed_cell)
        follow_me_pc_mfa_spawn_guide();
}

static void follow_me_pc_update_room_transitions(void)
{
    /* Edge transitions now happen directly in follow_me_pc_move_player(). */
}
