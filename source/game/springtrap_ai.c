#include "game/springtrap_ai.h"

#include <stddef.h>

#define FRAMES_PER_SECOND 60u
#define VENT_TRAVEL_FRAMES (3u * FRAMES_PER_SECOND)
#define WINDOW_MOVE_CHECK_FRAMES 90u
#define OFFICE_PANEL_GRACE_FRAMES (6u * FRAMES_PER_SECOND)

/* Zero-based camera graph: CAM 01 == 0, ... CAM 10 == 9. */
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

/* Distance, in normal rooms, from the two office approaches. This lets the
 * two camera positions represent the doorway toward the office versus the
 * doorway back into the attraction. */
static const uint8_t kOfficeDistance[SPRINGTRAP_CAMERA_COUNT] = {
    1u, 1u, 2u, 3u, 2u, 2u, 3u, 3u, 4u, 4u,
};

static uint32_t next_random(SpringtrapAI *ai)
{
    ai->rng = ai->rng * 1664525u + 1013904223u;
    return ai->rng;
}

static int neighbour_count(int camera)
{
    int count = 0;
    if (camera < 0 || camera >= SPRINGTRAP_CAMERA_COUNT) return 0;
    while (count < 4 && kNeighbours[camera][count] >= 0) ++count;
    return count;
}

bool springtrap_ai_cameras_adjacent(int first, int second)
{
    const int count = neighbour_count(first);
    for (int index = 0; index < count; ++index) {
        if (kNeighbours[first][index] == second) return true;
    }
    return false;
}

static SpringtrapVent vent_for_camera(int camera)
{
    switch (camera) {
        case 8: return SPRINGTRAP_VENT_11; /* CAM 09 -> CAM 07 */
        case 6: return SPRINGTRAP_VENT_12; /* CAM 07 -> CAM 01 */
        case 4: return SPRINGTRAP_VENT_13; /* CAM 05 -> window hall */
        case 9: return SPRINGTRAP_VENT_14; /* CAM 10 -> office */
        case 1: return SPRINGTRAP_VENT_15; /* CAM 02 -> office */
        default: return SPRINGTRAP_VENT_NONE;
    }
}

static void opportunity_range(int night_number,
                              uint32_t *minimum_seconds,
                              uint32_t *maximum_seconds)
{
    static const uint8_t minimums[6] = {0u, 9u, 8u, 7u, 6u, 4u};
    static const uint8_t maximums[6] = {0u, 23u, 22u, 21u, 20u, 18u};
    if (night_number < 1) night_number = 1;
    if (night_number > 6) night_number = 6;
    *minimum_seconds = minimums[night_number - 1];
    *maximum_seconds = maximums[night_number - 1];
}

static void schedule_next_opportunity(SpringtrapAI *ai, int night_number)
{
    uint32_t minimum_seconds = 0u;
    uint32_t maximum_seconds = 0u;
    opportunity_range(night_number, &minimum_seconds, &maximum_seconds);
    ai->move_frames = 0u;

    if (minimum_seconds == 0u || maximum_seconds < minimum_seconds) {
        ai->opportunity_frames = UINT32_MAX;
        return;
    }

    const uint32_t span = maximum_seconds - minimum_seconds + 1u;
    const uint32_t seconds = minimum_seconds + (next_random(ai) % span);
    ai->opportunity_frames = seconds * FRAMES_PER_SECOND;
}

static uint32_t event_danger_for_camera(int camera)
{
    if (camera == 4) return SPRINGTRAP_EVENT_DANGER_SOFT;
    if (camera == 0 || camera == 1) return SPRINGTRAP_EVENT_DANGER_LOUD;
    return SPRINGTRAP_EVENT_NONE;
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

void springtrap_ai_reset(SpringtrapAI *ai, int night_number, uint32_t seed)
{
    if (ai == NULL) return;

    ai->rng = seed ^ (0x5F3759DFu + (uint32_t) night_number * 0x1021u);
    ai->kind = SPRINGTRAP_LOCATION_CAMERA;
    ai->camera = 7 + (int) (next_random(ai) % 3u); /* CAM 08 / 09 / 10 */
    ai->camera_position = (next_random(ai) & 1u) != 0u
        ? SPRINGTRAP_CAMERA_POSITION_EXIT
        : SPRINGTRAP_CAMERA_POSITION_BACK;
    ai->vent_source_camera = -1;
    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->move_frames = 0u;
    ai->opportunity_frames = UINT32_MAX;
    ai->vent_frames = 0u;
    ai->office_inside_frames = 0u;
    ai->attack_reported = false;
    ai->office_panel_grace = false;
    schedule_next_opportunity(ai, night_number);
}

static SpringtrapCameraPosition position_for_target(int current, int target)
{
    if (current < 0 || current >= SPRINGTRAP_CAMERA_COUNT ||
        target < 0 || target >= SPRINGTRAP_CAMERA_COUNT) {
        return SPRINGTRAP_CAMERA_POSITION_EXIT;
    }

    if (kOfficeDistance[target] < kOfficeDistance[current])
        return SPRINGTRAP_CAMERA_POSITION_EXIT;
    if (kOfficeDistance[target] > kOfficeDistance[current])
        return SPRINGTRAP_CAMERA_POSITION_BACK;

    /* Lateral links still use one of the two physical door positions in a
     * deterministic way, so movement cannot teleport between room exits. */
    return target < current
        ? SPRINGTRAP_CAMERA_POSITION_EXIT
        : SPRINGTRAP_CAMERA_POSITION_BACK;
}

static SpringtrapEvent change_camera_position(SpringtrapAI *ai,
                                               SpringtrapCameraPosition position,
                                               int night_number)
{
    SpringtrapEvent event = empty_event();
    event.flags = SPRINGTRAP_EVENT_POSITION_CHANGED;
    event.from_camera = ai->camera;
    event.to_camera = ai->camera;
    ai->camera_position = position;
    schedule_next_opportunity(ai, night_number);
    return event;
}

static SpringtrapEvent enter_vent(SpringtrapAI *ai,
                                  SpringtrapVent vent,
                                  int night_number)
{
    if (ai->camera_position != SPRINGTRAP_CAMERA_POSITION_EXIT) {
        return change_camera_position(ai,
                                      SPRINGTRAP_CAMERA_POSITION_EXIT,
                                      night_number);
    }

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

static void enter_office_inside(SpringtrapAI *ai, bool panel_open)
{
    ai->kind = SPRINGTRAP_LOCATION_OFFICE_INSIDE;
    ai->camera = -1;
    ai->move_frames = 0u;
    ai->office_inside_frames = 0u;
    ai->attack_reported = false;
    ai->office_panel_grace = panel_open;
}

static SpringtrapEvent exit_vent(SpringtrapAI *ai,
                                 SpringtrapVent sealed_vent,
                                 int night_number,
                                 bool panel_open)
{
    SpringtrapEvent event = empty_event();
    event.flags = SPRINGTRAP_EVENT_VENT_EXIT | SPRINGTRAP_EVENT_MOVED;
    event.from_camera = ai->vent_source_camera;
    event.vent = ai->vent;

    if (sealed_vent == ai->vent) {
        ai->kind = SPRINGTRAP_LOCATION_CAMERA;
        ai->camera = ai->vent_source_camera;
        ai->camera_position = SPRINGTRAP_CAMERA_POSITION_BACK;
        event.to_camera = ai->camera;
        schedule_next_opportunity(ai, night_number);
    } else {
        switch (ai->vent) {
            case SPRINGTRAP_VENT_11:
                ai->kind = SPRINGTRAP_LOCATION_CAMERA;
                ai->camera = 6; /* CAM 07 */
                ai->camera_position = SPRINGTRAP_CAMERA_POSITION_BACK;
                event.to_camera = ai->camera;
                schedule_next_opportunity(ai, night_number);
                break;
            case SPRINGTRAP_VENT_12:
                ai->kind = SPRINGTRAP_LOCATION_CAMERA;
                ai->camera = 0; /* CAM 01 */
                ai->camera_position = SPRINGTRAP_CAMERA_POSITION_BACK;
                event.to_camera = ai->camera;
                event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD;
                schedule_next_opportunity(ai, night_number);
                break;
            case SPRINGTRAP_VENT_13:
                ai->kind = SPRINGTRAP_LOCATION_HALL_HIDDEN;
                ai->camera = -1;
                ai->move_frames = 0u;
                event.flags |= SPRINGTRAP_EVENT_DANGER_SOFT;
                break;
            case SPRINGTRAP_VENT_14:
            case SPRINGTRAP_VENT_15:
                enter_office_inside(ai, panel_open);
                event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD;
                if (!panel_open) {
                    event.flags |= SPRINGTRAP_EVENT_ATTACK;
                    ai->attack_reported = true;
                }
                break;
            default:
                ai->kind = SPRINGTRAP_LOCATION_CAMERA;
                ai->camera = ai->vent_source_camera;
                ai->camera_position = SPRINGTRAP_CAMERA_POSITION_BACK;
                event.to_camera = ai->camera;
                schedule_next_opportunity(ai, night_number);
                break;
        }
    }

    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->vent_source_camera = -1;
    ai->vent_frames = 0u;
    return event;
}

static SpringtrapEvent move_to_target(SpringtrapAI *ai,
                                      int target,
                                      int night_number)
{
    SpringtrapEvent event = empty_event();
    const int current = ai->camera;
    const SpringtrapCameraPosition required = position_for_target(current, target);

    if (ai->camera_position != required) {
        return change_camera_position(ai, required, night_number);
    }

    ai->camera = target;
    /* Entering through one doorway leaves him at the opposite side of the new
     * camera, which is why a later opportunity may first change his sprite
     * position before he can leave through another exit. */
    ai->camera_position = required == SPRINGTRAP_CAMERA_POSITION_EXIT
        ? SPRINGTRAP_CAMERA_POSITION_BACK
        : SPRINGTRAP_CAMERA_POSITION_EXIT;
    schedule_next_opportunity(ai, night_number);

    event.flags = SPRINGTRAP_EVENT_MOVED | event_danger_for_camera(target);
    event.from_camera = current;
    event.to_camera = target;
    return event;
}

static SpringtrapEvent move_to_random_neighbour(SpringtrapAI *ai,
                                                 int night_number)
{
    SpringtrapEvent event = empty_event();
    const int current = ai->camera;
    const int count = neighbour_count(current);

    /* CAM 01 can lead to the office door and CAM 02 can lead to the window.
     * They are treated as additional adjacent destinations. */
    const int office_option = (current == 0 || current == 1) ? 1 : 0;
    const int total = count + office_option;
    if (total <= 0) {
        event.flags = SPRINGTRAP_EVENT_STAYED;
        event.from_camera = current;
        event.to_camera = current;
        schedule_next_opportunity(ai, night_number);
        return event;
    }

    const int pick = (int) (next_random(ai) % (uint32_t) total);
    if (office_option != 0 && pick == count) {
        if (ai->camera_position != SPRINGTRAP_CAMERA_POSITION_EXIT) {
            return change_camera_position(ai,
                                          SPRINGTRAP_CAMERA_POSITION_EXIT,
                                          night_number);
        }

        event.flags = SPRINGTRAP_EVENT_MOVED | SPRINGTRAP_EVENT_DANGER_LOUD;
        event.from_camera = current;
        ai->move_frames = 0u;
        if (current == 0) {
            ai->kind = SPRINGTRAP_LOCATION_OFFICE_LEFT;
        } else {
            ai->kind = SPRINGTRAP_LOCATION_OFFICE_WINDOW;
            event.flags &= ~SPRINGTRAP_EVENT_DANGER_LOUD;
            event.flags |= SPRINGTRAP_EVENT_DANGER_SOFT;
        }
        ai->camera = -1;
        return event;
    }

    return move_to_target(ai, kNeighbours[current][pick], night_number);
}

static SpringtrapEvent movement_opportunity(SpringtrapAI *ai,
                                             int night_number,
                                             int current_hour,
                                             SpringtrapVent sealed_vent)
{
    SpringtrapEvent event = empty_event();
    const SpringtrapVent available_vent = vent_for_camera(ai->camera);
    const bool vent_allowed = current_hour >= 1 &&
        available_vent != SPRINGTRAP_VENT_NONE &&
        available_vent != sealed_vent;

    /* Authentic decision weights supplied for the port:
     * vent 1/3 weight, normal movement 1.5/3 weight, stay 1/3 weight.
     * Normalized, this is 2:3:2. If a vent is unavailable its weight is
     * removed, leaving movement:stay at 3:2. */
    const uint32_t roll = next_random(ai) % (vent_allowed ? 7u : 5u);
    if (vent_allowed && roll < 2u) {
        return enter_vent(ai, available_vent, night_number);
    }

    const uint32_t adjusted = vent_allowed ? roll - 2u : roll;
    if (adjusted < 3u) {
        return move_to_random_neighbour(ai, night_number);
    }

    event.flags = SPRINGTRAP_EVENT_STAYED;
    event.from_camera = ai->camera;
    event.to_camera = ai->camera;
    schedule_next_opportunity(ai, night_number);
    return event;
}

SpringtrapEvent springtrap_ai_update(SpringtrapAI *ai,
                                     int night_number,
                                     int current_hour,
                                     uint32_t movement_opportunity_frames,
                                     SpringtrapVent sealed_vent,
                                     bool directly_observed,
                                     bool player_blinded,
                                     bool panel_open)
{
    SpringtrapEvent event = empty_event();
    (void) movement_opportunity_frames;
    (void) player_blinded;
    if (ai == NULL) return event;

    switch (ai->kind) {
        case SPRINGTRAP_LOCATION_CAMERA:
            if (night_number <= 1) break;
            ++ai->move_frames;
            if (ai->move_frames >= ai->opportunity_frames) {
                event = movement_opportunity(ai, night_number,
                                             current_hour, sealed_vent);
            }
            break;

        case SPRINGTRAP_LOCATION_VENT:
            ++ai->vent_frames;
            if (ai->vent_frames >= VENT_TRAVEL_FRAMES) {
                event = exit_vent(ai, sealed_vent, night_number, panel_open);
            }
            break;

        case SPRINGTRAP_LOCATION_HALL_HIDDEN:
            ++ai->move_frames;
            if (ai->move_frames >= WINDOW_MOVE_CHECK_FRAMES) {
                ai->move_frames = 0u;
                ai->kind = SPRINGTRAP_LOCATION_OFFICE_WINDOW;
                event.flags = SPRINGTRAP_EVENT_MOVED |
                              SPRINGTRAP_EVENT_DANGER_SOFT;
            }
            break;

        case SPRINGTRAP_LOCATION_OFFICE_WINDOW:
            if (directly_observed) {
                ai->move_frames = 0u;
                break;
            }
            ++ai->move_frames;
            if (ai->move_frames >= WINDOW_MOVE_CHECK_FRAMES) {
                ai->move_frames = 0u;
                if ((next_random(ai) & 1u) == 0u) {
                    ai->kind = SPRINGTRAP_LOCATION_OFFICE_LEFT;
                    event.flags = SPRINGTRAP_EVENT_MOVED |
                                  SPRINGTRAP_EVENT_DANGER_LOUD;
                }
            }
            break;

        case SPRINGTRAP_LOCATION_OFFICE_LEFT:
            if (directly_observed) {
                ai->move_frames = 0u;
                break;
            }
            /* Losing sight of him at the door is immediately fatal: looking
             * away, opening either panel, a Phantom jumpscare or a vent blink
             * all release this observation lock. */
            enter_office_inside(ai, false);
            ai->attack_reported = true;
            event.flags = SPRINGTRAP_EVENT_MOVED |
                          SPRINGTRAP_EVENT_DANGER_LOUD |
                          SPRINGTRAP_EVENT_ATTACK;
            break;

        case SPRINGTRAP_LOCATION_OFFICE_INSIDE:
            if (ai->attack_reported) break;

            if (!ai->office_panel_grace) {
                ai->attack_reported = true;
                event.flags = SPRINGTRAP_EVENT_ATTACK;
                break;
            }

            /* Direct vent entry while a panel is already raised gives the
             * player six seconds. Closing it early is immediately fatal; at
             * six seconds Springtrap forces it down and attacks. */
            if (!panel_open) {
                ai->attack_reported = true;
                event.flags = SPRINGTRAP_EVENT_ATTACK;
                break;
            }
            ++ai->office_inside_frames;
            if (ai->office_inside_frames >= OFFICE_PANEL_GRACE_FRAMES) {
                ai->attack_reported = true;
                event.flags = SPRINGTRAP_EVENT_FORCE_CLOSE_PANEL |
                              SPRINGTRAP_EVENT_ATTACK;
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

    /* The lure resistance is one in seven on every active night. */
    if ((next_random(ai) % 7u) == 0u) {
        event.flags = SPRINGTRAP_EVENT_LURE_IGNORED;
        event.from_camera = ai->camera;
        event.to_camera = target_camera;
        return event;
    }

    const int from = ai->camera;
    const SpringtrapCameraPosition direction = position_for_target(from,
                                                                   target_camera);
    ai->camera = target_camera;
    ai->camera_position = direction == SPRINGTRAP_CAMERA_POSITION_EXIT
        ? SPRINGTRAP_CAMERA_POSITION_BACK
        : SPRINGTRAP_CAMERA_POSITION_EXIT;
    schedule_next_opportunity(ai, night_number);

    event.flags = SPRINGTRAP_EVENT_MOVED |
                  event_danger_for_camera(target_camera);
    event.from_camera = from;
    event.to_camera = target_camera;
    return event;
}

void springtrap_ai_release_observation(SpringtrapAI *ai)
{
    if (ai == NULL) return;
    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW) {
        ai->move_frames = WINDOW_MOVE_CHECK_FRAMES;
    } else if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT) {
        /* The next update sees the observation lock released and attacks. */
        ai->move_frames = 0u;
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

SpringtrapCameraPosition springtrap_ai_camera_position(const SpringtrapAI *ai)
{
    return ai != NULL ? ai->camera_position : SPRINGTRAP_CAMERA_POSITION_BACK;
}

SpringtrapVent springtrap_ai_current_vent(const SpringtrapAI *ai)
{
    return ai != NULL ? ai->vent : SPRINGTRAP_VENT_NONE;
}

SpringtrapOfficeSide springtrap_ai_office_side(const SpringtrapAI *ai)
{
    if (ai == NULL) return SPRINGTRAP_OFFICE_NONE;
    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW)
        return SPRINGTRAP_OFFICE_WINDOW;
    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT)
        return SPRINGTRAP_OFFICE_LEFT;
    return SPRINGTRAP_OFFICE_NONE;
}

bool springtrap_ai_is_danger_near(const SpringtrapAI *ai)
{
    if (ai == NULL) return false;
    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW ||
        ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT ||
        ai->kind == SPRINGTRAP_LOCATION_OFFICE_INSIDE ||
        ai->kind == SPRINGTRAP_LOCATION_HALL_HIDDEN) return true;
    return ai->kind == SPRINGTRAP_LOCATION_CAMERA &&
           (ai->camera == 0 || ai->camera == 1 || ai->camera == 4);
}

const char *springtrap_vent_label(SpringtrapVent vent)
{
    static const char *const labels[SPRINGTRAP_VENT_COUNT] = {
        "VENT 11: CAM 09 -> CAM 07",
        "VENT 12: CAM 07 -> CAM 01",
        "VENT 13: CAM 05 -> WINDOW HALL",
        "VENT 14: CAM 10 -> OFFICE",
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
    static const int destinations[SPRINGTRAP_VENT_COUNT] = {6, 0, -1, -1, -1};
    return vent >= 0 && vent < SPRINGTRAP_VENT_COUNT
        ? destinations[vent] : -1;
}
