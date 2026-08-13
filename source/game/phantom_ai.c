/*
 * PC/MFA Phantom runtime.
 *
 * part00 contains the decoded state machine.  The final wrapper below fixes two
 * integration details that the legacy Wii U representation cannot express on
 * its own:
 *   - the gameplay clock stores midnight as 12, while the decoded Phantom core
 *     uses zero as its internal non-1AM..5AM sentinel;
 *   - the PC events clear Chica/Mangle/Puppet when the camera monitor is
 *     LOWERED, not on every frame for which the monitor merely happens to be down.
 */
#define phantoms_update phantoms_update_pre_panel_edge
#define phantoms_on_hour_changed phantoms_on_hour_changed_pre_midnight
#include "phantom_ai_parts/part00.inc"
#undef phantoms_on_hour_changed
#undef phantoms_update

#include "phantom_ai_parts/part01.inc"
#include "phantom_ai_parts/part02.inc"

static PhantomSystem *sPanelHistorySystem = NULL;
static int sPanelHistoryNight = -1;
static bool sPanelWasOpen = false;

static int pc_mfa_phantom_hour(int hour)
{
    /* Game::hour is 12 at midnight.  The decoded Phantom event layer treats
     * midnight as the value outside the 1..5 hourly event range.  Normalizing
     * here keeps the explicit "no random Phantom cycles at 12 AM" rule and
     * prevents the >=4 aggression helper from mistaking 12 AM for late night. */
    return hour == 12 ? 0 : hour;
}

PhantomEvent phantoms_on_hour_changed(PhantomSystem *system, int hour)
{
    return phantoms_on_hour_changed_pre_midnight(
        system, pc_mfa_phantom_hour(hour));
}

PhantomEvent phantoms_update(PhantomSystem *system,
                             int hour,
                             bool camera_open,
                             bool maintenance_open,
                             int selected_camera,
                             int office_pan)
{
    PhantomEvent event = empty_event();
    if (system == NULL) {
        return event;
    }

    /* Reset the edge detector whenever the active Phantom system/night changes. */
    if (sPanelHistorySystem != system || sPanelHistoryNight != system->night) {
        sPanelHistorySystem = system;
        sPanelHistoryNight = system->night;
        sPanelWasOpen = false;
    }

    const bool panel_lowered = sPanelWasOpen && !camera_open;
    sPanelWasOpen = camera_open;
    const int event_hour = pc_mfa_phantom_hour(hour);

    const bool one_second = tick_every_second(system);
    const bool every20 = tick_every_20_seconds(system);
    const bool every60 = tick_every_60_seconds(system);

    if (system->jumpscare != PHANTOM_NONE) {
        return update_jumpscare(system);
    }

    system->aggressive_mode = event_hour >= 4;

    /* Spawn/forced-event conditions still see the REAL monitor state, but use
     * the MFA-normalized hour so 12 AM cannot satisfy late-night/random rules. */
    arm_random_phantoms(system, event_hour, camera_open, selected_camera,
                        every20, every60);
    apply_forced_phantoms(system, event_hour, camera_open, selected_camera);

    PhantomEvent sub = update_freddy(system, camera_open, maintenance_open,
                                     one_second);
    if (sub.flags != PHANTOM_EVENT_NONE) {
        event = sub;
        if ((sub.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
            return event;
        }
    }

    if (system->foxy_present && !camera_open && !maintenance_open &&
        office_pan <= -70 && system->scare_cooldown_seconds == 0u) {
        system->foxy_present = false;
        return start_jumpscare(system, PHANTOM_FOXY);
    }

    sub = update_bb(system, camera_open, selected_camera);
    if ((sub.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
        return sub;
    }
    if (sub.flags != PHANTOM_EVENT_NONE) event = sub;

    /*
     * Chica/Mangle/Puppet camera states survive while the monitor is already
     * down.  On the exact lowering edge they receive camera_open=false and are
     * cleared, matching the Clickteam panel-state event.  While merely closed,
     * feed selected=-1 so exposure counters cannot advance invisibly.
     */
    const bool camera_state_open = camera_open || !panel_lowered;
    const int camera_state_selected = camera_open ? selected_camera : -1;

    sub = update_chica(system, camera_state_open, maintenance_open,
                       camera_state_selected, office_pan);
    if ((sub.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
        return sub;
    }
    if (sub.flags != PHANTOM_EVENT_NONE) event = sub;

    sub = update_mangle(system, camera_state_open, camera_state_selected);
    if (sub.flags != PHANTOM_EVENT_NONE) {
        if ((sub.flags & (PHANTOM_EVENT_AUDIO_FAILURE |
                          PHANTOM_EVENT_GARBLE_STARTED |
                          PHANTOM_EVENT_GARBLE_ENDED)) != 0u) {
            return sub;
        }
        event = sub;
    }

    sub = update_puppet(system, camera_state_open, camera_state_selected);
    if (sub.flags != PHANTOM_EVENT_NONE) {
        if ((sub.flags & (PHANTOM_EVENT_MASK_STARTED |
                          PHANTOM_EVENT_MASK_ENDED)) != 0u) {
            return sub;
        }
        event = sub;
    }

    return event;
}
