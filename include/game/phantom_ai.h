#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum PhantomId {
    PHANTOM_NONE = -1,
    PHANTOM_FOXY = 0,
    PHANTOM_BALLOON_BOY,
    PHANTOM_FREDDY,
    PHANTOM_CHICA,
    PHANTOM_MANGLE,
    PHANTOM_PUPPET,
    PHANTOM_COUNT
} PhantomId;

typedef enum PhantomEventFlag {
    PHANTOM_EVENT_NONE                 = 0,
    PHANTOM_EVENT_VENTILATION_FAILURE = 1u << 0,
    PHANTOM_EVENT_AUDIO_FAILURE       = 1u << 1,
    PHANTOM_EVENT_FORCE_CLOSE_PANEL   = 1u << 2,
    PHANTOM_EVENT_RELEASE_SPRINGTRAP  = 1u << 3,
    PHANTOM_EVENT_JUMPSCARE_STARTED   = 1u << 4,
    PHANTOM_EVENT_JUMPSCARE_ENDED     = 1u << 5,
    PHANTOM_EVENT_GARBLE_STARTED      = 1u << 6,
    PHANTOM_EVENT_GARBLE_ENDED        = 1u << 7,
    PHANTOM_EVENT_MASK_STARTED        = 1u << 8,
    PHANTOM_EVENT_MASK_ENDED          = 1u << 9,
    PHANTOM_EVENT_VISUAL_CHANGED      = 1u << 10
} PhantomEventFlag;

typedef struct PhantomEvent {
    uint32_t flags;
    PhantomId phantom;
} PhantomEvent;

typedef struct PhantomSystem {
    int night;
    bool aggressive_mode;
    int ai_level;
    uint32_t time_limit_frames;
    uint32_t rng;
    uint32_t attacked_mask;

    /* Clickteam "Every" conditions and shared scare cooldown. */
    uint32_t one_second_frames;
    uint32_t twenty_second_frames;
    uint32_t sixty_second_frames;
    uint32_t scare_cooldown_seconds;

    bool forced_bb;
    bool forced_chica;
    bool forced_puppet;
    bool forced_freddy;
    bool forced_mangle;

    bool foxy_present;
    bool foxy_roll_pending;

    /* BB counter remains 1 while BBpeek is merely armed. */
    bool bb_armed;
    bool bb_camera_visible;
    int bb_camera;
    uint32_t bb_spawn_frames;
    uint32_t bb_seen_frames;

    /* fwalk: arm -> 50%/s create -> path -> duck -> 50%/s scare. */
    bool freddy_armed;
    bool freddy_walking;
    bool freddy_countered;
    bool freddy_post_walk_pending;
    uint32_t freddy_walk_frames;
    uint32_t freddy_duck_frames;
    uint32_t freddy_exposure_frames;
    uint32_t freddy_path_eighths;

    bool chica_camera_visible;
    bool chica_office_waiting;
    uint32_t chica_spawn_frames;
    uint32_t chica_seen_frames;

    bool mangle_camera_visible;
    bool mangle_office_active;
    uint32_t mangle_spawn_frames;
    uint32_t mangle_seen_frames;
    uint32_t mangle_office_frames;

    bool puppet_camera_visible;
    bool puppet_office_active;
    uint32_t puppet_spawn_frames;
    uint32_t puppet_seen_frames;
    uint32_t puppet_office_frames;

    PhantomId jumpscare;
    uint32_t jumpscare_frames;
} PhantomSystem;

void phantoms_reset(PhantomSystem *system,
                    int night,
                    bool aggressive_mode,
                    uint32_t seed);

PhantomEvent phantoms_on_panel_opened(PhantomSystem *system);
PhantomEvent phantoms_on_monitor_animation_finished(PhantomSystem *system);
PhantomEvent phantoms_on_hour_changed(PhantomSystem *system, int hour);
PhantomEvent phantoms_update(PhantomSystem *system,
                             int hour,
                             bool camera_open,
                             bool maintenance_open,
                             int selected_camera,
                             int office_pan);

PhantomId phantoms_camera_overlay(const PhantomSystem *system,
                                  int selected_camera);
PhantomId phantoms_office_overlay(const PhantomSystem *system);
PhantomId phantoms_active_jumpscare(const PhantomSystem *system);
uint32_t phantoms_jumpscare_frames(const PhantomSystem *system);
bool phantoms_blocks_input(const PhantomSystem *system);
bool phantoms_foxy_present(const PhantomSystem *system);
bool phantoms_freddy_walking(const PhantomSystem *system);
bool phantoms_mangle_active(const PhantomSystem *system);
bool phantoms_puppet_active(const PhantomSystem *system);
uint32_t phantoms_mangle_remaining(const PhantomSystem *system);
uint32_t phantoms_puppet_remaining(const PhantomSystem *system);
const char *phantom_name(PhantomId phantom);
