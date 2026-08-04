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

    if (hour == 5 && !attacked(system, PHANTOM_CHICA) &&
        !system->chica_camera_visible && !system->chica_office_waiting) {
        system->chica_camera_visible = true;
        system->chica_seen_frames = 0u;
        event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = PHANTOM_CHICA;
    }
    if (hour == 5 && system->night >= 3 &&
        !attacked(system, PHANTOM_MANGLE) &&
        !system->mangle_camera_visible && !system->mangle_office_active) {
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
