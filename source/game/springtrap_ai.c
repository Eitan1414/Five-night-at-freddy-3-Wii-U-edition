#include "game/springtrap_ai.h"

#include <stddef.h>
#include <string.h>

/*
 * Springtrap state machine reconstructed from the PC Clickteam event logic.
 *
 * Core rules reproduced here:
 *   move_counter > 10 - AI - aggressive + Random(15) - total_turns
 *   action_selected = Random(3 + aggressive) + 1
 *   action 1 stalls and increments total_turns; a real move resets it
 *   aggression is re-rolled every 15 seconds and is also forced by the
 *   ventilation error, 10 seconds of office inactivity and 4 AM+
 *   Phantom encounters arm force_move / force_to from Night 3 onward
 *   audio lures use the original hard-coded one-way hearing table and 6/7 roll
 */
#define FRAMES_PER_SECOND 60u
#define AGGRESSIVE_REFRESH_FRAMES (15u * FRAMES_PER_SECOND)
#define RANDOM_ACTION_REFRESH_FRAMES (10u * FRAMES_PER_SECOND)
#define OFFICE_IDLE_AGGRESSIVE_FRAMES (10u * FRAMES_PER_SECOND)

#define CAM01 0
#define CAM02 1
#define CAM03 2
#define CAM04 3
#define CAM05 4
#define CAM06 5
#define CAM07 6
#define CAM08 7
#define CAM09 8
#define CAM10 9

static uint32_t next_random(SpringtrapAI *ai)
{
    ai->rng = ai->rng * 1664525u + 1013904223u;
    return ai->rng;
}

static int mfa_ai_level(int night)
{
    if (night <= 1) return 0;
    if (night == 2) return 2;
    if (night == 3) return 3;
    if (night == 4) return 4;
    if (night == 5) return 5;
    return 7;
}

/* The old Wii U caller still passes its former fixed movement interval.  It is
 * no longer used as the movement clock, but a reduced value is kept as a
 * compatibility signal for environmental aggression while the rest of the
 * port transitions to the MFA counters. */
static uint32_t legacy_base_interval(int night)
{
    if (night <= 2) return 360u;
    if (night == 3) return 300u;
    if (night == 4) return 240u;
    if (night == 5) return 180u;
    return 120u;
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

static uint32_t danger_for_camera(int camera)
{
    if (camera == CAM03 || camera == CAM05) {
        return SPRINGTRAP_EVENT_DANGER_SOFT;
    }
    if (camera == CAM01 || camera == CAM02) {
        return SPRINGTRAP_EVENT_DANGER_LOUD;
    }
    return SPRINGTRAP_EVENT_NONE;
}

static SpringtrapVent vent_for_camera(int camera)
{
    switch (camera) {
        case CAM09: return SPRINGTRAP_VENT_11;
        case CAM07: return SPRINGTRAP_VENT_12;
        case CAM05: return SPRINGTRAP_VENT_13;
        case CAM10: return SPRINGTRAP_VENT_14;
        case CAM02: return SPRINGTRAP_VENT_15;
        default: return SPRINGTRAP_VENT_NONE;
    }
}

static void reset_after_real_move(SpringtrapAI *ai)
{
    ai->move_counter = 0u;
    ai->total_turns = 0u;
}

static SpringtrapEvent stayed(SpringtrapAI *ai)
{
    SpringtrapEvent event = empty_event();
    event.flags = SPRINGTRAP_EVENT_STAYED;
    if (ai->kind == SPRINGTRAP_LOCATION_CAMERA) {
        event.from_camera = ai->camera;
        event.to_camera = ai->camera;
    }
    if (ai->total_turns < 31u) {
        ++ai->total_turns;
    }
    return event;
}

static SpringtrapEvent move_to_camera(SpringtrapAI *ai, int target)
{
    SpringtrapEvent event = empty_event();
    event.flags = SPRINGTRAP_EVENT_MOVED | danger_for_camera(target);
    event.from_camera = ai->kind == SPRINGTRAP_LOCATION_CAMERA ? ai->camera : -1;
    event.to_camera = target;
    ai->kind = SPRINGTRAP_LOCATION_CAMERA;
    ai->camera = target;
    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->vent_source_camera = -1;
    reset_after_real_move(ai);
    return event;
}

static SpringtrapEvent move_to_attack_stage(SpringtrapAI *ai,
                                             SpringtrapLocationKind target)
{
    SpringtrapEvent event = empty_event();
    event.flags = SPRINGTRAP_EVENT_MOVED;
    event.from_camera = ai->kind == SPRINGTRAP_LOCATION_CAMERA ? ai->camera : -1;
    ai->kind = target;
    ai->camera = -1;
    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->vent_source_camera = -1;
    reset_after_real_move(ai);

    if (target == SPRINGTRAP_LOCATION_OFFICE_WINDOW) {
        event.flags |= SPRINGTRAP_EVENT_DANGER_SOFT;
    } else if (target == SPRINGTRAP_LOCATION_HALL_HIDDEN ||
               target == SPRINGTRAP_LOCATION_OFFICE_LEFT ||
               target == SPRINGTRAP_LOCATION_OFFICE_INSIDE) {
        event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD;
    }
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
    ai->camera = -1;
    reset_after_real_move(ai);
    return event;
}

static SpringtrapEvent exit_or_finish_vent(SpringtrapAI *ai,
                                            SpringtrapVent sealed_vent)
{
    SpringtrapEvent event = empty_event();
    const SpringtrapVent vent = ai->vent;
    const int source = ai->vent_source_camera;
    event.flags = SPRINGTRAP_EVENT_VENT_EXIT | SPRINGTRAP_EVENT_MOVED;
    event.from_camera = source;
    event.vent = vent;

    if (sealed_vent == vent) {
        ai->kind = SPRINGTRAP_LOCATION_CAMERA;
        ai->camera = source;
        event.to_camera = source;
        event.flags |= danger_for_camera(source);
        ai->vent = SPRINGTRAP_VENT_NONE;
        ai->vent_source_camera = -1;
        reset_after_real_move(ai);
        return event;
    }

    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->vent_source_camera = -1;
    ai->camera = -1;
    reset_after_real_move(ai);

    switch (vent) {
        case SPRINGTRAP_VENT_11:
        case SPRINGTRAP_VENT_12:
            ai->kind = SPRINGTRAP_LOCATION_HALL_HIDDEN;
            event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD;
            break;
        case SPRINGTRAP_VENT_13:
            ai->kind = SPRINGTRAP_LOCATION_OFFICE_WINDOW;
            event.flags |= SPRINGTRAP_EVENT_DANGER_SOFT;
            break;
        case SPRINGTRAP_VENT_14:
        case SPRINGTRAP_VENT_15:
            ai->kind = SPRINGTRAP_LOCATION_OFFICE_INSIDE;
            ai->attack_reported = true;
            event.flags |= SPRINGTRAP_EVENT_DANGER_LOUD |
                           SPRINGTRAP_EVENT_ATTACK;
            break;
        default:
            ai->kind = SPRINGTRAP_LOCATION_CAMERA;
            ai->camera = source;
            event.to_camera = source;
            break;
    }
    return event;
}

static bool can_hear_lure_from_camera(int current_camera, int target_camera)
{
    switch (target_camera) {
        case CAM02:
            return current_camera == CAM03 || current_camera == CAM04 ||
                   current_camera == CAM05;
        case CAM03:
            return current_camera == CAM02 || current_camera == CAM04;
        case CAM04:
            return current_camera == CAM02 || current_camera == CAM03;
        case CAM05:
            return current_camera == CAM02 || current_camera == CAM06 ||
                   current_camera == CAM07 || current_camera == CAM08;
        case CAM06:
            return current_camera == CAM05 || current_camera == CAM07;
        case CAM07:
            return current_camera == CAM06 || current_camera == CAM08;
        case CAM08:
            return current_camera == CAM05 || current_camera == CAM07 ||
                   current_camera == CAM09;
        case CAM09:
            return current_camera == CAM08 || current_camera == CAM10;
        case CAM10:
            return current_camera == CAM09;
        default:
            return false;
    }
}

bool springtrap_ai_cameras_adjacent(int first, int second)
{
    if (first < 0 || first >= SPRINGTRAP_CAMERA_COUNT ||
        second < 0 || second >= SPRINGTRAP_CAMERA_COUNT) {
        return false;
    }
    return can_hear_lure_from_camera(first, second);
}

static bool camera_is_being_watched(const SpringtrapAI *ai)
{
    return ai->kind == SPRINGTRAP_LOCATION_CAMERA &&
           ai->camera_screen_open &&
           ai->selected_camera == ai->camera;
}

static void update_random_action_branch(SpringtrapAI *ai)
{
    if (++ai->random_action_frames < RANDOM_ACTION_REFRESH_FRAMES) {
        return;
    }
    ai->random_action_frames = 0u;
    if (!camera_is_being_watched(ai)) {
        ai->random_action_choice = (uint8_t) (next_random(ai) & 1u);
    }
}

static void update_aggression(SpringtrapAI *ai,
                              int current_hour,
                              uint32_t legacy_interval)
{
    if (++ai->aggressive_refresh_frames >= AGGRESSIVE_REFRESH_FRAMES) {
        ai->aggressive_refresh_frames = 0u;
        ai->aggressive = (next_random(ai) % 5u) < (uint32_t) ai->ai_level;
    }

    if (ai->ventilation_failed ||
        ai->office_idle_frames >= OFFICE_IDLE_AGGRESSIVE_FRAMES ||
        current_hour >= 4 ||
        legacy_interval < legacy_base_interval(ai->night)) {
        ai->aggressive = true;
    }
}

static bool tick_one_second(SpringtrapAI *ai)
{
    if (++ai->one_second_frames < FRAMES_PER_SECOND) {
        return false;
    }
    ai->one_second_frames = 0u;
    ai->move_counter += ai->aggressive ? 2u : 1u;
    return true;
}

static bool movement_opportunity(SpringtrapAI *ai)
{
    const int random_term = (int) (next_random(ai) % 15u);
    const int threshold = 10 - ai->ai_level - (ai->aggressive ? 1 : 0) +
                          random_term - (int) ai->total_turns;
    return (int) ai->move_counter > threshold;
}

static int select_action(SpringtrapAI *ai)
{
    const uint32_t range = ai->aggressive ? 4u : 3u;
    return 1 + (int) (next_random(ai) % range);
}

static SpringtrapEvent apply_camera_action(SpringtrapAI *ai, int action)
{
    const int camera = ai->camera;
    if (action == 1) {
        return stayed(ai);
    }

    switch (camera) {
        case CAM10:
            if (action == 4) return enter_vent(ai, SPRINGTRAP_VENT_14);
            return move_to_camera(ai, CAM09);

        case CAM09:
            if (action == 2) return move_to_camera(ai, CAM10);
            if (action == 3) return move_to_camera(ai, CAM08);
            return enter_vent(ai, SPRINGTRAP_VENT_11);

        case CAM08:
            if (action == 2) return move_to_camera(ai, CAM09);
            if (action == 3) return move_to_camera(ai, CAM07);
            return move_to_camera(ai, CAM05);

        case CAM07:
            if (action == 2) return move_to_camera(ai, CAM08);
            if (action == 3) return move_to_camera(ai, CAM06);
            return enter_vent(ai, SPRINGTRAP_VENT_12);

        case CAM06:
            if (action == 2) return move_to_camera(ai, CAM07);
            return move_to_camera(ai, CAM05);

        case CAM05:
            if (action == 2) return move_to_camera(ai, CAM06);
            if (action == 3) return move_to_camera(ai, CAM02);
            if (ai->random_action_choice == 0u)
                return move_to_camera(ai, CAM04);
            return enter_vent(ai, SPRINGTRAP_VENT_13);

        case CAM04:
            if (action == 2) return move_to_camera(ai, CAM02);
            return move_to_camera(ai, CAM03);

        case CAM03:
            if (action == 2) return move_to_camera(ai, CAM04);
            return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_WINDOW);

        case CAM02:
            if (action == 2) return move_to_camera(ai, CAM05);
            if (action == 3) return move_to_camera(ai, CAM04);
            if (ai->random_action_choice == 0u)
                return enter_vent(ai, SPRINGTRAP_VENT_15);
            return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_WINDOW);

        case CAM01:
            /* CAM01 belongs to the attack chain: action 2 backs into stage 3,
             * action >2 advances to stage 4. */
            if (!(ai->camera_screen_open || ai->maintenance_screen_open)) {
                return stayed(ai);
            }
            if (action == 2)
                return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_HALL_HIDDEN);
            return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_LEFT);

        default:
            return stayed(ai);
    }
}

static SpringtrapEvent apply_attack_action(SpringtrapAI *ai,
                                            int action,
                                            bool player_blinded)
{
    const bool system_screen_or_blackout =
        ai->camera_screen_open || ai->maintenance_screen_open || player_blinded;

    if (action == 1) {
        return stayed(ai);
    }

    switch (ai->kind) {
        case SPRINGTRAP_LOCATION_OFFICE_WINDOW:
            if (action > 2 && system_screen_or_blackout)
                return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_HALL_RUN);
            return stayed(ai);

        case SPRINGTRAP_LOCATION_HALL_RUN:
            /* The stage-2 -> stage-3 transition is the one attack transition
             * that does not require a system screen or blackout. */
            if (action > 2)
                return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_HALL_HIDDEN);
            return stayed(ai);

        case SPRINGTRAP_LOCATION_HALL_HIDDEN:
            if (!system_screen_or_blackout)
                return stayed(ai);
            if (action == 2)
                return move_to_camera(ai, CAM01);
            return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_LEFT);

        case SPRINGTRAP_LOCATION_OFFICE_LEFT:
            if (action > 1 && system_screen_or_blackout) {
                SpringtrapEvent event =
                    move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_INSIDE);
                ai->attack_reported = true;
                event.flags |= SPRINGTRAP_EVENT_ATTACK |
                               SPRINGTRAP_EVENT_DANGER_LOUD;
                return event;
            }
            return stayed(ai);

        default:
            return stayed(ai);
    }
}

static bool forced_move_eligible(const SpringtrapAI *ai)
{
    if (!ai->force_move_pending || ai->kind != SPRINGTRAP_LOCATION_CAMERA) {
        return false;
    }
    /* MFA postpones the teleport while Springtrap is in CAM03, CAM04 or a
     * vent. Vents are already excluded by the location-kind check. */
    return ai->camera != CAM03 && ai->camera != CAM04;
}

static SpringtrapEvent apply_forced_move(SpringtrapAI *ai)
{
    SpringtrapEvent event = empty_event();
    if (!forced_move_eligible(ai)) {
        return event;
    }

    const int from = ai->camera;
    int target = -1;
    bool target_is_stage1 = false;

    if (from >= CAM06 && from <= CAM10) {
        if (ai->force_to == 1u) target = CAM05;
        else if (ai->force_to == 2u) target = CAM04;
        else target = CAM10;
    } else if (from == CAM05) {
        target = CAM02;
    } else if (from == CAM02) {
        target_is_stage1 = true;
    } else {
        /* CAM01 is already in the attack chain. Keep force_move armed until a
         * later eligible camera state, mirroring the source postponement. */
        return event;
    }

    ai->force_move_pending = false;
    ai->total_turns = 0u;

    if (target_is_stage1) {
        event = move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_WINDOW);
        /* The source only explicitly zeroes move_counter when force_move lands
         * on CAM04 or CAM10. Restore the previous counter for this route. */
        return event;
    }

    const uint32_t previous_counter = ai->move_counter;
    event = move_to_camera(ai, target);
    if (target != CAM04 && target != CAM10) {
        ai->move_counter = previous_counter;
    }
    return event;
}

void springtrap_ai_reset(SpringtrapAI *ai, int night_number, uint32_t seed)
{
    if (ai == NULL) {
        return;
    }

    memset(ai, 0, sizeof(*ai));
    ai->night = night_number;
    ai->ai_level = mfa_ai_level(night_number);
    ai->rng = seed ^ (0x5F3759DFu + (uint32_t) night_number * 0x1021u);
    ai->kind = SPRINGTRAP_LOCATION_CAMERA;
    /* Random(5)+1: 1=CAM10, 2=CAM09, 3=CAM08, 4=CAM07, 5=CAM06. */
    ai->camera = CAM10 - (int) (next_random(ai) % 5u);
    ai->vent_source_camera = -1;
    ai->vent = SPRINGTRAP_VENT_NONE;
    ai->selected_camera = -1;
    ai->random_action_choice = (uint8_t) (next_random(ai) & 1u);
    ai->force_to = 1u;
    ai->attack_reported = false;
}

void springtrap_ai_set_runtime_state(SpringtrapAI *ai,
                                     bool camera_open,
                                     bool maintenance_open,
                                     int selected_camera,
                                     bool ventilation_failed,
                                     uint32_t office_idle_frames)
{
    if (ai == NULL) {
        return;
    }
    ai->camera_screen_open = camera_open;
    ai->maintenance_screen_open = maintenance_open;
    ai->selected_camera = selected_camera;
    ai->ventilation_failed = ventilation_failed;
    ai->office_idle_frames = office_idle_frames;
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

    (void) directly_observed;

    if (night_number != ai->night) {
        ai->night = night_number;
        ai->ai_level = mfa_ai_level(night_number);
    }

    update_random_action_branch(ai);
    update_aggression(ai, current_hour, movement_opportunity_frames);

    event = apply_forced_move(ai);
    if (event.flags != SPRINGTRAP_EVENT_NONE) {
        return event;
    }

    if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_INSIDE) {
        if (!ai->attack_reported) {
            ai->attack_reported = true;
            event.flags = SPRINGTRAP_EVENT_ATTACK;
        }
        return event;
    }

    if (!tick_one_second(ai) || !movement_opportunity(ai)) {
        return event;
    }

    /* Every opportunity resets move_counter, even when action 1 stalls. */
    ai->move_counter = 0u;
    const int action = select_action(ai);

    if (ai->kind == SPRINGTRAP_LOCATION_VENT) {
        if (action == 1) {
            return stayed(ai);
        }
        return exit_or_finish_vent(ai, sealed_vent);
    }

    if (ai->kind == SPRINGTRAP_LOCATION_CAMERA) {
        /* CAM01 can advance to stage 4 during a ventilation blackout even if
         * action_selected would otherwise be blocked by the closed screens. */
        if (ai->camera == CAM01 && player_blinded && action > 1)
            return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_LEFT);
        return apply_camera_action(ai, action);
    }

    return apply_attack_action(ai, action, player_blinded);
}

SpringtrapEvent springtrap_ai_lure(SpringtrapAI *ai,
                                   int target_camera,
                                   int night_number)
{
    SpringtrapEvent event = empty_event();
    if (ai == NULL || target_camera < 0 ||
        target_camera >= SPRINGTRAP_CAMERA_COUNT) {
        event.flags = SPRINGTRAP_EVENT_LURE_INVALID;
        return event;
    }

    (void) night_number;

    bool valid = false;
    if (ai->kind == SPRINGTRAP_LOCATION_CAMERA) {
        valid = can_hear_lure_from_camera(ai->camera, target_camera);
    } else if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW) {
        valid = target_camera == CAM02;
    } else if (ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT) {
        valid = target_camera == CAM01;
    }

    if (!valid) {
        event.flags = SPRINGTRAP_EVENT_LURE_INVALID;
        event.from_camera = ai->kind == SPRINGTRAP_LOCATION_CAMERA
            ? ai->camera : -1;
        event.to_camera = target_camera;
        return event;
    }

    /* Original audio lure: six chances out of seven to succeed. */
    if ((next_random(ai) % 7u) == 0u) {
        event.flags = SPRINGTRAP_EVENT_LURE_IGNORED;
        event.from_camera = ai->kind == SPRINGTRAP_LOCATION_CAMERA
            ? ai->camera : -1;
        event.to_camera = target_camera;
        return event;
    }

    event = move_to_camera(ai, target_camera);
    event.to_camera = target_camera;
    return event;
}

void springtrap_ai_release_observation(SpringtrapAI *ai)
{
    if (ai == NULL) {
        return;
    }

    /* PC force_move is armed by Phantom encounters from Night 3 onward.  The
     * existing game integration already calls this hook for those encounters,
     * so preserve the API while restoring the source behavior. */
    if (ai->night >= 3) {
        ai->force_move_pending = true;
        ai->force_to = (uint8_t) (1u + (next_random(ai) % 3u));
    }
    ai->aggressive = true;
    ai->aggressive_refresh_frames = 0u;
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
        ai->kind == SPRINGTRAP_LOCATION_HALL_RUN ||
        ai->kind == SPRINGTRAP_LOCATION_HALL_HIDDEN ||
        ai->kind == SPRINGTRAP_LOCATION_OFFICE_LEFT ||
        ai->kind == SPRINGTRAP_LOCATION_OFFICE_INSIDE) {
        return true;
    }
    return ai->kind == SPRINGTRAP_LOCATION_CAMERA &&
           (ai->camera == CAM01 || ai->camera == CAM02 ||
            ai->camera == CAM03 || ai->camera == CAM05);
}

const char *springtrap_vent_label(SpringtrapVent vent)
{
    static const char *const labels[SPRINGTRAP_VENT_COUNT] = {
        "VENT 11: CAM 09 -> HALL",
        "VENT 12: CAM 07 -> HALL",
        "VENT 13: CAM 05 -> WINDOW",
        "VENT 14: CAM 10 -> OFFICE",
        "VENT 15: CAM 02 -> OFFICE",
    };
    return vent >= 0 && vent < SPRINGTRAP_VENT_COUNT
        ? labels[vent] : "NO VENT";
}

int springtrap_vent_source_camera(SpringtrapVent vent)
{
    static const int sources[SPRINGTRAP_VENT_COUNT] = {
        CAM09, CAM07, CAM05, CAM10, CAM02
    };
    return vent >= 0 && vent < SPRINGTRAP_VENT_COUNT ? sources[vent] : -1;
}

int springtrap_vent_destination_camera(SpringtrapVent vent)
{
    /* All five PC vents leave free-roaming camera space: 11/12 go to attack
     * stage 3, 13 to stage 1, and 14/15 directly to the office. */
    (void) vent;
    return -1;
}
