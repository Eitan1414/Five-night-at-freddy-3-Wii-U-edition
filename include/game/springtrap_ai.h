#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SPRINGTRAP_CAMERA_COUNT 10

typedef enum SpringtrapVent {
    SPRINGTRAP_VENT_NONE = -1,
    SPRINGTRAP_VENT_11 = 0,
    SPRINGTRAP_VENT_12,
    SPRINGTRAP_VENT_13,
    SPRINGTRAP_VENT_14,
    SPRINGTRAP_VENT_15,
    SPRINGTRAP_VENT_COUNT
} SpringtrapVent;

typedef enum SpringtrapLocationKind {
    SPRINGTRAP_LOCATION_CAMERA = 0,
    SPRINGTRAP_LOCATION_VENT,
    SPRINGTRAP_LOCATION_HALL_HIDDEN,
    SPRINGTRAP_LOCATION_OFFICE_WINDOW,
    SPRINGTRAP_LOCATION_OFFICE_LEFT,
    SPRINGTRAP_LOCATION_OFFICE_INSIDE
} SpringtrapLocationKind;

typedef enum SpringtrapCameraPosition {
    SPRINGTRAP_CAMERA_POSITION_BACK = 0,
    SPRINGTRAP_CAMERA_POSITION_EXIT = 1
} SpringtrapCameraPosition;

typedef enum SpringtrapOfficeSide {
    SPRINGTRAP_OFFICE_NONE = 0,
    SPRINGTRAP_OFFICE_WINDOW,
    SPRINGTRAP_OFFICE_LEFT
} SpringtrapOfficeSide;

typedef enum SpringtrapEventFlag {
    SPRINGTRAP_EVENT_NONE              = 0,
    SPRINGTRAP_EVENT_MOVED             = 1u << 0,
    SPRINGTRAP_EVENT_VENT_ENTER        = 1u << 1,
    SPRINGTRAP_EVENT_VENT_EXIT         = 1u << 2,
    SPRINGTRAP_EVENT_DANGER_SOFT       = 1u << 3,
    SPRINGTRAP_EVENT_DANGER_LOUD       = 1u << 4,
    SPRINGTRAP_EVENT_ATTACK            = 1u << 5,
    SPRINGTRAP_EVENT_LURE_IGNORED      = 1u << 6,
    SPRINGTRAP_EVENT_LURE_INVALID      = 1u << 7,
    SPRINGTRAP_EVENT_STAYED            = 1u << 8,
    SPRINGTRAP_EVENT_POSITION_CHANGED  = 1u << 9,
    SPRINGTRAP_EVENT_FORCE_CLOSE_PANEL = 1u << 10
} SpringtrapEventFlag;

typedef struct SpringtrapEvent {
    uint32_t flags;
    int from_camera;
    int to_camera;
    SpringtrapVent vent;
} SpringtrapEvent;

typedef struct SpringtrapAI {
    SpringtrapLocationKind kind;
    int camera;
    SpringtrapCameraPosition camera_position;
    int vent_source_camera;
    SpringtrapVent vent;
    uint32_t move_frames;
    uint32_t opportunity_frames;
    uint32_t vent_frames;
    uint32_t office_inside_frames;
    uint32_t rng;
    bool attack_reported;
    bool office_panel_grace;
} SpringtrapAI;

void springtrap_ai_reset(SpringtrapAI *ai, int night_number, uint32_t seed);

/* Legacy-compatible entry point used by the Extras aggressive cheat. */
SpringtrapEvent springtrap_ai_update(SpringtrapAI *ai,
                                     int night_number,
                                     int current_hour,
                                     uint32_t movement_opportunity_frames,
                                     SpringtrapVent sealed_vent,
                                     bool directly_observed,
                                     bool player_blinded);

/* Full office-aware update used by normal gameplay. */
SpringtrapEvent springtrap_ai_update_ex(SpringtrapAI *ai,
                                        int night_number,
                                        int current_hour,
                                        uint32_t movement_opportunity_frames,
                                        SpringtrapVent sealed_vent,
                                        bool directly_observed,
                                        bool player_blinded,
                                        bool panel_open);
SpringtrapEvent springtrap_ai_lure(SpringtrapAI *ai,
                                   int target_camera,
                                   int night_number);
void springtrap_ai_release_observation(SpringtrapAI *ai);

bool springtrap_ai_is_on_camera(const SpringtrapAI *ai, int camera);
bool springtrap_ai_is_in_vent(const SpringtrapAI *ai);
int springtrap_ai_camera(const SpringtrapAI *ai);
SpringtrapCameraPosition springtrap_ai_camera_position(const SpringtrapAI *ai);
SpringtrapVent springtrap_ai_current_vent(const SpringtrapAI *ai);
SpringtrapOfficeSide springtrap_ai_office_side(const SpringtrapAI *ai);
bool springtrap_ai_is_danger_near(const SpringtrapAI *ai);
bool springtrap_ai_cameras_adjacent(int first, int second);
const char *springtrap_vent_label(SpringtrapVent vent);
int springtrap_vent_source_camera(SpringtrapVent vent);
int springtrap_vent_destination_camera(SpringtrapVent vent);
