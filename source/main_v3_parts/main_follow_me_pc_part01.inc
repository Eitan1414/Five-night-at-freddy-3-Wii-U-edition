/* Authentic PC-style Follow Me override.
 *
 * This layer leaves the existing finishing/save flow intact and intercepts only
 * FINISHING_FOLLOW_ME. Original PC sprites are supplied through
 * follow_me_pc_assets.c; the room graph mirrors the post-night FNaF 1 route.
 */

#include <stdio.h>
#include <string.h>

typedef enum FollowMePcRoom {
    FOLLOW_ROOM_SHOW_STAGE = 0,
    FOLLOW_ROOM_DINING,
    FOLLOW_ROOM_WEST_HALL,
    FOLLOW_ROOM_EAST_HALL,
    FOLLOW_ROOM_PIRATE_COVE,
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

typedef struct FollowMePcObstacle {
    FollowMePcRoom room;
    int x;
    int y;
    int width;
    int height;
} FollowMePcObstacle;

static FollowMePcState sFollowMePc;

static const FollowMePcObstacle kFollowMePcObstacles[] = {
    { FOLLOW_ROOM_SHOW_STAGE, 145, 105, 565, 185 },
    { FOLLOW_ROOM_DINING, 72, 132, 180, 118 },
    { FOLLOW_ROOM_DINING, 336, 132, 180, 118 },
    { FOLLOW_ROOM_DINING, 602, 132, 180, 118 },
    { FOLLOW_ROOM_DINING, 205, 286, 180, 118 },
    { FOLLOW_ROOM_DINING, 470, 286, 180, 118 },
    { FOLLOW_ROOM_WEST_HALL, 94, 82, 205, 132 },
    { FOLLOW_ROOM_WEST_HALL, 334, 244, 190, 160 },
    { FOLLOW_ROOM_EAST_HALL, 88, 118, 172, 104 },
    { FOLLOW_ROOM_EAST_HALL, 620, 246, 136, 158 },
    { FOLLOW_ROOM_PIRATE_COVE, 82, 88, 130, 230 },
    { FOLLOW_ROOM_SAFE_ENTRANCE, 165, 270, 205, 130 },
    { FOLLOW_ROOM_SAFE_ENTRANCE, 390, 260, 205, 140 },
    { FOLLOW_ROOM_SAFE_ROOM, 112, 278, 108, 126 },
    { FOLLOW_ROOM_SAFE_ROOM, 265, 86, 102, 126 },
    { FOLLOW_ROOM_SAFE_ROOM, 382, 86, 102, 126 },
    { FOLLOW_ROOM_SAFE_ROOM, 500, 86, 102, 126 },
    { FOLLOW_ROOM_SAFE_ROOM, 714, 82, 108, 160 }
};

static int follow_me_pc_abs(int value)
{
    return value < 0 ? -value : value;
}

static bool follow_me_pc_near(int ax, int ay, int bx, int by, int radius)
{
    return follow_me_pc_abs(ax - bx) <= radius &&
           follow_me_pc_abs(ay - by) <= radius;
}

static bool follow_me_pc_position_blocked(int x, int y)
{
    const int half_width = 13;
    const int foot_height = 17;
    const int left = x - half_width;
    const int right = x + half_width;
    const int top = y - foot_height;
    const int bottom = y + 2;

    for (size_t index = 0u;
         index < sizeof(kFollowMePcObstacles) / sizeof(kFollowMePcObstacles[0]);
         ++index) {
        const FollowMePcObstacle *obstacle = &kFollowMePcObstacles[index];
        if (obstacle->room != sFollowMePc.room) continue;
        if (right <= obstacle->x || left >= obstacle->x + obstacle->width ||
            bottom <= obstacle->y || top >= obstacle->y + obstacle->height)
            continue;
        return true;
    }
    return false;
}

static void follow_me_pc_reset(int chapter)
{
    memset(&sFollowMePc, 0, sizeof(sFollowMePc));
    sFollowMePc.initialized = true;
    sFollowMePc.chapter = chapter;
    sFollowMePc.direction = FOLLOW_DIR_DOWN;
    sFollowMePc.phase = FOLLOW_PHASE_GUIDE;

    if (chapter == 4) {
        sFollowMePc.room = FOLLOW_ROOM_PIRATE_COVE;
        sFollowMePc.player_x = 240;
        sFollowMePc.player_y = 352;
        sFollowMePc.shadow_x = 705;
        sFollowMePc.shadow_y = 345;
    } else {
        sFollowMePc.room = FOLLOW_ROOM_SHOW_STAGE;
        sFollowMePc.player_x = 430;
        sFollowMePc.player_y = 325;
        if (chapter == 5) {
            sFollowMePc.shadow_x = -100;
            sFollowMePc.shadow_y = -100;
        } else {
            sFollowMePc.shadow_x = 430;
            sFollowMePc.shadow_y = 395;
        }
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

    int next_x = sFollowMePc.player_x + dx;
    if (next_x < 35) next_x = 35;
    if (next_x > 819) next_x = 819;
    if (!follow_me_pc_position_blocked(next_x, sFollowMePc.player_y))
        sFollowMePc.player_x = next_x;

    int next_y = sFollowMePc.player_y + dy;
    if (next_y < 68) next_y = 68;
    if (next_y > 428) next_y = 428;
    if (!follow_me_pc_position_blocked(sFollowMePc.player_x, next_y))
        sFollowMePc.player_y = next_y;
}

static void follow_me_pc_enter_room(FollowMePcRoom room, int x, int y)
{
    sFollowMePc.room = room;
    sFollowMePc.player_x = x;
    sFollowMePc.player_y = y;
    sFollowMePc.phase_frames = 0u;
    switch (room) {
        case FOLLOW_ROOM_DINING:
            sFollowMePc.shadow_x = 300;
            sFollowMePc.shadow_y = 355;
            break;
        case FOLLOW_ROOM_WEST_HALL:
            sFollowMePc.shadow_x = 590;
            sFollowMePc.shadow_y = 320;
            break;
        case FOLLOW_ROOM_EAST_HALL:
            sFollowMePc.shadow_x = 265;
            sFollowMePc.shadow_y = 318;
            break;
        case FOLLOW_ROOM_PIRATE_COVE:
            sFollowMePc.shadow_x = 705;
            sFollowMePc.shadow_y = 345;
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
    if (sFollowMePc.room == FOLLOW_ROOM_PIRATE_COVE) {
        target_x = 790;
        target_y = 345;
    } else if (sFollowMePc.room == FOLLOW_ROOM_DINING) {
        target_x = 776;
        target_y = 240;
    } else if (sFollowMePc.room == FOLLOW_ROOM_WEST_HALL) {
        target_x = 790;
        target_y = 300;
    } else if (sFollowMePc.room == FOLLOW_ROOM_EAST_HALL) {
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

    if (sFollowMePc.room == FOLLOW_ROOM_SHOW_STAGE &&
        sFollowMePc.player_y >= 425 &&
        sFollowMePc.player_x >= 320 && sFollowMePc.player_x <= 540) {
        follow_me_pc_enter_room(FOLLOW_ROOM_DINING, 430, 78);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_DINING &&
        sFollowMePc.player_y <= 72 &&
        sFollowMePc.player_x >= 320 && sFollowMePc.player_x <= 540) {
        follow_me_pc_enter_room(FOLLOW_ROOM_SHOW_STAGE, 430, 420);
        return;
    }

    if (sFollowMePc.room == FOLLOW_ROOM_DINING &&
        sFollowMePc.player_x <= 38 &&
        sFollowMePc.player_y >= 165 && sFollowMePc.player_y <= 360) {
        follow_me_pc_enter_room(FOLLOW_ROOM_WEST_HALL, 812, 300);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_WEST_HALL &&
        sFollowMePc.player_x >= 816 &&
        sFollowMePc.player_y >= 165 && sFollowMePc.player_y <= 370) {
        follow_me_pc_enter_room(FOLLOW_ROOM_DINING, 42, 300);
        return;
    }

    if (sFollowMePc.room == FOLLOW_ROOM_PIRATE_COVE &&
        sFollowMePc.player_x >= 816 &&
        sFollowMePc.player_y >= 170 && sFollowMePc.player_y <= 420) {
        follow_me_pc_enter_room(FOLLOW_ROOM_WEST_HALL, 42, 300);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_WEST_HALL &&
        sFollowMePc.player_x <= 38 &&
        sFollowMePc.player_y >= 170 && sFollowMePc.player_y <= 370) {
        follow_me_pc_enter_room(FOLLOW_ROOM_PIRATE_COVE, 812, 300);
        return;
    }

    if (sFollowMePc.room == FOLLOW_ROOM_DINING &&
        sFollowMePc.player_x >= 816 &&
        sFollowMePc.player_y >= 165 && sFollowMePc.player_y <= 360) {
        follow_me_pc_enter_room(FOLLOW_ROOM_EAST_HALL, 42, 300);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_EAST_HALL &&
        sFollowMePc.player_x <= 38 &&
        sFollowMePc.player_y >= 165 && sFollowMePc.player_y <= 370) {
        follow_me_pc_enter_room(FOLLOW_ROOM_DINING, 812, 300);
        return;
    }

    if (sFollowMePc.room == FOLLOW_ROOM_EAST_HALL &&
        sFollowMePc.player_y <= 72 &&
        sFollowMePc.player_x >= 560 && sFollowMePc.player_x <= 760) {
        follow_me_pc_enter_room(FOLLOW_ROOM_SAFE_ENTRANCE, 650, 420);
        return;
    }
    if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE &&
        sFollowMePc.player_y >= 426 &&
        sFollowMePc.player_x >= 560 && sFollowMePc.player_x <= 760) {
        follow_me_pc_enter_room(FOLLOW_ROOM_EAST_HALL, 650, 78);
        return;
    }

    if (sFollowMePc.room == FOLLOW_ROOM_SAFE_ENTRANCE &&
        sFollowMePc.player_y <= 78 &&
        sFollowMePc.player_x >= 610 && sFollowMePc.player_x <= 780) {
        if (sFollowMePc.chapter == 5) {
            follow_me_pc_enter_room(FOLLOW_ROOM_SAFE_ROOM, 430, 394);
            sFollowMePc.phase = FOLLOW_PHASE_GHOST_CHASE;
            sFollowMePc.purple_x = 565;
            sFollowMePc.purple_y = 210;
            sFollowMePc.purple_target = 0;
            audio_play(AUDIO_CUE_MINIGAME_GLITCH, 0.55f, false);
        } else {
            sFollowMePc.phase = FOLLOW_PHASE_ERR;
            sFollowMePc.phase_frames = 1u;
            sFollowMePc.player_y = 116;
            audio_play(AUDIO_CUE_MINIGAME_GLITCH, 0.72f, false);
        }
    }
}
