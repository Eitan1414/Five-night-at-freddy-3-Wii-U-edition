/* Authentic PC-style Follow Me override.
 *
 * This layer leaves the existing finishing/save flow intact and intercepts only
 * FINISHING_FOLLOW_ME. Original PC sprites are supplied through
 * follow_me_pc_assets.c; the room graph and events mirror the five post-night
 * sequences while remaining controller-friendly on Wii U.
 */

#include <stdio.h>
#include <string.h>

typedef enum FollowMePcRoom {
    FOLLOW_ROOM_SHOW_STAGE = 0,
    FOLLOW_ROOM_DINING,
    FOLLOW_ROOM_HALL,
    FOLLOW_ROOM_SAFE_ENTRANCE,
    FOLLOW_ROOM_SAFE_ROOM
} FollowMePcRoom;

typedef enum FollowMePcPhase {
    FOLLOW_PHASE_GUIDE = 0,
    FOLLOW_PHASE_ERR,
    FOLLOW_PHASE_HUNT,
    FOLLOW_PHASE_DISMANTLED,
    FOLLOW_PHASE_GHOST_CHASE,
    FOLLOW_PHASE_SUIT,
    FOLLOW_PHASE_SPRINGLOCK
} FollowMePcPhase;

typedef enum FollowMePcDirection {
    FOLLOW_DIR_LEFT = 0,
    FOLLOW_DIR_UP,
    FOLLOW_DIR_RIGHT,
    FOLLOW_DIR_DOWN
} FollowMePcDirection;

typedef struct FollowMePcState {
    bool initialized;
    int chapter;
    FollowMePcRoom room;
    FollowMePcPhase phase;
    FollowMePcDirection direction;
    int player_x;
    int player_y;
    int shadow_x;
    int shadow_y;
    int purple_x;
    int purple_y;
    int purple_target;
    int chase_contacts;
    uint32_t contact_cooldown;
    uint32_t frames;
    uint32_t phase_frames;
    uint32_t move_frames;
    bool moving;
    bool crush_played;
} FollowMePcState;

static FollowMePcState sFollowMePc;

static int follow_me_pc_abs(int value)
{
    return value < 0 ? -value : value;
}

static bool follow_me_pc_near(int ax, int ay, int bx, int by, int radius)
{
    return follow_me_pc_abs(ax - bx) <= radius &&
           follow_me_pc_abs(ay - by) <= radius;
}

static void follow_me_pc_reset(int chapter)
{
    memset(&sFollowMePc, 0, sizeof(sFollowMePc));
    sFollowMePc.initialized = true;
    sFollowMePc.chapter = chapter;
    sFollowMePc.direction = FOLLOW_DIR_DOWN;
    sFollowMePc.room = chapter == 5
        ? FOLLOW_ROOM_SAFE_ENTRANCE : FOLLOW_ROOM_SHOW_STAGE;
    sFollowMePc.phase = FOLLOW_PHASE_GUIDE;
    if (chapter == 5) {
        sFollowMePc.player_x = 430;
        sFollowMePc.player_y = 390;
        sFollowMePc.shadow_x = -100;
        sFollowMePc.shadow_y = -100;
    } else {
        sFollowMePc.player_x = 430;
        sFollowMePc.player_y = 305;
        sFollowMePc.shadow_x = 430;
        sFollowMePc.shadow_y = 392;
    }
}

static void follow_me_pc_sync(void)
{
    sFinishing.minigame_x = sFollowMePc.player_x;
    sFinishing.minigame_y = sFollowMePc.player_y;
    sFinishing.minigame_waypoint = (int) sFollowMePc.room;
    sFinishing.minigame_event_frames = sFollowMePc.phase_frames;
}

static void follow_me_pc_move_player(const InputState *input)
{
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

    sFollowMePc.player_x += dx;
    sFollowMePc.player_y += dy;
    if (sFollowMePc.player_x < 35) sFollowMePc.player_x = 35;
    if (sFollowMePc.player_x > 819) sFollowMePc.player_x = 819;
    if (sFollowMePc.player_y < 68) sFollowMePc.player_y = 68;
    if (sFollowMePc.player_y > 428) sFollowMePc.player_y = 428;
}

static void follow_me_pc_enter_room(FollowMePcRoom room, int x, int y)
{
    sFollowMePc.room = room;
    sFollowMePc.player_x = x;
    sFollowMePc.player_y = y;
    sFollowMePc.phase_frames = 0u;
    switch (room) {
        case FOLLOW_ROOM_DINING:
            sFollowMePc.shadow_x = 245;
            sFollowMePc.shadow_y = 354;
            break;
        case FOLLOW_ROOM_HALL:
            sFollowMePc.shadow_x = 220;
            sFollowMePc.shadow_y = 320;
            break;
        case FOLLOW_ROOM_SAFE_ENTRANCE:
            sFollowMePc.shadow_x = 580;
            sFollowMePc.shadow_y = 340;
            break;
        default:
            break;
    }
}

static void follow_me_pc_update_shadow(void)
{
    if (sFollowMePc.chapter == 5 || sFollowMePc.phase != FOLLOW_PHASE_GUIDE)
        return;

    int target_x = sFollowMePc.shadow_x;
    int target_y = sFollowMePc.shadow_y;
    if (sFollowMePc.room == FOLLOW_ROOM_DINING) {
        target_x = 776;
        target_y = 240;
    } else if (sFollowMePc.room == FOLLOW_ROOM_HALL) {
        target_x = 665;
        target_y = 96;
    } else if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE) {
        target_x = 690;
        target_y = 90;
    }

    if (follow_me_pc_near(sFollowMePc.player_x, sFollowMePc.player_y,
                          sFollowMePc.shadow_x, sFollowMePc.shadow_y, 190)) {
        if (sFollowMePc.shadow_x < target_x) sFollowMePc.shadow_x += 2;
        else if (sFollowMePc.shadow_x > target_x) sFollowMePc.shadow_x -= 2;
        if (sFollowMePc.shadow_y < target_y) sFollowMePc.shadow_y += 2;
        else if (sFollowMePc.shadow_y > target_y) sFollowMePc.shadow_y -= 2;
    }
}

static void follow_me_pc_update_room_transitions(void)
{
    if (sFollowMePc.phase != FOLLOW_PHASE_GUIDE) return;

    if (sFollowMePc.chapter == 5) {
        if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE &&
            sFollowMePc.player_y <= 74 &&
            sFollowMePc.player_x >= 610 && sFollowMePc.player_x <= 780) {
            follow_me_pc_enter_room(FOLLOW_ROOM_SAFE_ROOM, 430, 394);
            sFollowMePc.phase = FOLLOW_PHASE_GHOST_CHASE;
            sFollowMePc.purple_x = 565;
            sFollowMePc.purple_y = 210;
            sFollowMePc.purple_target = 0;
            audio_play(AUDIO_CUE_MINIGAME_GLITCH, 0.55f, false);
        }
        return;
    }

    if (sFollowMePc.room == FOLLOW_ROOM_SHOW_STAGE &&
        sFollowMePc.player_y >= 425 &&
        sFollowMePc.player_x >= 320 && sFollowMePc.player_x <= 540) {
        follow_me_pc_enter_room(FOLLOW_ROOM_DINING, 430, 78);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_DINING &&
        sFollowMePc.player_x >= 816 &&
        sFollowMePc.player_y >= 165 && sFollowMePc.player_y <= 335) {
        follow_me_pc_enter_room(FOLLOW_ROOM_HALL, 42, 300);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_HALL &&
        sFollowMePc.player_y <= 72 &&
        sFollowMePc.player_x >= 560 && sFollowMePc.player_x <= 760) {
        follow_me_pc_enter_room(FOLLOW_ROOM_SAFE_ENTRANCE, 650, 420);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE &&
        sFollowMePc.player_y <= 78 &&
        sFollowMePc.player_x >= 610 && sFollowMePc.player_x <= 780) {
        sFollowMePc.phase = FOLLOW_PHASE_ERR;
        sFollowMePc.phase_frames = 1u;
        sFollowMePc.player_y = 116;
        audio_play(AUDIO_CUE_MINIGAME_GLITCH, 0.72f, false);
    }
}
