/*
 * Authentic PC-style Follow Me minigames.
 *
 * This module is intentionally self-contained and is not wired into main.c yet.
 * It provides the gameplay/state layer for the five Follow Me chapters while
 * the secret-minigame CI pass is being stabilised.  All names are prefixed
 * follow_pc_ so it can coexist with the current Wii U recreation until the
 * replacement is validated.
 */

#ifndef FNAF3_FOLLOW_ME_PC_INC
#define FNAF3_FOLLOW_ME_PC_INC

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum FollowPcChapter {
    FOLLOW_PC_FREDDY = 0,
    FOLLOW_PC_BONNIE = 1,
    FOLLOW_PC_CHICA = 2,
    FOLLOW_PC_FOXY = 3,
    FOLLOW_PC_FINAL_CHILD = 4,
    FOLLOW_PC_CHAPTER_COUNT = 5
} FollowPcChapter;

typedef enum FollowPcFacing {
    FOLLOW_PC_FACE_DOWN = 0,
    FOLLOW_PC_FACE_LEFT,
    FOLLOW_PC_FACE_RIGHT,
    FOLLOW_PC_FACE_UP
} FollowPcFacing;

typedef enum FollowPcPhase {
    FOLLOW_PC_PHASE_FOLLOW = 0,
    FOLLOW_PC_PHASE_SAFE_ROOM,
    FOLLOW_PC_PHASE_DISMANTLE,
    FOLLOW_PC_PHASE_FINAL_CONFRONTATION,
    FOLLOW_PC_PHASE_SPRINGLOCK,
    FOLLOW_PC_PHASE_COMPLETE
} FollowPcPhase;

typedef struct FollowPcRect {
    int x;
    int y;
    int w;
    int h;
} FollowPcRect;

typedef struct FollowPcPoint {
    int x;
    int y;
} FollowPcPoint;

typedef struct FollowPcState {
    bool active;
    FollowPcChapter chapter;
    FollowPcPhase phase;
    FollowPcFacing facing;
    int x;
    int y;
    int previous_x;
    int previous_y;
    int velocity_x;
    int velocity_y;
    int room;
    int shadow_waypoint;
    int walk_frame;
    uint32_t frames;
    uint32_t phase_frames;
    uint32_t walk_clock;
    bool shadow_visible;
    bool purple_visible;
    bool player_dismantled;
    bool springlock_started;
} FollowPcState;

/*
 * The PC minigame uses a low-resolution tile-like play field.  Wii U rendering
 * can scale these coordinates with nearest-neighbour sampling without changing
 * collision behaviour.  Exact sprite art is supplied separately by the asset
 * pack; these rectangles describe only collision/route logic.
 */
enum {
    FOLLOW_PC_WORLD_W = 854,
    FOLLOW_PC_WORLD_H = 480,
    FOLLOW_PC_PLAYER_W = 28,
    FOLLOW_PC_PLAYER_H = 34,
    FOLLOW_PC_SPEED = 3,
    FOLLOW_PC_WALK_FRAME_TICKS = 7
};

/* Shared route followed by Shadow Freddy toward the sealed safe room. */
static const FollowPcPoint kFollowPcShadowRoute[] = {
    {118, 347},
    {252, 347},
    {365, 347},
    {365, 257},
    {510, 257},
    {510, 168},
    {650, 168},
    {742, 168}
};

/*
 * Collision blocks are deliberately independent from graphics.  Their final
 * values can therefore be tuned against the MFA without touching the renderer.
 */
static const FollowPcRect kFollowPcWalls[] = {
    {0, 58, 854, 16},
    {0, 464, 854, 16},
    {0, 58, 16, 422},
    {838, 58, 16, 422},

    {92, 108, 16, 116},
    {92, 108, 166, 16},
    {242, 108, 16, 116},

    {316, 108, 16, 116},
    {316, 108, 166, 16},
    {466, 108, 16, 116},

    {540, 108, 16, 116},
    {540, 108, 166, 16},
    {690, 108, 16, 116},

    {92, 286, 16, 118},
    {92, 388, 166, 16},
    {242, 286, 16, 118},

    {316, 286, 16, 118},
    {316, 388, 166, 16},
    {466, 286, 16, 118},

    {540, 286, 16, 118},
    {540, 388, 166, 16},
    {690, 286, 16, 118}
};

static const FollowPcRect kFollowPcSafeRoom = {706, 105, 126, 126};
static const FollowPcPoint kFollowPcStart = {118, 347};
static const FollowPcPoint kFollowPcPurplePosition = {774, 165};

static int follow_pc_abs(int value)
{
    return value < 0 ? -value : value;
}

static bool follow_pc_point_in_rect(int x, int y, const FollowPcRect *rect)
{
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

static bool follow_pc_rects_overlap(int ax, int ay, int aw, int ah,
                                    const FollowPcRect *b)
{
    return ax < b->x + b->w && ax + aw > b->x &&
           ay < b->y + b->h && ay + ah > b->y;
}

static bool follow_pc_collides(int x, int y)
{
    const int count = (int) (sizeof(kFollowPcWalls) / sizeof(kFollowPcWalls[0]));
    for (int i = 0; i < count; ++i) {
        if (follow_pc_rects_overlap(x - FOLLOW_PC_PLAYER_W / 2,
                                   y - FOLLOW_PC_PLAYER_H,
                                   FOLLOW_PC_PLAYER_W,
                                   FOLLOW_PC_PLAYER_H,
                                   &kFollowPcWalls[i])) {
            return true;
        }
    }
    return false;
}

static void follow_pc_reset(FollowPcState *state, FollowPcChapter chapter)
{
    memset(state, 0, sizeof(*state));
    state->active = true;
    state->chapter = chapter;
    state->phase = chapter == FOLLOW_PC_FINAL_CHILD
        ? FOLLOW_PC_PHASE_FINAL_CONFRONTATION
        : FOLLOW_PC_PHASE_FOLLOW;
    state->facing = FOLLOW_PC_FACE_RIGHT;
    state->x = kFollowPcStart.x;
    state->y = kFollowPcStart.y;
    state->previous_x = state->x;
    state->previous_y = state->y;
    state->shadow_visible = chapter != FOLLOW_PC_FINAL_CHILD;
    state->purple_visible = chapter == FOLLOW_PC_FINAL_CHILD;
}

static void follow_pc_set_direction(FollowPcState *state, int dx, int dy)
{
    state->velocity_x = dx;
    state->velocity_y = dy;
    if (follow_pc_abs(dx) > follow_pc_abs(dy)) {
        state->facing = dx < 0 ? FOLLOW_PC_FACE_LEFT : FOLLOW_PC_FACE_RIGHT;
    } else if (dy != 0) {
        state->facing = dy < 0 ? FOLLOW_PC_FACE_UP : FOLLOW_PC_FACE_DOWN;
    }
}

static void follow_pc_try_move(FollowPcState *state, int dx, int dy)
{
    state->previous_x = state->x;
    state->previous_y = state->y;
    follow_pc_set_direction(state, dx, dy);

    const int next_x = state->x + dx;
    if (!follow_pc_collides(next_x, state->y)) state->x = next_x;

    const int next_y = state->y + dy;
    if (!follow_pc_collides(state->x, next_y)) state->y = next_y;

    if (state->x != state->previous_x || state->y != state->previous_y) {
        ++state->walk_clock;
        if (state->walk_clock >= FOLLOW_PC_WALK_FRAME_TICKS) {
            state->walk_clock = 0;
            state->walk_frame = (state->walk_frame + 1) & 1;
        }
    } else {
        state->walk_clock = 0;
        state->walk_frame = 0;
    }
}

static void follow_pc_update_shadow(FollowPcState *state)
{
    if (!state->shadow_visible || state->phase != FOLLOW_PC_PHASE_FOLLOW)
        return;

    const int route_count =
        (int) (sizeof(kFollowPcShadowRoute) / sizeof(kFollowPcShadowRoute[0]));
    if (state->shadow_waypoint >= route_count - 1) return;

    const FollowPcPoint *target = &kFollowPcShadowRoute[state->shadow_waypoint + 1];
    const int dx = target->x - state->x;
    const int dy = target->y - state->y;

    /* Shadow Freddy advances when the player gets close, reproducing the
     * stop-and-follow rhythm instead of moving continuously off screen. */
    if (follow_pc_abs(dx) + follow_pc_abs(dy) < 190)
        ++state->shadow_waypoint;
}

static FollowPcPoint follow_pc_shadow_position(const FollowPcState *state)
{
    const int route_count =
        (int) (sizeof(kFollowPcShadowRoute) / sizeof(kFollowPcShadowRoute[0]));
    int index = state->shadow_waypoint + 1;
    if (index < 0) index = 0;
    if (index >= route_count) index = route_count - 1;
    return kFollowPcShadowRoute[index];
}

static void follow_pc_update_phase(FollowPcState *state)
{
    ++state->phase_frames;

    if (state->chapter != FOLLOW_PC_FINAL_CHILD) {
        if (state->phase == FOLLOW_PC_PHASE_FOLLOW &&
            follow_pc_point_in_rect(state->x, state->y, &kFollowPcSafeRoom)) {
            state->phase = FOLLOW_PC_PHASE_SAFE_ROOM;
            state->phase_frames = 0;
            state->shadow_visible = false;
            state->purple_visible = true;
        } else if (state->phase == FOLLOW_PC_PHASE_SAFE_ROOM &&
                   state->phase_frames >= 55u) {
            state->phase = FOLLOW_PC_PHASE_DISMANTLE;
            state->phase_frames = 0;
            state->player_dismantled = true;
        } else if (state->phase == FOLLOW_PC_PHASE_DISMANTLE &&
                   state->phase_frames >= 125u) {
            state->phase = FOLLOW_PC_PHASE_COMPLETE;
            state->phase_frames = 0;
        }
        return;
    }

    /* Fifth chapter: the child corners Purple Guy; he enters Spring Bonnie,
     * the springlocks fail, then the chapter hands control back to progression. */
    if (state->phase == FOLLOW_PC_PHASE_FINAL_CONFRONTATION &&
        follow_pc_abs(state->x - kFollowPcPurplePosition.x) < 52 &&
        follow_pc_abs(state->y - kFollowPcPurplePosition.y) < 52) {
        state->phase = FOLLOW_PC_PHASE_SPRINGLOCK;
        state->phase_frames = 0;
        state->springlock_started = true;
    } else if (state->phase == FOLLOW_PC_PHASE_SPRINGLOCK &&
               state->phase_frames >= 210u) {
        state->phase = FOLLOW_PC_PHASE_COMPLETE;
        state->phase_frames = 0;
    }
}

/* Input-independent tick.  The caller supplies -1/0/1 axes after translating
 * Wii U controls, so GamePad, Pro Controller and Wii Remote mappings can share
 * exactly the same minigame logic. */
static void follow_pc_tick(FollowPcState *state, int axis_x, int axis_y)
{
    if (!state || !state->active || state->phase == FOLLOW_PC_PHASE_COMPLETE)
        return;

    ++state->frames;
    int dx = axis_x * FOLLOW_PC_SPEED;
    int dy = axis_y * FOLLOW_PC_SPEED;

    /* Match the original four-direction movement: no diagonal speed boost. */
    if (dx != 0 && dy != 0) dy = 0;

    if (state->phase == FOLLOW_PC_PHASE_FOLLOW ||
        state->phase == FOLLOW_PC_PHASE_FINAL_CONFRONTATION) {
        follow_pc_try_move(state, dx, dy);
    }

    follow_pc_update_shadow(state);
    follow_pc_update_phase(state);
}

static bool follow_pc_is_complete(const FollowPcState *state)
{
    return state && state->phase == FOLLOW_PC_PHASE_COMPLETE;
}

static const char *follow_pc_character_name(FollowPcChapter chapter)
{
    static const char *const names[FOLLOW_PC_CHAPTER_COUNT] = {
        "FREDDY",
        "BONNIE",
        "CHICA",
        "FOXY",
        "CHILD"
    };
    if ((int) chapter < 0 || chapter >= FOLLOW_PC_CHAPTER_COUNT) return "?";
    return names[chapter];
}

#endif /* FNAF3_FOLLOW_ME_PC_INC */
