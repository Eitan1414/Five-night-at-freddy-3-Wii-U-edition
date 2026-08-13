/* MFA-derived Follow Me room grid and edge transitions.
 *
 * Source: fivenights3-94.mfa, frame `cutscenes` (1024x768).
 * The original event sheet starts `vertical` at 2 and `horizontal` at 3,
 * with the player hit box at (500,272).  Leaving an edge increments/decrements
 * the matching counter and positions the hit box relative to the opposite
 * leave object.  Coordinates below are the exact event-sheet results after
 * the existing 1024x768 -> centered 640x480 Wii U transform.
 */

static int sFollowMfaVertical = 2;
static int sFollowMfaHorizontal = 3;

enum {
    FOLLOW_MFA_WALL_UP    = 1u << 0,
    FOLLOW_MFA_WALL_DOWN  = 1u << 1,
    FOLLOW_MFA_WALL_LEFT  = 1u << 2,
    FOLLOW_MFA_WALL_RIGHT = 1u << 3,
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

/* Events 9..24 in the original cutscenes frame.  Event action 27 makes the
 * corresponding edge barrier active for the current (vertical,horizontal)
 * counter pair. */
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

static void follow_me_pc_reset(int chapter)
{
    follow_me_pc_reset_legacy(chapter);

    /* Original Start-of-Frame event: vertical=2, horizontal=3 and hit box at
     * source (500,272). */
    sFollowMfaVertical = 2;
    sFollowMfaHorizontal = 3;
    sFollowMePc.player_x = follow_me_pc_grid_x(500);
    sFollowMePc.player_y = follow_me_pc_grid_y(272);
    sFollowMePc.room = FOLLOW_ROOM_SHOW_STAGE; /* legacy field; no longer routing */
}

static void follow_me_pc_move_player(const InputState *input)
{
    /* Preserve the already-tested Wii U input cadence; only geometry/routing
     * changes in this pass. */
    const int speed = 4;
    int dx = 0;
    int dy = 0;
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

    /* `leave left/right/top/bottom` are at source -1/1023/-1/771. */
    const int left_edge = follow_me_pc_grid_x(-1);
    const int right_edge = follow_me_pc_grid_x(1023);
    const int top_edge = follow_me_pc_grid_y(-1);
    const int bottom_edge = follow_me_pc_grid_y(771);

    if (next_x <= left_edge) {
        if ((walls & FOLLOW_MFA_WALL_LEFT) == 0u) {
            --sFollowMfaHorizontal; /* original Horizontal -= 1 */
            /* leave left (-1,369) + event offset (+880,0) = (879,369) */
            next_x = follow_me_pc_grid_x(879);
        } else {
            next_x = left_edge + 1;
        }
    } else if (next_x >= right_edge) {
        if ((walls & FOLLOW_MFA_WALL_RIGHT) == 0u) {
            ++sFollowMfaHorizontal; /* original Horizontal += 1 */
            /* leave right (1023,377) + event offset (-881,0) = (142,377) */
            next_x = follow_me_pc_grid_x(142);
        } else {
            next_x = right_edge - 1;
        }
    }

    if (next_y <= top_edge) {
        if ((walls & FOLLOW_MFA_WALL_UP) == 0u) {
            --sFollowMfaVertical; /* original Vertical -= 1 */
            /* leave top (499,-1) + event offset (0,+665) = (499,664) */
            next_y = follow_me_pc_grid_y(664);
        } else {
            next_y = top_edge + 1;
        }
    } else if (next_y >= bottom_edge) {
        if ((walls & FOLLOW_MFA_WALL_DOWN) == 0u) {
            ++sFollowMfaVertical; /* original Vertical += 1 */
            /* leave bottom (525,771) + event offset (0,-649) = (525,122) */
            next_y = follow_me_pc_grid_y(122);
        } else {
            next_y = bottom_edge - 1;
        }
    }

    sFollowMePc.player_x = next_x;
    sFollowMePc.player_y = next_y;
}

static void follow_me_pc_update_room_transitions(void)
{
    /* Transitions are now performed by the four original MFA leave-edge events
     * in follow_me_pc_move_player().  Do not execute the old seven-room graph. */
}
