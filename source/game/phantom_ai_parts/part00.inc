#include "game/phantom_ai.h"

#include <stddef.h>
#include <string.h>

/*
 * PC/MFA Phantom state machine.
 *
 * The constants below are decoded from fivenights3-94.mfa:
 *   - Event groups 502-507: AI level by night.
 *   - 630-635: "time limit" camera exposure counter by night.
 *   - 521-530: random/forced Phantom arming.
 *   - 531-600: BB/Freddy/Mangle/Puppet/Chica state transitions.
 *   - 640: ten-second shared scare cooldown.
 *   - 721-725: Phantom Foxy monitor roll and office collision scare.
 */
#define FRAMES_PER_SECOND 60u
#define EVERY_20_SECONDS (20u * FRAMES_PER_SECOND)
#define EVERY_60_SECONDS (60u * FRAMES_PER_SECOND)
#define SCARE_COOLDOWN_SECONDS 10u

#define CAM01_INDEX 0
#define CAM04_INDEX 3
#define CAM07_INDEX 6
#define CAM08_INDEX 7
#define CAM09_INDEX 8
#define CAM10_INDEX 9

/* MFA object "fwalk": one path step, length 1061, speed 6.
 * The Wii U port keeps the path accumulator in eighth-pixel units so the
 * decoded MFA length/speed ratio is preserved without rounding each frame. */
#define FREDDY_PATH_LENGTH_EIGHTHS (1061u * 8u)
#define FREDDY_PATH_SPEED_EIGHTHS 6u
#define FREDDY_DUCK_FRAMES (11u * 4u)

/* MFA "mangle 2" path after Set Speed 5:
 * 100 units @5, then 101 units @50, followed by a 500ms pause.  The same
 * eighth-unit accumulator used above keeps the two decoded path segments
 * proportional on the Wii U runtime. */
#define MANGLE_PATH_FRAMES \
    (((100u * 8u + 4u) / 5u) + ((101u * 8u + 49u) / 50u) + \
     (FRAMES_PER_SECOND / 2u))

/* Phantom Head alterable value B is set to 1000, then subtracted every tick. */
#define PUPPET_MASK_FRAMES 1000u

static uint32_t next_random(PhantomSystem *system)
{
    system->rng = system->rng * 1664525u + 1013904223u;
    return system->rng;
}

static PhantomEvent empty_event(void)
{
    PhantomEvent event = {
        .flags = PHANTOM_EVENT_NONE,
        .phantom = PHANTOM_NONE,
    };
    return event;
}

static bool attacked(const PhantomSystem *system, PhantomId phantom)
{
    return phantom >= 0 && phantom < PHANTOM_COUNT &&
           (system->attacked_mask & (1u << (uint32_t) phantom)) != 0u;
}

static void mark_attacked(PhantomSystem *system, PhantomId phantom)
{
    if (phantom >= 0 && phantom < PHANTOM_COUNT) {
        system->attacked_mask |= 1u << (uint32_t) phantom;
    }
}

static int mfa_ai_level(int night)
{
    if (night <= 2) return 2;
    if (night == 3) return 3;
    if (night == 4) return 4;
    if (night == 5) return 5;
    return 7;
}

static uint32_t mfa_time_limit(int night)
{
    if (night <= 1) return 100u;
    if (night == 2) return 90u;
    if (night == 3) return 80u;
    if (night == 4) return 70u;
    if (night == 5) return 60u;
    return 50u;
}

static bool ai_roll(PhantomSystem *system, uint32_t range)
{
    /* MFA: Random(range) + 1 <= AI. */
    return (next_random(system) % range) + 1u <= (uint32_t) system->ai_level;
}

static bool bb_camera_is_valid(int camera)
{
    return camera == CAM01_INDEX || camera == CAM07_INDEX ||
           camera == CAM09_INDEX || camera == CAM10_INDEX;
}

static bool camera_is_selected(bool camera_open, int selected, int target)
{
    return camera_open && selected == target;
}

static bool bb_available(const PhantomSystem *system)
{
    return system->night >= 2 && !attacked(system, PHANTOM_BALLOON_BOY);
}

static bool mangle_available(const PhantomSystem *system)
{
    return system->night >= 2 && !attacked(system, PHANTOM_MANGLE);
}

static bool freddy_available(const PhantomSystem *system)
{
    return system->night >= 3 && !attacked(system, PHANTOM_FREDDY);
}

static bool chica_available(const PhantomSystem *system)
{
    return system->night >= 3 && !attacked(system, PHANTOM_CHICA);
}

static bool puppet_available(const PhantomSystem *system)
{
    return system->night >= 4 && !attacked(system, PHANTOM_PUPPET);
}

static PhantomEvent start_jumpscare(PhantomSystem *system, PhantomId phantom)
{
    PhantomEvent event = empty_event();
    if (attacked(system, phantom) || system->jumpscare != PHANTOM_NONE) {
        return event;
    }

    mark_attacked(system, phantom);
    system->jumpscare = phantom;
    system->jumpscare_frames = 0u;
    /* BB sets the shared cooldown when its scare animation finishes (MFA
     * group 565). Foxy/Chica/Freddy set it as their attack begins. */
    if (phantom != PHANTOM_BALLOON_BOY) {
        system->scare_cooldown_seconds = SCARE_COOLDOWN_SECONDS;
    }

    event.flags = PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                  PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                  PHANTOM_EVENT_JUMPSCARE_STARTED |
                  PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = phantom;
    return event;
}

static uint32_t jumpscare_duration_frames(PhantomId phantom)
{
    switch (phantom) {
        case PHANTOM_FOXY: return 13u * 4u;
        case PHANTOM_BALLOON_BOY: return 11u * 4u;
        case PHANTOM_FREDDY: return 21u * 4u;
        case PHANTOM_CHICA: return 15u * 4u;
        default: return 90u;
    }
}

static PhantomEvent update_jumpscare(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (system->jumpscare == PHANTOM_NONE) {
        return event;
    }

    ++system->jumpscare_frames;
    if (system->jumpscare_frames >=
        jumpscare_duration_frames(system->jumpscare)) {
        const PhantomId finished = system->jumpscare;
        system->jumpscare = PHANTOM_NONE;
        system->jumpscare_frames = 0u;
        if (finished == PHANTOM_BALLOON_BOY) {
            system->scare_cooldown_seconds = SCARE_COOLDOWN_SECONDS;
        }
        event.flags = PHANTOM_EVENT_VENTILATION_FAILURE |
                      PHANTOM_EVENT_JUMPSCARE_ENDED |
                      PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = finished;
    } else {
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = system->jumpscare;
    }
    return event;
}

static bool tick_every_second(PhantomSystem *system)
{
    if (++system->one_second_frames < FRAMES_PER_SECOND) {
        return false;
    }
    system->one_second_frames = 0u;
    if (system->scare_cooldown_seconds > 0u) {
        --system->scare_cooldown_seconds;
    }
    return true;
}

static bool tick_every_20_seconds(PhantomSystem *system)
{
    if (++system->twenty_second_frames < EVERY_20_SECONDS) {
        return false;
    }
    system->twenty_second_frames = 0u;
    return true;
}

static bool tick_every_60_seconds(PhantomSystem *system)
{
    if (++system->sixty_second_frames < EVERY_60_SECONDS) {
        return false;
    }
    system->sixty_second_frames = 0u;
    return true;
}

static void arm_random_phantoms(PhantomSystem *system,
                                int hour,
                                bool camera_open,
                                int selected_camera,
                                bool every20,
                                bool every60)
{
    /* All MFA random spawn groups exclude 12 AM ("time of night" == 12). */
    if (hour <= 0) {
        return;
    }

    if (every20) {
        /* BB: every 20s, only while the camera monitor is not fully raised. */
        if (bb_available(system) && !camera_open && !system->bb_camera_visible &&
            ai_roll(system, 10u)) {
            system->bb_armed = true;
        }

        /* Mangle is active from Night 2. A random state-2 spawn consumes its
         * later forced opportunity through event group 642. */
        if (mangle_available(system) && !system->mangle_camera_visible &&
            !system->mangle_office_active &&
            !camera_is_selected(camera_open, selected_camera, CAM04_INDEX) &&
            ai_roll(system, 7u)) {
            system->mangle_camera_visible = true;
            system->mangle_seen_frames = 0u;
            system->forced_mangle = true;
        }

        if (puppet_available(system) && !system->puppet_camera_visible &&
            !system->puppet_office_active &&
            !camera_is_selected(camera_open, selected_camera, CAM08_INDEX) &&
            ai_roll(system, 10u)) {
            system->puppet_camera_visible = true;
            system->puppet_seen_frames = 0u;
            system->forced_puppet = true;
        }

        if (chica_available(system) && !system->chica_camera_visible &&
            !system->chica_office_waiting &&
            !camera_is_selected(camera_open, selected_camera, CAM07_INDEX) &&
            ai_roll(system, 10u)) {
            system->chica_camera_visible = true;
            system->chica_seen_frames = 0u;
            system->forced_chica = true;
        }
    }

    if (every60 && freddy_available(system) && !system->freddy_walking &&
        !system->freddy_post_walk_pending && ai_roll(system, 12u)) {
        /* The PC counter remains 1 here; the 4 AM forced event may still arm
         * the same pending pass, exactly like the MFA. */
        system->freddy_armed = true;
    }
}

static void apply_forced_phantoms(PhantomSystem *system,
                                  int hour,
                                  bool camera_open,
                                  int selected_camera)
{
    if (hour == 3 && !system->forced_bb && bb_available(system) &&
        !camera_open && !system->bb_camera_visible) {
        system->bb_armed = true;
        system->forced_bb = true;
    }

    if (hour == 4 && !system->forced_freddy && freddy_available(system) &&
        !system->freddy_walking && !system->freddy_post_walk_pending) {
        system->freddy_armed = true;
        system->forced_freddy = true;
    }

    if (hour == 5) {
        /* Mangle's forced event uniquely requires the camera monitor fully up. */
        if (!system->forced_mangle && mangle_available(system) &&
            !system->mangle_camera_visible && !system->mangle_office_active &&
            camera_open && selected_camera != CAM04_INDEX) {
            system->mangle_camera_visible = true;
            system->mangle_seen_frames = 0u;
            system->forced_mangle = true;
        }

        if (!system->forced_puppet && puppet_available(system) &&
            !system->puppet_camera_visible && !system->puppet_office_active &&
            !camera_is_selected(camera_open, selected_camera, CAM08_INDEX)) {
            system->puppet_camera_visible = true;
            system->puppet_seen_frames = 0u;
            system->forced_puppet = true;
        }

        if (!system->forced_chica && chica_available(system) &&
            !system->chica_camera_visible && !system->chica_office_waiting &&
            !camera_is_selected(camera_open, selected_camera, CAM07_INDEX)) {
            system->chica_camera_visible = true;
            system->chica_seen_frames = 0u;
            system->forced_chica = true;
        }
    }
}

static PhantomEvent update_bb(PhantomSystem *system,
                              bool camera_open,
                              int selected_camera)
{
    PhantomEvent event = empty_event();

    if (system->bb_armed && camera_open && bb_camera_is_valid(selected_camera)) {
        system->bb_armed = false;
        system->bb_camera_visible = true;
        system->bb_camera = selected_camera;
        system->bb_seen_frames = 0u;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_BALLOON_BOY;
    }

    if (!system->bb_camera_visible) {
        return event;
    }

    /* BBpeek is a one-frame MFA animation. Once shown, its D counter keeps
     * increasing even if the player changes camera or lowers the monitor. */
    ++system->bb_seen_frames;
    if (system->bb_seen_frames > system->time_limit_frames) {
        system->bb_camera_visible = false;
        system->bb_camera = -1;
        system->bb_seen_frames = 0u;
        return start_jumpscare(system, PHANTOM_BALLOON_BOY);
    }

    event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_BALLOON_BOY;
    return event;
}

static PhantomEvent update_freddy(PhantomSystem *system,
                                  bool camera_open,
                                  bool maintenance_open,
                                  bool one_second)
{
    PhantomEvent event = empty_event();

    if (system->freddy_post_walk_pending) {
        if (one_second && system->scare_cooldown_seconds == 0u &&
            (next_random(system) % 2u) == 1u) {
            system->freddy_post_walk_pending = false;
            return start_jumpscare(system, PHANTOM_FREDDY);
        }
        return event;
    }

    if (!system->freddy_walking) {
        if (system->freddy_armed && one_second &&
            (next_random(system) % 2u) == 1u) {
            system->freddy_armed = false;
            system->freddy_walking = true;
            system->freddy_countered = false;
            system->freddy_walk_frames = 0u;
            system->freddy_duck_frames = 0u;
            system->freddy_exposure_frames = 0u;
            system->freddy_path_eighths = 0u;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_FREDDY;
        }
        return event;
    }

    if (system->freddy_duck_frames > 0u) {
        ++system->freddy_duck_frames;
        if (system->freddy_duck_frames >= FREDDY_DUCK_FRAMES) {
            system->freddy_walking = false;
            system->freddy_duck_frames = 0u;
            system->freddy_post_walk_pending = true;
        }
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FREDDY;
        return event;
    }

    ++system->freddy_walk_frames;
    system->freddy_path_eighths += FREDDY_PATH_SPEED_EIGHTHS;

    /* MFA groups 553/561: looking at either system screen resets fwalk A to
     * zero; looking at the office increments it. It is not a permanent cancel. */
    if (camera_open || maintenance_open) {
        system->freddy_exposure_frames = 0u;
        system->freddy_countered = true;
    } else {
        ++system->freddy_exposure_frames;
        system->freddy_countered = false;
    }

    if (system->scare_cooldown_seconds == 0u &&
        system->freddy_exposure_frames >= system->time_limit_frames * 3u) {
        system->freddy_duck_frames = 1u;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FREDDY;
        return event;
    }

    /* If the path leaves the play area before the duck condition, fwalk is
     * destroyed with no scare and Golden Freddy remains eligible for a later
     * 60-second roll. */
    if (system->freddy_path_eighths >= FREDDY_PATH_LENGTH_EIGHTHS) {
        system->freddy_walking = false;
        system->freddy_walk_frames = 0u;
        system->freddy_exposure_frames = 0u;
        system->freddy_path_eighths = 0u;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FREDDY;
        return event;
    }

    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_FREDDY;
    return event;
}

static PhantomEvent update_chica(PhantomSystem *system,
                                 bool camera_open,
                                 bool maintenance_open,
                                 int selected_camera,
                                 int office_pan)
{
    PhantomEvent event = empty_event();

    if (system->chica_office_waiting) {
        if (!camera_open && !maintenance_open && office_pan <= -45) {
            system->chica_office_waiting = false;
            return start_jumpscare(system, PHANTOM_CHICA);
        }
        return event;
    }

    if (!system->chica_camera_visible) {
        return event;
    }

    /* MFA group 569: lowering the monitor resets state 2 and its exposure.
     * Switching to another camera merely pauses the counter. */
    if (!camera_open) {
        system->chica_camera_visible = false;
        system->chica_seen_frames = 0u;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_CHICA;
        return event;
    }

    if (selected_camera == CAM07_INDEX) {
        ++system->chica_seen_frames;
        if (system->chica_seen_frames >= system->time_limit_frames &&
            system->scare_cooldown_seconds == 0u) {
            system->chica_camera_visible = false;
            system->chica_seen_frames = 0u;
            system->chica_office_waiting = true;
            event.flags = PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                          PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_CHICA;
            return event;
        }
    }

    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_CHICA;
    return event;
}

static PhantomEvent update_mangle(PhantomSystem *system,
                                  bool camera_open,
                                  int selected_camera)
{
    PhantomEvent event = empty_event();

    if (system->mangle_office_active) {
        if (system->mangle_office_frames > 0u) {
            --system->mangle_office_frames;
        }
        if (system->mangle_office_frames == 0u) {
            system->mangle_office_active = false;
            event.flags = PHANTOM_EVENT_GARBLE_ENDED |
                          PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_MANGLE;
            return event;
        }
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_MANGLE;
        return event;
    }

    if (!system->mangle_camera_visible) {
        return event;
    }

    if (!camera_open) {
        system->mangle_camera_visible = false;
        system->mangle_seen_frames = 0u;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_MANGLE;
        return event;
    }

    if (selected_camera == CAM04_INDEX) {
        ++system->mangle_seen_frames;
        if (system->mangle_seen_frames >= system->time_limit_frames) {
            system->mangle_camera_visible = false;
            system->mangle_seen_frames = 0u;
            system->mangle_office_active = true;
            system->mangle_office_frames = MANGLE_PATH_FRAMES;
            mark_attacked(system, PHANTOM_MANGLE);
            event.flags = PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                          PHANTOM_EVENT_AUDIO_FAILURE |
                          PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                          PHANTOM_EVENT_GARBLE_STARTED |
                          PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_MANGLE;
            return event;
        }
    }

    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_MANGLE;
    return event;
}

static PhantomEvent update_puppet(PhantomSystem *system,
                                  bool camera_open,
                                  int selected_camera)
{
    PhantomEvent event = empty_event();

    if (system->puppet_office_active) {
        if (system->puppet_office_frames > 0u) {
            --system->puppet_office_frames;
        }
        if (system->puppet_office_frames == 0u) {
            system->puppet_office_active = false;
            event.flags = PHANTOM_EVENT_MASK_ENDED |
                          PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_PUPPET;
            return event;
        }
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_PUPPET;
        return event;
    }

    if (!system->puppet_camera_visible) {
        return event;
    }

    if (!camera_open) {
        system->puppet_camera_visible = false;
        system->puppet_seen_frames = 0u;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_PUPPET;
        return event;
    }

    if (selected_camera == CAM08_INDEX) {
        ++system->puppet_seen_frames;
        if (system->puppet_seen_frames >= system->time_limit_frames &&
            system->scare_cooldown_seconds == 0u) {
            system->puppet_camera_visible = false;
            system->puppet_seen_frames = 0u;
            system->puppet_office_active = true;
            system->puppet_office_frames = PUPPET_MASK_FRAMES;
            mark_attacked(system, PHANTOM_PUPPET);
            event.flags = PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                          PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                          PHANTOM_EVENT_MASK_STARTED |
                          PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_PUPPET;
            return event;
        }
    }

    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_PUPPET;
    return event;
}

void phantoms_reset(PhantomSystem *system,
                    int night,
                    bool aggressive_mode,
                    uint32_t seed)
{
    if (system == NULL) {
        return;
    }

    memset(system, 0, sizeof(*system));
    system->night = night;
    system->aggressive_mode = aggressive_mode;
    system->ai_level = mfa_ai_level(night);
    system->time_limit_frames = mfa_time_limit(night);
    system->rng = seed ^ 0xA17C9E31u;
    system->bb_camera = -1;
    system->jumpscare = PHANTOM_NONE;
}

PhantomEvent phantoms_on_panel_opened(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (system != NULL && system->night >= 2 &&
        !attacked(system, PHANTOM_FOXY)) {
        /* The actual roll belongs to MFA groups 721-724, which fire when the
         * eleven-frame monitor animation finishes. */
        system->foxy_roll_pending = true;
    }
    return event;
}

PhantomEvent phantoms_on_monitor_animation_finished(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (system == NULL || !system->foxy_roll_pending) {
        return event;
    }
    system->foxy_roll_pending = false;

    if (system->night < 2 || attacked(system, PHANTOM_FOXY)) {
        return event;
    }

    uint32_t denominator;
    if (system->night == 2) denominator = 1000u;
    else if (system->night == 3) denominator = 50u;
    else if (system->night == 4) denominator = 25u;
    else denominator = 10u;

    const bool previous = system->foxy_present;
    system->foxy_present = (next_random(system) % denominator) == 1u;
    if (previous != system->foxy_present) {
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FOXY;
    }
    return event;
}

PhantomEvent phantoms_on_hour_changed(PhantomSystem *system, int hour)
{
    PhantomEvent event = empty_event();
    if (system == NULL) {
        return event;
    }

    /* MFA group 743: time of night >= 4 enables aggressive mode.
     * Forced Phantom events themselves are evaluated continuously in update(),
     * because several of them depend on which camera is currently selected. */
    system->aggressive_mode = hour >= 4;
    return event;
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

    const bool one_second = tick_every_second(system);
    const bool every20 = tick_every_20_seconds(system);
    const bool every60 = tick_every_60_seconds(system);

    if (system->jumpscare != PHANTOM_NONE) {
        return update_jumpscare(system);
    }

    system->aggressive_mode = hour >= 4;

    arm_random_phantoms(system, hour, camera_open, selected_camera,
                        every20, every60);
    apply_forced_phantoms(system, hour, camera_open, selected_camera);

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

    sub = update_chica(system, camera_open, maintenance_open,
                       selected_camera, office_pan);
    if ((sub.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
        return sub;
    }
    if (sub.flags != PHANTOM_EVENT_NONE) event = sub;

    sub = update_mangle(system, camera_open, selected_camera);
    if (sub.flags != PHANTOM_EVENT_NONE) {
        /* Preserve simultaneous system/audio event flags from Mangle. */
        if ((sub.flags & (PHANTOM_EVENT_AUDIO_FAILURE |
                          PHANTOM_EVENT_GARBLE_STARTED |
                          PHANTOM_EVENT_GARBLE_ENDED)) != 0u) {
            return sub;
        }
        event = sub;
    }

    sub = update_puppet(system, camera_open, selected_camera);
    if (sub.flags != PHANTOM_EVENT_NONE) {
        if ((sub.flags & (PHANTOM_EVENT_MASK_STARTED |
                          PHANTOM_EVENT_MASK_ENDED)) != 0u) {
            return sub;
        }
        event = sub;
    }

    return event;
}

PhantomId phantoms_camera_overlay(const PhantomSystem *system,
                                  int selected_camera)
{
    if (system == NULL) {
        return PHANTOM_NONE;
    }
    if (system->bb_camera_visible && selected_camera == system->bb_camera) {
        return PHANTOM_BALLOON_BOY;
    }
    if (system->chica_camera_visible && selected_camera == CAM07_INDEX) {
        return PHANTOM_CHICA;
    }
    if (system->mangle_camera_visible && selected_camera == CAM04_INDEX) {
        return PHANTOM_MANGLE;
    }
    if (system->puppet_camera_visible && selected_camera == CAM08_INDEX) {
        return PHANTOM_PUPPET;
    }
    return PHANTOM_NONE;
}

PhantomId phantoms_office_overlay(const PhantomSystem *system)
{
    if (system == NULL) {
        return PHANTOM_NONE;
    }
    if (system->puppet_office_active) return PHANTOM_PUPPET;
    if (system->mangle_office_active) return PHANTOM_MANGLE;
    if (system->chica_office_waiting) return PHANTOM_CHICA;
    if (system->foxy_present) return PHANTOM_FOXY;
    if (system->freddy_walking) return PHANTOM_FREDDY;
    return PHANTOM_NONE;
}

PhantomId phantoms_active_jumpscare(const PhantomSystem *system)
{
    return system != NULL ? system->jumpscare : PHANTOM_NONE;
}

uint32_t phantoms_jumpscare_frames(const PhantomSystem *system)
{
    return system != NULL ? system->jumpscare_frames : 0u;
}

bool phantoms_blocks_input(const PhantomSystem *system)
{
    /* The PC Phantom Head does not set the MFA "frozen" counter. */
    return system != NULL && system->jumpscare != PHANTOM_NONE;
}

bool phantoms_foxy_present(const PhantomSystem *system)
{
    return system != NULL && system->foxy_present;
}

bool phantoms_freddy_walking(const PhantomSystem *system)
{
    return system != NULL && system->freddy_walking;
}

bool phantoms_mangle_active(const PhantomSystem *system)
{
    return system != NULL && system->mangle_office_active;
}

bool phantoms_puppet_active(const PhantomSystem *system)
{
    return system != NULL && system->puppet_office_active;
}

uint32_t phantoms_mangle_remaining(const PhantomSystem *system)
{
    return system != NULL ? system->mangle_office_frames : 0u;
}

uint32_t phantoms_puppet_remaining(const PhantomSystem *system)
{
    return system != NULL ? system->puppet_office_frames : 0u;
}

const char *phantom_name(PhantomId phantom)
{
    static const char *const names[PHANTOM_COUNT] = {
        "PHANTOM FOXY",
        "PHANTOM BALLOON BOY",
        "PHANTOM FREDDY",
        "PHANTOM CHICA",
        "PHANTOM MANGLE",
        "PHANTOM PUPPET",
    };
    return phantom >= 0 && phantom < PHANTOM_COUNT
        ? names[phantom]
        : "NO PHANTOM";
}
