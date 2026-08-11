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
#include "springtrap_ai_base.inc"
#undef springtrap_ai_release_observation
#undef springtrap_ai_update

/*
 * Clickteam Event Editor groups (one-based numbers from the supplied MFA):
 *
 * 145: every 15000 ms -> aggressive? = 0, arm one aggression roll
 * 516: Random(5) < AI -> aggressive? = 1
 * 299: ventilation error state -> aggressive? = 1
 * 607: frozen == 1 -> aggressive? = 1
 * 608: office shake/after-effect active -> aggressive? = 1
 * 744: time of night >= 4 -> aggressive? = 1
 * 749: advanced ventilation-error state -> aggressive? = 1
 * 758: time of night == 12 -> aggressive? = 0
 * 759 is the old demo-only Night-4 override and is intentionally irrelevant to
 * the full retail project.
 *
 * The Wii U runtime has one boolean ventilation-failure state instead of the
 * two Clickteam visual counters used by groups 299/749, so both source events
 * collapse to the same exact gameplay condition here.  Phantom group 607 is
 * represented by springtrap_ai_release_observation(), which is fired by the
 * Phantom state machine when the original event releases Springtrap.
 *
 * Random values use the Wii U port RNG; the probabilities/cadence/state order
 * are MFA-derived, but this does not claim Clickteam's internal RNG sequence.
 */
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

    /* Groups 299 and 749 are two visual-counter representations of the same
     * ventilation failure in the original frame. */
    if (ai->ventilation_failed) {
        ai->aggressive = true;
    }

    /* Group 744. */
    if (current_hour >= 4) {
        ai->aggressive = true;
    }

    /* Group 758 appears later in the Event Editor than the forcing groups, so
     * 12 AM wins for the retail game. The Wii U caller represents 12 as zero. */
    if (current_hour == 0) {
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

void springtrap_ai_release_observation(SpringtrapAI *ai)
{
    if (ai == NULL) {
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
