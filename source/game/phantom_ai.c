#include "game/phantom_ai.h"

#include <stddef.h>
#include <string.h>

#define FRAMES_PER_SECOND 60u
#define CAMERA_REACTION_FRAMES 90u
#define FREDDY_REACTION_FRAMES (3u * FRAMES_PER_SECOND)
#define TWENTY_SECOND_CHECK_FRAMES (20u * FRAMES_PER_SECOND)
#define FREDDY_CHECK_FRAMES (60u * FRAMES_PER_SECOND)
#define JUMPSCARE_DURATION_FRAMES 90u
#define MANGLE_OFFICE_FRAMES (8u * FRAMES_PER_SECOND)
#define PUPPET_OFFICE_FRAMES (16u * FRAMES_PER_SECOND)

static uint32_t next_random(PhantomSystem *system)
{
    system->rng = system->rng * 1664525u + 1013904223u;
    return system->rng;
}

static int ai_for_night(int night)
{
    static const int values[6] = {0, 2, 3, 4, 5, 7};
    if (night < 1) night = 1;
    if (night > 6) night = 6;
    return values[night - 1];
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
    if (phantom >= 0 && phantom < PHANTOM_COUNT)
        system->attacked_mask |= 1u << (uint32_t) phantom;
}

static bool early_phantoms_active(const PhantomSystem *system)
{
    return system->night >= 3;
}

static bool late_phantoms_active(const PhantomSystem *system)
{
    return system->night >= 4;
}

static PhantomEvent start_jumpscare(PhantomSystem *system, PhantomId phantom)
{
    PhantomEvent event = empty_event();
    if (attacked(system, phantom) || system->jumpscare != PHANTOM_NONE)
        return event;

    system->jumpscare = phantom;
    system->jumpscare_frames = 0u;
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
        case PHANTOM_FOXY: return 11u * 4u;
        case PHANTOM_BALLOON_BOY: return 9u * 4u;
        case PHANTOM_FREDDY: return 7u * 4u;
        case PHANTOM_CHICA: return 6u * 4u;
        default: return JUMPSCARE_DURATION_FRAMES;
    }
}

static PhantomEvent update_jumpscare(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (system->jumpscare == PHANTOM_NONE) return event;

    ++system->jumpscare_frames;
    if (system->jumpscare_frames >= jumpscare_duration_frames(system->jumpscare)) {
        const PhantomId finished = system->jumpscare;
        mark_attacked(system, finished);
        system->jumpscare = PHANTOM_NONE;
        system->jumpscare_frames = 0u;
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

static void clear_camera_phantom(PhantomSystem *system, PhantomId phantom)
{
    switch (phantom) {
        case PHANTOM_BALLOON_BOY:
            system->bb_camera_visible = false;
            system->bb_seen_frames = 0u;
            break;
        case PHANTOM_CHICA:
            system->chica_camera_visible = false;
            system->chica_seen_frames = 0u;
            break;
        case PHANTOM_MANGLE:
            system->mangle_camera_visible = false;
            system->mangle_seen_frames = 0u;
            break;
        case PHANTOM_PUPPET:
            system->puppet_camera_visible = false;
            system->puppet_seen_frames = 0u;
            break;
        default:
            break;
    }
}

static bool chance_out_of(PhantomSystem *system, uint32_t denominator)
{
    if (system->ai_level <= 0 || denominator == 0u) return false;
    return (int) (next_random(system) % denominator) < system->ai_level;
}

static int choose_bb_camera(PhantomSystem *system, int springtrap_camera)
{
    int candidates[10];
    int count = 0;
    for (int camera = 0; camera < 10; ++camera) {
        if (camera == springtrap_camera) continue;
        candidates[count++] = camera;
    }
    if (count <= 0) return -1;
    return candidates[next_random(system) % (uint32_t) count];
}

static PhantomEvent start_freddy(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (!early_phantoms_active(system) || attacked(system, PHANTOM_FREDDY) ||
        system->freddy_walking) return event;

    system->freddy_walking = true;
    system->freddy_countered = false;
    system->freddy_walk_frames = 0u;
    system->freddy_duck_frames = 0u;
    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_FREDDY;
    return event;
}

void phantoms_reset(PhantomSystem *system,
                    int night,
                    bool aggressive_mode,
                    uint32_t seed)
{
    if (system == NULL) return;
    memset(system, 0, sizeof(*system));
    system->night = night;
    system->ai_level = ai_for_night(night);
    system->aggressive_mode = aggressive_mode;
    system->rng = seed ^ 0xA17C9E31u;
    system->bb_camera = -1;
    system->jumpscare = PHANTOM_NONE;
}

PhantomEvent phantoms_on_camera_toggled(PhantomSystem *system,
                                        bool opening,
                                        int office_pan,
                                        int springtrap_camera)
{
    PhantomEvent event = empty_event();
    if (system == NULL || !early_phantoms_active(system)) return event;

    /* Foxy gets an appearance opportunity every time the camera monitor is
     * raised or lowered. If he is only partly seen, moving back right and
     * raising the cameras dismisses him; looking fully left is fatal. */
    if (opening && system->foxy_present) {
        if (office_pan <= -70) {
            system->foxy_present = false;
            return start_jumpscare(system, PHANTOM_FOXY);
        }
        system->foxy_present = false;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FOXY;
        return event;
    }

    if (!system->foxy_present && !attacked(system, PHANTOM_FOXY) &&
        chance_out_of(system, 100u)) {
        system->foxy_present = true;
        event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FOXY;
    }

    if (!opening) return event;

    /* BB and Mangle are monitor-raised hallucinations whose appearance chance
     * is directly tied to the same AI value as Springtrap. */
    if (!attacked(system, PHANTOM_BALLOON_BOY) &&
        !system->bb_camera_visible && chance_out_of(system, 50u)) {
        const int camera = choose_bb_camera(system, springtrap_camera);
        if (camera >= 0) {
            system->bb_camera = camera;
            system->bb_camera_visible = true;
            system->bb_seen_frames = 0u;
            event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_BALLOON_BOY;
        }
    }

    if (late_phantoms_active(system) &&
        !attacked(system, PHANTOM_MANGLE) &&
        !system->mangle_camera_visible && !system->mangle_office_active &&
        springtrap_camera != 3 && chance_out_of(system, 60u)) {
        system->mangle_camera_visible = true;
        system->mangle_seen_frames = 0u;
        event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_MANGLE;
    }

    return event;
}

PhantomEvent phantoms_on_hour_changed(PhantomSystem *system, int hour)
{
    PhantomEvent event = empty_event();
    if (system == NULL || !early_phantoms_active(system)) return event;

    /* Freddy has a guaranteed opportunity at 4 AM. */
    if (hour == 4 && !attacked(system, PHANTOM_FREDDY) &&
        !system->freddy_walking) {
        return start_freddy(system);
    }

    /* Puppet gets the same 5 AM forced appearance used by the PC behaviour,
     * but only from Night 4 onward and never on top of Springtrap (the latter
     * is checked in the regular update before timed spawns). */
    if (hour == 5 && late_phantoms_active(system) &&
        !attacked(system, PHANTOM_PUPPET) &&
        !system->puppet_camera_visible && !system->puppet_office_active) {
        system->puppet_spawn_frames = TWENTY_SECOND_CHECK_FRAMES;
    }
    return event;
}

static void maybe_spawn_freddy(PhantomSystem *system)
{
    if (!early_phantoms_active(system) || attacked(system, PHANTOM_FREDDY) ||
        system->freddy_walking) return;

    if (++system->freddy_spawn_frames < FREDDY_CHECK_FRAMES) return;
    system->freddy_spawn_frames = 0u;

    const uint32_t roll = (next_random(system) % 12u) + 1u;
    if ((int) roll <= system->ai_level) (void) start_freddy(system);
}

static void maybe_spawn_chica(PhantomSystem *system,
                              bool camera_open,
                              int selected_camera,
                              int springtrap_camera)
{
    if (!early_phantoms_active(system) || attacked(system, PHANTOM_CHICA) ||
        system->chica_camera_visible || system->chica_office_waiting) return;

    if (++system->chica_spawn_frames < TWENTY_SECOND_CHECK_FRAMES) return;
    system->chica_spawn_frames = 0u;
    if (springtrap_camera == 6 || (camera_open && selected_camera == 6)) return;

    const uint32_t roll = (next_random(system) % 10u) + 1u;
    if ((int) roll <= system->ai_level) {
        system->chica_camera_visible = true;
        system->chica_seen_frames = 0u;
    }
}

static void maybe_spawn_puppet(PhantomSystem *system,
                               bool camera_open,
                               int selected_camera,
                               int springtrap_camera)
{
    if (!late_phantoms_active(system) || attacked(system, PHANTOM_PUPPET) ||
        system->puppet_camera_visible || system->puppet_office_active) return;

    if (++system->puppet_spawn_frames < TWENTY_SECOND_CHECK_FRAMES) return;
    system->puppet_spawn_frames = 0u;
    if (springtrap_camera == 7 || (camera_open && selected_camera == 7)) return;

    const uint32_t roll = (next_random(system) % 10u) + 1u;
    if ((int) roll <= system->ai_level) {
        system->puppet_camera_visible = true;
        system->puppet_seen_frames = 0u;
    }
}

static PhantomEvent update_freddy(PhantomSystem *system,
                                  bool camera_open,
                                  bool maintenance_open)
{
    PhantomEvent event = empty_event();
    if (!system->freddy_walking) return event;

    if (camera_open || maintenance_open) {
        system->freddy_walking = false;
        system->freddy_countered = true;
        system->freddy_walk_frames = 0u;
        system->freddy_duck_frames = 0u;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FREDDY;
        return event;
    }

    if (system->freddy_walk_frames < FREDDY_REACTION_FRAMES)
        ++system->freddy_walk_frames;
    ++system->freddy_duck_frames;

    if (system->freddy_duck_frames >= FREDDY_REACTION_FRAMES) {
        system->freddy_walking = false;
        return start_jumpscare(system, PHANTOM_FREDDY);
    }

    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_FREDDY;
    return event;
}

static PhantomEvent begin_mangle_office(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    system->mangle_camera_visible = false;
    system->mangle_seen_frames = 0u;
    system->mangle_office_active = true;
    system->mangle_office_frames = MANGLE_OFFICE_FRAMES;
    event.flags = PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                  PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                  PHANTOM_EVENT_GARBLE_STARTED |
                  PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_MANGLE;
    return event;
}

static PhantomEvent begin_puppet_office(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    system->puppet_camera_visible = false;
    system->puppet_seen_frames = 0u;
    system->puppet_office_active = true;
    system->puppet_office_frames = PUPPET_OFFICE_FRAMES;
    event.flags = PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                  PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                  PHANTOM_EVENT_MASK_STARTED |
                  PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_PUPPET;
    return event;
}

static PhantomEvent update_camera_phantom(PhantomSystem *system,
                                          PhantomId phantom,
                                          bool camera_open,
                                          int selected_camera,
                                          int camera,
                                          bool *visible,
                                          uint32_t *seen_frames)
{
    PhantomEvent event = empty_event();
    if (!*visible) return event;

    if (!camera_open || selected_camera != camera) {
        if (*seen_frames > 0u) {
            clear_camera_phantom(system, phantom);
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = phantom;
        }
        return event;
    }

    if (++(*seen_frames) < CAMERA_REACTION_FRAMES) return event;

    if (phantom == PHANTOM_MANGLE) return begin_mangle_office(system);
    if (phantom == PHANTOM_PUPPET) return begin_puppet_office(system);

    *visible = false;
    *seen_frames = 0u;
    return start_jumpscare(system, phantom);
}

PhantomEvent phantoms_update(PhantomSystem *system,
                             int hour,
                             bool camera_open,
                             bool maintenance_open,
                             int selected_camera,
                             int office_pan,
                             int springtrap_camera)
{
    PhantomEvent event = empty_event();
    (void) hour;
    if (system == NULL) return event;

    if (system->jumpscare != PHANTOM_NONE)
        return update_jumpscare(system);

    maybe_spawn_freddy(system);
    maybe_spawn_chica(system, camera_open, selected_camera, springtrap_camera);
    maybe_spawn_puppet(system, camera_open, selected_camera, springtrap_camera);

    PhantomEvent freddy = update_freddy(system, camera_open, maintenance_open);
    if (freddy.flags != PHANTOM_EVENT_NONE) {
        if ((freddy.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u)
            return freddy;
        event = freddy;
    }

    if (system->foxy_present && !camera_open && !maintenance_open &&
        office_pan <= -70) {
        system->foxy_present = false;
        return start_jumpscare(system, PHANTOM_FOXY);
    }

    if (system->mangle_office_active) {
        if (system->mangle_office_frames > 0u) --system->mangle_office_frames;
        event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_MANGLE;
        if (system->mangle_office_frames == 0u) {
            system->mangle_office_active = false;
            mark_attacked(system, PHANTOM_MANGLE);
            event.flags |= PHANTOM_EVENT_GARBLE_ENDED |
                           PHANTOM_EVENT_VENTILATION_FAILURE;
        }
        return event;
    }

    if (system->puppet_office_active) {
        if (system->puppet_office_frames > 0u) --system->puppet_office_frames;
        event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_PUPPET;
        if (system->puppet_office_frames == 0u) {
            system->puppet_office_active = false;
            mark_attacked(system, PHANTOM_PUPPET);
            event.flags |= PHANTOM_EVENT_MASK_ENDED |
                           PHANTOM_EVENT_VENTILATION_FAILURE;
        }
        return event;
    }

    PhantomEvent camera_event = update_camera_phantom(
        system, PHANTOM_BALLOON_BOY, camera_open, selected_camera,
        system->bb_camera, &system->bb_camera_visible, &system->bb_seen_frames);
    if (camera_event.flags != PHANTOM_EVENT_NONE) return camera_event;

    camera_event = update_camera_phantom(
        system, PHANTOM_CHICA, camera_open, selected_camera,
        6, &system->chica_camera_visible, &system->chica_seen_frames);
    if (camera_event.flags != PHANTOM_EVENT_NONE) return camera_event;

    camera_event = update_camera_phantom(
        system, PHANTOM_MANGLE, camera_open, selected_camera,
        3, &system->mangle_camera_visible, &system->mangle_seen_frames);
    if (camera_event.flags != PHANTOM_EVENT_NONE) return camera_event;

    camera_event = update_camera_phantom(
        system, PHANTOM_PUPPET, camera_open, selected_camera,
        7, &system->puppet_camera_visible, &system->puppet_seen_frames);
    if (camera_event.flags != PHANTOM_EVENT_NONE) return camera_event;

    return event;
}

PhantomId phantoms_camera_overlay(const PhantomSystem *system,
                                  int selected_camera)
{
    if (system == NULL) return PHANTOM_NONE;
    if (system->bb_camera_visible && selected_camera == system->bb_camera)
        return PHANTOM_BALLOON_BOY;
    if (system->chica_camera_visible && selected_camera == 6)
        return PHANTOM_CHICA;
    if (system->mangle_camera_visible && selected_camera == 3)
        return PHANTOM_MANGLE;
    if (system->puppet_camera_visible && selected_camera == 7)
        return PHANTOM_PUPPET;
    return PHANTOM_NONE;
}

PhantomId phantoms_office_overlay(const PhantomSystem *system)
{
    if (system == NULL) return PHANTOM_NONE;
    if (system->puppet_office_active) return PHANTOM_PUPPET;
    if (system->mangle_office_active) return PHANTOM_MANGLE;
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
    return system != NULL &&
           (system->jumpscare != PHANTOM_NONE || system->puppet_office_active);
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
