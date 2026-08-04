#include "game/phantom_ai.h"

#include <stddef.h>
#include <string.h>

#define FRAMES_PER_SECOND 60u
#define CAMERA_REACTION_FRAMES 33u
#define CHICA_REACTION_FRAMES 15u
#define JUMPSCARE_DURATION_FRAMES 90u
#define FREDDY_WALK_LIMIT_FRAMES 150u
#define FREDDY_DUCK_DELAY_FRAMES 60u

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

static uint32_t timed_interval(const PhantomSystem *system,
                               uint32_t normal_seconds,
                               uint32_t aggressive_seconds)
{
    return (system->aggressive_mode ? aggressive_seconds : normal_seconds) *
           FRAMES_PER_SECOND;
}

static int chance_tenths(const PhantomSystem *system,
                         int normal,
                         int aggressive)
{
    return system->aggressive_mode ? aggressive : normal;
}

static int foxy_chance_twentieths(const PhantomSystem *system)
{
    static const int chance_by_night[] = {0, 1, 3, 5, 7, 10};
    int night = system->night;
    if (night < 1) {
        night = 1;
    }
    if (night > 6) {
        night = 6;
    }
    return chance_by_night[night - 1];
}

static uint32_t mangle_duration(const PhantomSystem *system)
{
    static const uint32_t durations[] = {
        0u, 0u, 300u, 360u, 420u, 480u
    };
    int night = system->night;
    if (night < 1) {
        night = 1;
    }
    if (night > 6) {
        night = 6;
    }
    return durations[night - 1];
}

static uint32_t puppet_duration(const PhantomSystem *system)
{
    static const uint32_t durations[] = {
        0u, 0u, 600u, 720u, 840u, 1020u
    };
    int night = system->night;
    if (night < 1) {
        night = 1;
    }
    if (night > 6) {
        night = 6;
    }
    return durations[night - 1];
}

static PhantomEvent start_jumpscare(PhantomSystem *system, PhantomId phantom)
{
    PhantomEvent event = empty_event();
    if (attacked(system, phantom) || system->jumpscare != PHANTOM_NONE) {
        return event;
    }
    system->jumpscare = phantom;
    system->jumpscare_frames = 0u;
    event.flags = PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                  PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                  PHANTOM_EVENT_JUMPSCARE_STARTED |
                  PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = phantom;
    return event;
}

static void dismiss_camera_phantom(PhantomSystem *system, PhantomId phantom)
{
    switch (phantom) {
        case PHANTOM_BALLOON_BOY:
            system->bb_camera_visible = false;
            system->bb_seen_frames = 0u;
            system->bb_spawn_frames = 0u;
            break;
        case PHANTOM_CHICA:
            system->chica_camera_visible = false;
            system->chica_seen_frames = 0u;
            system->chica_spawn_frames = 0u;
            break;
        case PHANTOM_MANGLE:
            system->mangle_camera_visible = false;
            system->mangle_seen_frames = 0u;
            system->mangle_spawn_frames = 0u;
            break;
        case PHANTOM_PUPPET:
            system->puppet_camera_visible = false;
            system->puppet_seen_frames = 0u;
            system->puppet_spawn_frames = 0u;
            break;
        default:
            break;
    }
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
    system->rng = seed ^ 0xA17C9E31u;
    system->bb_camera = -1;
    system->jumpscare = PHANTOM_NONE;
}

PhantomEvent phantoms_on_panel_opened(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (system == NULL || system->night < 2) {
        return event;
    }

    if (system->foxy_present) {
        system->foxy_present = false;
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_FOXY;
        return event;
    }

    if (!attacked(system, PHANTOM_FOXY)) {
        const int chance = foxy_chance_twentieths(system);
        if (chance > 0 && (int) (next_random(system) % 20u) < chance) {
            system->foxy_present = true;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_FOXY;
        }
    }
    return event;
}

static PhantomEvent start_freddy(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (attacked(system, PHANTOM_FREDDY) || system->freddy_walking) {
        return event;
    }
    system->freddy_walking = true;
    system->freddy_countered = false;
    system->freddy_walk_frames = 0u;
    system->freddy_duck_frames = 0u;
    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_FREDDY;
    return event;
}

PhantomEvent phantoms_on_hour_changed(PhantomSystem *system, int hour)
{
    PhantomEvent event = empty_event();
    if (system == NULL || system->night < 2 || hour < 1) {
        return event;
    }

    if (hour == 4) {
        return start_freddy(system);
    }

    if (!attacked(system, PHANTOM_FREDDY) && !system->freddy_walking) {
        const int chance = system->aggressive_mode ? 5 : 3;
        if ((int) (next_random(system) % 10u) < chance) {
            event = start_freddy(system);
        }
    }

    if (hour == 5 && !attacked(system, PHANTOM_CHICA)) {
        system->chica_camera_visible = true;
        system->chica_seen_frames = 0u;
        event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_CHICA;
    }
    if (hour == 5 && system->night >= 3 &&
        !attacked(system, PHANTOM_MANGLE)) {
        system->mangle_camera_visible = true;
        system->mangle_seen_frames = 0u;
        event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_MANGLE;
    }
    return event;
}

static void maybe_spawn_bb(PhantomSystem *system, int hour)
{
    if (system->night < 2 || hour < 1 ||
        attacked(system, PHANTOM_BALLOON_BOY) ||
        system->bb_camera_visible) {
        return;
    }
    ++system->bb_spawn_frames;
    if (system->bb_spawn_frames < timed_interval(system, 26u, 20u)) {
        return;
    }
    system->bb_spawn_frames = 0u;
    const int chance = chance_tenths(system, 4, 7);
    if ((int) (next_random(system) % 10u) < chance) {
        static const int cameras[] = {0, 6, 8, 9};
        system->bb_camera = cameras[next_random(system) % 4u];
        system->bb_camera_visible = true;
        system->bb_seen_frames = 0u;
    }
}

static void maybe_spawn_chica(PhantomSystem *system, int hour)
{
    if (system->night < 2 || hour < 1 ||
        attacked(system, PHANTOM_CHICA) ||
        system->chica_camera_visible || system->chica_office_waiting) {
        return;
    }
    ++system->chica_spawn_frames;
    if (system->chica_spawn_frames < timed_interval(system, 35u, 20u)) {
        return;
    }
    system->chica_spawn_frames = 0u;
    if ((int) (next_random(system) % 10u) <
        chance_tenths(system, 4, 7)) {
        system->chica_camera_visible = true;
        system->chica_seen_frames = 0u;
    }
}

static void maybe_spawn_mangle(PhantomSystem *system, int hour)
{
    if (system->night < 3 || hour < 1 ||
        attacked(system, PHANTOM_MANGLE) ||
        system->mangle_camera_visible || system->mangle_office_active) {
        return;
    }
    ++system->mangle_spawn_frames;
    if (system->mangle_spawn_frames < timed_interval(system, 35u, 20u)) {
        return;
    }
    system->mangle_spawn_frames = 0u;
    if ((int) (next_random(system) % 10u) <
        chance_tenths(system, 4, 7)) {
        system->mangle_camera_visible = true;
        system->mangle_seen_frames = 0u;
    }
}

static void maybe_spawn_puppet(PhantomSystem *system, int hour)
{
    if (system->night < 3 || hour < 1 ||
        attacked(system, PHANTOM_PUPPET) ||
        system->puppet_camera_visible || system->puppet_office_active) {
        return;
    }
    ++system->puppet_spawn_frames;
    if (system->puppet_spawn_frames < timed_interval(system, 30u, 20u)) {
        return;
    }
    system->puppet_spawn_frames = 0u;
    if ((int) (next_random(system) % 10u) <
        chance_tenths(system, 5, 7)) {
        system->puppet_camera_visible = true;
        system->puppet_seen_frames = 0u;
    }
}

static PhantomEvent update_jumpscare(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (system->jumpscare == PHANTOM_NONE) {
        return event;
    }
    ++system->jumpscare_frames;
    if (system->jumpscare_frames >= JUMPSCARE_DURATION_FRAMES) {
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

static PhantomEvent update_freddy(PhantomSystem *system,
                                  bool camera_open,
                                  bool maintenance_open)
{
    PhantomEvent event = empty_event();
    if (!system->freddy_walking) {
        return event;
    }
    if (camera_open || maintenance_open) {
        system->freddy_countered = true;
    }
    ++system->freddy_walk_frames;
    if (system->freddy_countered) {
        if (system->freddy_walk_frames >= FREDDY_WALK_LIMIT_FRAMES) {
            system->freddy_walking = false;
            system->freddy_walk_frames = 0u;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_FREDDY;
        }
        return event;
    }
    if (system->freddy_walk_frames >= FREDDY_WALK_LIMIT_FRAMES) {
        ++system->freddy_duck_frames;
        if (system->freddy_duck_frames >= FREDDY_DUCK_DELAY_FRAMES) {
            system->freddy_walking = false;
            return start_jumpscare(system, PHANTOM_FREDDY);
        }
    }
    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_FREDDY;
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

    if (system->jumpscare != PHANTOM_NONE) {
        return update_jumpscare(system);
    }

    maybe_spawn_bb(system, hour);
    maybe_spawn_chica(system, hour);
    maybe_spawn_mangle(system, hour);
    maybe_spawn_puppet(system, hour);

    PhantomEvent freddy_event = update_freddy(
        system, camera_open, maintenance_open);
    if (freddy_event.flags != PHANTOM_EVENT_NONE) {
        event.flags |= freddy_event.flags;
        event.phantom = freddy_event.phantom;
        if ((freddy_event.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
            return event;
        }
    }

    if (system->foxy_present && !camera_open && !maintenance_open &&
        office_pan <= -70) {
        system->foxy_present = false;
        return start_jumpscare(system, PHANTOM_FOXY);
    }

    if (system->chica_office_waiting && !camera_open && !maintenance_open &&
        office_pan <= -45) {
        system->chica_office_waiting = false;
        return start_jumpscare(system, PHANTOM_CHICA);
    }

    if (system->mangle_office_active) {
        if (system->mangle_office_frames > 0u) {
            --system->mangle_office_frames;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_MANGLE;
        }
        if (system->mangle_office_frames == 0u) {
            system->mangle_office_active = false;
            mark_attacked(system, PHANTOM_MANGLE);
            event.flags |= PHANTOM_EVENT_GARBLE_ENDED |
                           PHANTOM_EVENT_VISUAL_CHANGED;
        }
        return event;
    }

    if (system->puppet_office_active) {
        if (system->puppet_office_frames > 0u) {
            --system->puppet_office_frames;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_PUPPET;
        }
        if (system->puppet_office_frames == 0u) {
            system->puppet_office_active = false;
            mark_attacked(system, PHANTOM_PUPPET);
            event.flags |= PHANTOM_EVENT_MASK_ENDED |
                           PHANTOM_EVENT_VENTILATION_FAILURE |
                           PHANTOM_EVENT_VISUAL_CHANGED;
        }
        return event;
    }

    if (system->bb_camera_visible) {
        if (!camera_open || selected_camera != system->bb_camera) {
            dismiss_camera_phantom(system, PHANTOM_BALLOON_BOY);
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_BALLOON_BOY;
        } else if (++system->bb_seen_frames >= CAMERA_REACTION_FRAMES) {
            system->bb_camera_visible = false;
            system->bb_seen_frames = 0u;
            return start_jumpscare(system, PHANTOM_BALLOON_BOY);
        }
    }

    if (system->chica_camera_visible) {
        if (!camera_open || selected_camera != 6) {
            dismiss_camera_phantom(system, PHANTOM_CHICA);
            event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_CHICA;
        } else if (++system->chica_seen_frames >= CHICA_REACTION_FRAMES) {
            system->chica_camera_visible = false;
            system->chica_seen_frames = 0u;
            system->chica_office_waiting = true;
            event.flags |= PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                           PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                           PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_CHICA;
        }
    }

    if (system->mangle_camera_visible) {
        if (!camera_open || selected_camera != 3) {
            dismiss_camera_phantom(system, PHANTOM_MANGLE);
            event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_MANGLE;
        } else if (++system->mangle_seen_frames >= CAMERA_REACTION_FRAMES) {
            system->mangle_camera_visible = false;
            system->mangle_seen_frames = 0u;
            system->mangle_office_active = true;
            system->mangle_office_frames = mangle_duration(system);
            event.flags |= PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                           PHANTOM_EVENT_AUDIO_FAILURE |
                           PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                           PHANTOM_EVENT_GARBLE_STARTED |
                           PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_MANGLE;
        }
    }

    if (system->puppet_camera_visible) {
        if (!camera_open || selected_camera != 7) {
            dismiss_camera_phantom(system, PHANTOM_PUPPET);
            event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_PUPPET;
        } else if (++system->puppet_seen_frames >= CAMERA_REACTION_FRAMES) {
            system->puppet_camera_visible = false;
            system->puppet_seen_frames = 0u;
            system->puppet_office_active = true;
            system->puppet_office_frames = puppet_duration(system);
            event.flags |= PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                           PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                           PHANTOM_EVENT_MASK_STARTED |
                           PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_PUPPET;
        }
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
    if (system->chica_camera_visible && selected_camera == 6) {
        return PHANTOM_CHICA;
    }
    if (system->mangle_camera_visible && selected_camera == 3) {
        return PHANTOM_MANGLE;
    }
    if (system->puppet_camera_visible && selected_camera == 7) {
        return PHANTOM_PUPPET;
    }
    return PHANTOM_NONE;
}

PhantomId phantoms_office_overlay(const PhantomSystem *system)
{
    if (system == NULL) {
        return PHANTOM_NONE;
    }
    if (system->puppet_office_active) {
        return PHANTOM_PUPPET;
    }
    if (system->mangle_office_active) {
        return PHANTOM_MANGLE;
    }
    if (system->chica_office_waiting) {
        return PHANTOM_CHICA;
    }
    if (system->foxy_present) {
        return PHANTOM_FOXY;
    }
    if (system->freddy_walking) {
        return PHANTOM_FREDDY;
    }
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
