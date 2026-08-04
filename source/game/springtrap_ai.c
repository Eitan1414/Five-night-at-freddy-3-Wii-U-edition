#include "game/springtrap_ai.h"

#include <stddef.h>

#define VENT_TRAVEL_BASE_FRAMES 180u
#define OFFICE_ATTACK_BASE_FRAMES 360u

static const int kNeighbours[SPRINGTRAP_CAMERA_COUNT][4] = {
    {1, 5, -1, -1},
    {0, 2, 4, -1},
    {1, 3, -1, -1},
    {2, 4, 9, -1},
    {1, 3, 5, 7},
    {0, 4, 6, -1},
    {5, 7, -1, -1},
    {4, 6, 8, -1},
    {7, 9, -1, -1},
    {3, 8, -1, -1},
};

static uint32_t next_random(SpringtrapAI *ai)
{
    ai->rng = ai->rng * 1664525u + 1013904223u;
    return ai->rng;
}

static int neighbour_count(int camera)
{
    int count = 0;
    if (camera < 0 || camera >= SPRINGTRAP_CAMERA_COUNT) {
        return 0;
    }
    while (count < 4 && kNeighbours[camera][count] >= 0) {
        ++count;
    }
    return count;
}

bool springtrap_ai_cameras_adjacent(int first, int second)
{
    const int count = neighbour_count(first);
    for (int index = 0; index < count; ++index) {
        if (kNeighbours[first][index] == second) {
            return true;
        }
    }
    return false;
}

static SpringtrapVent vent_for_camera(int camera)
{
    switch (camera) {
        case 8: return SPRINGTRAP_VENT_11;
        case 6: return SPRINGTRAP_VENT_12;
        case 4: return SPRINGTRAP_VENT_13;
        case 9: return SPRINGTRAP_VENT_14;
        case 1: return SPRINGTRAP_VENT_15;
        default: return SPRINGTRAP_VENT_NONE;
    }
}

static uint32_t vent_travel_frames(int night_number)
{
    uint32_t reduction = night_number > 1
        ? (uint32_t) (night_number - 1) * 12u
        : 0u;
    return reduction < 72u ? VENT_TRAVEL_BASE_FRAMES - reduction : 108u;
}

static uint32_t office_attack_frames(int night_number)
{
    uint32_t reduction = night_number > 1
        ? (uint32_t) (night_number - 1) * 45u
        : 0u;
    return reduction < 240u ? OFFICE_ATTACK_BASE_FRAMES - reduction : 120u;
}

static uint32_t event_danger_for_camera(int camera)
{
    if (camera == 4) {
        return SPRINGTRAP_EVENT_DANGER_SOFT;
    }
    if (camera == 0 || camera == 1) {
        return SPRINGTRAP_EVENT_DANGER_LOUD;
    }
    return SPRINGTRAP_EVENT_NONE;
}

void springtrap_ai_reset(SpringtrapAI *ai, int night_number, uint32_t seed)
{
    if (ai == NULL) {
        return;
    }
    ai->kind = SPRINGTRAP_LOCATION_CAMERA;
    ai->camera = 6 + (int) ((seed + (uint32_t) night_number * 17u) % 4u);
    ai->vent_source_camera = -1;
    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->move_frames = 0u;
    ai->vent_frames = 0u;
    ai->rng = seed ^ (0x5F3759DFu + (uint32_t) night_number * 0x1021u);
    ai->attack_reported = false;
}

static SpringtrapEvent empty_event(void)
{
    SpringtrapEvent event = {
        .flags = SPRINGTRAP_EVENT_NONE,
        .from_camera = -1,
        .to_camera = -1,
        .vent = SPRINGTRAP_VENT_NONE,
    };
    return event;
}

static SpringtrapEvent enter_vent(SpringtrapAI *ai, SpringtrapVent vent)
{
    SpringtrapEvent event = empty_event();
    event.flags = SPRINGTRAP_EVENT_VENT_ENTER;
    event.from_camera = ai->camera;
    event.vent = vent;
    ai->kind = SPRINGTRAP_LOCATION_VENT;
    ai->vent_source_camera = ai->camera;
    ai->vent = vent;
    ai->vent_frames = 0u;
    ai->move_frames = 0u;
    return event;
}

static SpringtrapEvent exit_vent(SpringtrapAI *ai,
                                 SpringtrapVent sealed_vent)
{
    SpringtrapEvent event = empty_event();
    event.flags = SPRINGTRAP_EVENT_VENT_EXIT | SPRINGTRAP_EVENT_MOVED;
    event.from_camera = ai->vent_source_camera;
    event.vent = ai->vent;

    if (sealed_vent == ai->vent) {
        ai->kind = SPRINGTRAP_LOCATION_CAMERA;
        ai->camera = ai->vent_source_camera;
        event.to_camera = ai->camera;
    } else {
        switch (ai->vent) {
            case SPRINGTRAP_VENT_11:
                ai->kind = SPRINGTRAP_LOCATION_CAMERA;
                ai->camera = 5;
                event.to_camera = ai->camera;
                break;
            case SPRINGTRAP_VENT_12:
                ai->kind = SPRINGTRAP_LOCATION_HALL_HIDDEN;
                ai->camera = -1;
                event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD;
                break;
            case SPRINGTRAP_VENT_13:
                ai->kind = SPRINGTRAP_LOCATION_OFFICE_WINDOW;
                ai->camera = -1;
                event.flags |= SPRINGTRAP_EVENT_DANGER_SOFT;
                break;
            case SPRINGTRAP_VENT_14:
                ai->kind = SPRINGTRAP_LOCATION_CAMERA;
                ai->camera = 1;
                event.to_camera = ai->camera;
                event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD;
                break;
            case SPRINGTRAP_VENT_15:
                ai->kind = SPRINGTRAP_LOCATION_OFFICE_INSIDE;
                ai->camera = -1;
                event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD |
                               SPRINGTRAP_EVENT_ATTACK;
                ai->attack_reported = true;
                break;
            default:
                ai->kind = SPRINGTRAP_LOCATION_CAMERA;
                ai->camera = ai->vent_source_camera;
                event.to_camera = ai->camera;
                break;
        }
    }

    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->vent_source_camera = -1;
    ai->vent_frames = 0u;
    ai->move_frames = 0u;
    return event;
}

static SpringtrapEvent move_to_random_neighbour(SpringtrapAI *ai)
{
    SpringtrapEvent event = empty_event();
    const int current = ai->camera;
    const int count = neighbour_count(current);
    if (count <= 0) {
        event.flags = SPRINGTRAP_EVENT_STAYED;
        ai->move_frames = 0u;
        return event;
    }

    if (current == 0 && (next_random(ai) % 100u) < 28u) {
        ai->kind = SPRINGTRAP_LOCATION_OFFICE_LEFT;
        ai->camera = -1;
        ai->move_frames = 0u;
        event.flags = SPRINGTRAP_EVENT_MOVED |
                      SPRINGTRAP_EVENT_DANGER_LOUD;
        event.from_camera = current;
        return event;
    }

    const int next = kNeighbours[current][next_random(ai) % (uint32_t) count];
    ai->camera = next;
    ai->move_frames = 0u;
    event.flags = SPRINGTRAP_EVENT_MOVED | event_danger_for_camera(next);
    event.from_camera = current;
    event.to_camera = next;
    return event;
}

static SpringtrapEvent movement_opportunity(SpringtrapAI *ai,
                                             int current_hour)
{
    SpringtrapEvent event = empty_event();
    const SpringtrapVent available_vent = vent_for_camera(ai->camera);
    uint32_t choice = next_random(ai) % 3u;

    if (choice == 2u &&
        (current_hour < 1 || available_vent == SPRINGTRAP_VENT_NONE)) {
        choice = next_random(ai) & 1u;
    }

    if (choice == 0u) {
        ai->move_frames = 0u;
        event.flags = SPRINGTRAP_EVENT_STAYED;
        event.from_camera = ai->camera;
        event.to_camera = ai->camera;
        return event;
    }
    if (choice == 2u) {
        return enter_vent(ai, available_vent);
    }
    return move_to_random_neighbour(ai);
}

SpringtrapEvent springtrap_ai_update(SpringtrapAI *ai,
                                     int night_number,
                                     int current_hour,
                                     uint32_t movement_opportunity_frames,
                                     SpringtrapVent sealed_vent,
                                     bool directly_observed,
                                     bool player_blinded)
{
    SpringtrapEvent event = empty_event();
    if (ai == NULL) {
        return event;
    }

    switch (ai->kind) {
        case SPRINGTRAP_LOCATION_CAMERA:
            ++ai->move_frames;
            if (ai->move_frames >= movement_opportunity_frames) {
                event = movement_opportunity(ai, current_hour);
            }
            break;
        case SPRINGTRAP_LOCATION_VENT:
            ++ai->vent_frames;
            if (ai->vent_frames >= vent_travel_frames(night_number)) {
                event = exit_vent(ai, sealed_vent);
            }
            break;
        case SPRINGTRAP_LOCATION_HALL_HIDDEN:
            ++ai->move_frames;
            if (ai->move_frames >= movement_opportunity_frames) {
                ai->move_frames = 0u;
                event.flags = SPRINGTRAP_EVENT_MOVED;
                if ((next_random(ai) & 1u) == 0u) {
                    ai->kind = SPRINGTRAP_LOCATION_CAMERA;
                    ai->camera = 0;
                    event.to_camera = 0;
                    event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD;
                } else {
                    ai->kind = SPRINGTRAP_LOCATION_OFFICE_WINDOW;
                    ai->camera = -1;
                    event.flags |= SPRINGTRAP_EVENT_DANGER_SOFT;
                }
            }
            break;
        case SPRINGTRAP_LOCATION_OFFICE_WINDOW:
        case SPRINGTRAP_LOCATION_OFFICE_LEFT:
            if (directly_observed && !player_blinded) {
                break;
            }
            ++ai->move_frames;
            if (ai->move_frames >= office_attack_frames(night_number)) {
                ai->kind = SPRINGTRAP_LOCATION_OFFICE_INSIDE;
                ai->move_frames = 0u;
                ai->attack_reported = true;
                event.flags = SPRINGTRAP_EVENT_MOVED |
                              SPRINGTRAP_EVENT_DANGER_LOUD |
                              SPRINGTRAP_EVENT_ATTACK;
            }
            break;
        case SPRINGTRAP_LOCATION_OFFICE_INSIDE:
            if (!ai->attack_reported) {
                ai->attack_reported = true;
                event.flags = SPRINGTRAP_EVENT_ATTACK;
            }
            break;
    }
    return event;
}

SpringtrapEvent springtrap_ai_lure(SpringtrapAI *ai,
                                   int target_camera,
                                   int night_number)
{
    SpringtrapEvent event = empty_event();
    if (ai == NULL || target_camera < 0 ||
        target_camera >= SPRINGTRAP_CAMERA_COUNT ||
        ai->kind != SPRINGTRAP_LOCATION_CAMERA ||
        target_camera == ai->camera ||
        !springtrap_ai_cameras_adjacent(ai->camera, target_camera)) {
        event.flags = SPRINGTRAP_EVENT_LURE_INVALID;
        return event;
    }

    if (night_number >= 3 && (next_random(ai) % 100u) < 20u) {
        event.flags = SPRINGTRAP_EVENT_LURE_IGNORED;
        event.from_camera = ai->camera;
        event.to_camera = target_camera;
        return event;
    }

    event.flags = SPRINGTRAP_EVENT_MOVED |
                  event_danger_for_camera(target_camera);
    event.from_camera = ai->camera;
    event.to_camera = target_camera;
    ai->camera = target_camera;
    ai->move_frames = 0u;
    return event;
}

void springtrap_ai_release_observation(SpringtrapAI *ai)
{
    if (ai != NULL &&
        (ai->kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW ||
         ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT)) {
        ai->move_frames += 90u;
    }
}

bool springtrap_ai_is_on_camera(const SpringtrapAI *ai, int camera)
{
    return ai != NULL && ai->kind == SPRINGTRAP_LOCATION_CAMERA &&
           ai->camera == camera;
}

bool springtrap_ai_is_in_vent(const SpringtrapAI *ai)
{
    return ai != NULL && ai->kind == SPRINGTRAP_LOCATION_VENT;
}

int springtrap_ai_camera(const SpringtrapAI *ai)
{
    return ai != NULL && ai->kind == SPRINGTRAP_LOCATION_CAMERA
        ? ai->camera : -1;
}

SpringtrapVent springtrap_ai_current_vent(const SpringtrapAI *ai)
{
    return ai != NULL ? ai->vent : SPRINGTRAP_VENT_NONE;
}

SpringtrapOfficeSide springtrap_ai_office_side(const SpringtrapAI *ai)
{
    if (ai == NULL) {
        return SPRINGTRAP_OFFICE_NONE;
    }
    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW) {
        return SPRINGTRAP_OFFICE_WINDOW;
    }
    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT) {
        return SPRINGTRAP_OFFICE_LEFT;
    }
    return SPRINGTRAP_OFFICE_NONE;
}

bool springtrap_ai_is_danger_near(const SpringtrapAI *ai)
{
    if (ai == NULL) {
        return false;
    }
    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW ||
        ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT ||
        ai->kind == SPRINGTRAP_LOCATION_OFFICE_INSIDE ||
        ai->kind == SPRINGTRAP_LOCATION_HALL_HIDDEN) {
        return true;
    }
    return ai->kind == SPRINGTRAP_LOCATION_CAMERA &&
           (ai->camera == 0 || ai->camera == 1 || ai->camera == 4);
}

const char *springtrap_vent_label(SpringtrapVent vent)
{
    static const char *const labels[SPRINGTRAP_VENT_COUNT] = {
        "VENT 11: CAM 09 -> CAM 06",
        "VENT 12: CAM 07 -> UPPER HALL",
        "VENT 13: CAM 05 -> WINDOW",
        "VENT 14: CAM 10 -> CAM 02",
        "VENT 15: CAM 02 -> OFFICE",
    };
    return vent >= 0 && vent < SPRINGTRAP_VENT_COUNT
        ? labels[vent] : "NO VENT";
}

int springtrap_vent_source_camera(SpringtrapVent vent)
{
    static const int sources[SPRINGTRAP_VENT_COUNT] = {8, 6, 4, 9, 1};
    return vent >= 0 && vent < SPRINGTRAP_VENT_COUNT ? sources[vent] : -1;
}

int springtrap_vent_destination_camera(SpringtrapVent vent)
{
    static const int destinations[SPRINGTRAP_VENT_COUNT] = {5, -1, -1, 1, -1};
    return vent >= 0 && vent < SPRINGTRAP_VENT_COUNT
        ? destinations[vent] : -1;
}
