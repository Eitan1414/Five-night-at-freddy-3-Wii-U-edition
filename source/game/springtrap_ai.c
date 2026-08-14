/*
 * Final Springtrap runtime.
 *
 * Keep the already-decoded movement/vent/lure graph in one source include, but
 * replace the last compatibility aggression wrapper with the exact event order
 * recovered from fivenights3-94.mfa.  The PC rules are therefore explicit here
 * without duplicating the large route table.
 */
#define springtrap_ai_update springtrap_ai_update_pre_mfa_aggression
#define springtrap_ai_release_observation springtrap_ai_release_observation_pre_mfa_aggression
#define springtrap_ai_lure springtrap_ai_lure_pre_night_gate
#define springtrap_ai_is_on_camera springtrap_ai_is_on_camera_pre_night_gate
#define springtrap_ai_is_in_vent springtrap_ai_is_in_vent_pre_night_gate
#define springtrap_ai_camera springtrap_ai_camera_pre_night_gate
#define springtrap_ai_current_vent springtrap_ai_current_vent_pre_night_gate
#define springtrap_ai_office_side springtrap_ai_office_side_pre_night_gate
#define springtrap_ai_is_danger_near springtrap_ai_is_danger_near_pre_night_gate
#define springtrap_ai_set_runtime_state springtrap_ai_set_runtime_state_pre_mfa_idle
#include "springtrap_ai_base.inc"
#undef springtrap_ai_set_runtime_state
#undef springtrap_ai_is_danger_near
#undef springtrap_ai_office_side
#undef springtrap_ai_current_vent
#undef springtrap_ai_camera
#undef springtrap_ai_is_in_vent
#undef springtrap_ai_is_on_camera
#undef springtrap_ai_lure
#undef springtrap_ai_release_observation
#undef springtrap_ai_update

/*
 * Clickteam Event Editor groups (one-based numbers from the supplied MFA):
 *
 * 145: every 15000 ms -> aggressive? = 0, arm one aggression roll
 * 516: Random(5) < AI -> aggressive? = 1
 * 299: ventilation error state -> aggressive? = 1
 * 607/608: frozen/office inactivity pressure -> aggressive? = 1
 * 744: time of night >= 4 -> aggressive? = 1
 * 746: each 1000 ms without either system screen -> idle counter +1
 * 747: either system screen visible -> idle counter = 0
 * 749: idle counter > 10 -> aggressive? = 1
 * 758: time of night == 12 -> aggressive? = 0
 * 759 is the old demo-only Night-4 override and is intentionally irrelevant to
 * the full retail project.
 *
 * The Wii U runtime has one boolean ventilation-failure state instead of the
 * two Clickteam visual counters used by the source. Phantom release events are
 * represented by springtrap_ai_release_observation(), which is fired by the
 * Phantom state machine when the original event releases Springtrap.
 *
 * Random values use the Wii U port RNG; the probabilities/cadence/state order
 * are MFA-derived, but this does not claim Clickteam's internal RNG sequence.
 */

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

    /* The MFA counter is not player-input inactivity. Groups 746/747 count
     * whole gameplay frames spent with neither system screen open, regardless
     * of office panning or button presses. Keep the old parameter for API
     * compatibility but drive the source-faithful counter locally. */
    (void) office_idle_frames;
    if (camera_open || maintenance_open) {
        ai->office_idle_frames = 0u;
    } else if (ai->office_idle_frames < UINT32_MAX) {
        ++ai->office_idle_frames;
    }
}

static void mfa_exact_update_aggression(SpringtrapAI *ai, int current_hour)
{
    if (++ai->aggressive_refresh_frames >= AGGRESSIVE_REFRESH_FRAMES) {
        ai->aggressive_refresh_frames = 0u;

        /* Groups 145 -> 516 run in this order: clear first, then the single
         * five-way AI roll may set the counter back to one. */
        ai->aggressive = false;
        if ((next_random(ai) % 5u) < (uint32_t) ai->ai_level) {
            ai->aggressive = true;
        }
    }

    /* Group 299: a ventilation error forces aggression. */
    if (ai->ventilation_failed) {
        ai->aggressive = true;
    }

    /* Groups 746/749: the alterable counter gains one each second and the
     * source tests strictly > 10. The first forced-aggression tick is therefore
     * at eleven seconds with neither camera nor maintenance screen visible. */
    if (ai->office_idle_frames >= 11u * FRAMES_PER_SECOND) {
        ai->aggressive = true;
    }

    /* Group 744. */
    if (current_hour >= 4) {
        ai->aggressive = true;
    }

    /* Group 758 appears later in the Event Editor than the forcing groups, so
     * midnight wins. The legacy Wii U caller normalizes Game::hour 12 to zero,
     * while direct/test callers may pass the stored value 12; accept both. */
    if (current_hour == 0 || current_hour == 12) {
        ai->aggressive = false;
    }
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

    /* Springtrap does not exist in the playable Night 1 frame.  The base reset
     * still seeds a camera for Nights 2+, but no Night-1 caller may advance,
     * expose or otherwise observe that latent state. */
    if (night_number <= 1) {
        ai->night = night_number;
        ai->ai_level = 0;
        return event;
    }

    /* These two parameters are retained for ABI/source compatibility with the
     * older Wii U caller. Movement is now wholly driven by the MFA one-second
     * counter, and direct office observation is encoded by the attack events. */
    (void) movement_opportunity_frames;
    (void) directly_observed;

    if (night_number != ai->night) {
        ai->night = night_number;
        ai->ai_level = mfa_ai_level(night_number);
    }

    update_random_action_branch(ai);
    mfa_exact_update_aggression(ai, current_hour);

    /* MFA groups 731-761: Phantom-triggered force_move is processed before the
     * normal move-counter opportunity. */
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

    /* Groups 146/147 increment move counter by one or two each second, group
     * 148 tests 10 - AI - aggressive + Random(15) - total_turns. */
    if (!tick_one_second(ai) || !movement_opportunity(ai)) {
        return event;
    }

    /* The source clears move counter when the opportunity fires, even when
     * action 1 later elects to stay and increase total_turns. */
    ai->move_counter = 0u;
    const int action = select_action(ai);

    if (ai->kind == SPRINGTRAP_LOCATION_VENT) {
        if (action == 1) {
            return stayed(ai);
        }
        return exit_or_finish_vent(ai, sealed_vent);
    }

    if (ai->kind == SPRINGTRAP_LOCATION_CAMERA) {
        if (ai->camera == CAM01 && player_blinded && action > 1) {
            return move_to_attack_stage(ai, SPRINGTRAP_LOCATION_OFFICE_LEFT);
        }
        return apply_camera_action(ai, action);
    }

    return apply_attack_action(ai, action, player_blinded);
}

SpringtrapEvent springtrap_ai_lure(SpringtrapAI *ai,
                                   int target_camera,
                                   int night_number)
{
    if (ai == NULL || night_number <= 1 || ai->night <= 1) {
        SpringtrapEvent event = empty_event();
        event.flags = SPRINGTRAP_EVENT_LURE_INVALID;
        event.to_camera = target_camera;
        return event;
    }
    return springtrap_ai_lure_pre_night_gate(ai, target_camera, night_number);
}

void springtrap_ai_release_observation(SpringtrapAI *ai)
{
    if (ai == NULL || ai->night <= 1) {
        return;
    }

    /* MFA group 731: a Phantom/frozen event on Night 3+ arms force_move and
     * chooses force_to = Random(3)+1. Groups 607/608 make Springtrap aggressive.
     * Importantly this does NOT restart the independent 15-second aggression
     * clock; the old compatibility implementation did, changing the PC timing. */
    if (ai->night >= 3) {
        ai->force_move_pending = true;
        ai->force_to = (uint8_t) (1u + (next_random(ai) % 3u));
    }
    ai->aggressive = true;
}

bool springtrap_ai_is_on_camera(const SpringtrapAI *ai, int camera)
{
    return ai != NULL && ai->night >= 2 &&
           springtrap_ai_is_on_camera_pre_night_gate(ai, camera);
}

bool springtrap_ai_is_in_vent(const SpringtrapAI *ai)
{
    return ai != NULL && ai->night >= 2 &&
           springtrap_ai_is_in_vent_pre_night_gate(ai);
}

int springtrap_ai_camera(const SpringtrapAI *ai)
{
    return ai != NULL && ai->night >= 2
        ? springtrap_ai_camera_pre_night_gate(ai) : -1;
}

SpringtrapVent springtrap_ai_current_vent(const SpringtrapAI *ai)
{
    return ai != NULL && ai->night >= 2
        ? springtrap_ai_current_vent_pre_night_gate(ai)
        : SPRINGTRAP_VENT_NONE;
}

SpringtrapOfficeSide springtrap_ai_office_side(const SpringtrapAI *ai)
{
    return ai != NULL && ai->night >= 2
        ? springtrap_ai_office_side_pre_night_gate(ai)
        : SPRINGTRAP_OFFICE_NONE;
}

bool springtrap_ai_is_danger_near(const SpringtrapAI *ai)
{
    return ai != NULL && ai->night >= 2 &&
           springtrap_ai_is_danger_near_pre_night_gate(ai);
}
