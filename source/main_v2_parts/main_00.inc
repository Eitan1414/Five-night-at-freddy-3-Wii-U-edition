#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <whb/proc.h>

#include "assets/camera01_texture.h"
#include "assets/camera02_texture.h"
#include "assets/camera03_texture.h"
#include "assets/menu_springtrap_texture.h"
#include "assets/menu_springtrap_texture2.h"
#include "assets/menu_springtrap_texture3.h"
#include "assets/menu_springtrap_texture4.h"
#include "assets/menu_springtrap_texture5.h"
#include "assets/office_texture.h"
#include "assets/warning_texture.h"
#include "game/springtrap_ai.h"
#include "platform/audio.h"
#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"
#include "renderer/texture.h"

#define CAMERA_COUNT 10
#define NIGHT_COUNT 5
#define VENT_COUNT 5
#define TITLE_ITEM_COUNT 3
#define MAINTENANCE_ITEM_COUNT 5
#define TITLE_FRAME_COUNT 5
#define WARNING_FRAMES 180u
#define NIGHT_INTRO_FRAMES 180u
#define FRAMES_PER_HOUR 3600u
#define IDLE_VENT_FAILURE_FRAMES 600u
#define VENT_BLACKOUT_CYCLE_FRAMES 600u
#define VENT_DIM_START_FRAMES 180u
#define VENT_BLACK_START_FRAMES 360u
#define AUDIO_LURE_TRAVEL_FRAMES 90u
#define AUDIO_LURE_COOLDOWN_FRAMES 300u
#define MESSAGE_FRAMES 150u
#define REPAIR_SINGLE_FRAMES 120u
#define REPAIR_ALL_FRAMES 240u
#define NO_FAILURE_FRAME UINT32_MAX

#define COLOUR_BLACK GRAPHICS_RGB(0, 0, 0)
#define COLOUR_GREEN GRAPHICS_RGB(120, 184, 112)
#define COLOUR_WHITE GRAPHICS_RGB(220, 235, 215)
#define COLOUR_RED GRAPHICS_RGB(229, 62, 42)

typedef enum GameScreen {
    SCREEN_WARNING = 0,
    SCREEN_TITLE,
    SCREEN_NIGHT_INTRO,
    SCREEN_OFFICE,
    SCREEN_VICTORY,
    SCREEN_GAME_OVER,
    SCREEN_PLACEHOLDER
} GameScreen;

typedef enum MaintenanceItem {
    MAINT_CAMERA = 0,
    MAINT_AUDIO,
    MAINT_VENTILATION,
    MAINT_REBOOT_ALL,
    MAINT_EXIT
} MaintenanceItem;

typedef enum MessageKind {
    MESSAGE_NONE = 0,
    MESSAGE_AUDIO_PLAYING,
    MESSAGE_AUDIO_SUCCESS,
    MESSAGE_AUDIO_FAILED,
    MESSAGE_AUDIO_RECHARGING,
    MESSAGE_AUDIO_NO_TARGET,
    MESSAGE_VENT_SEALED,
    MESSAGE_DANGER_SOFT,
    MESSAGE_DANGER_LOUD
} MessageKind;

typedef struct NightConfig {
    bool springtrap_enabled;
    uint32_t move_interval;
    uint32_t camera_failure_frame;
    uint32_t ventilation_failure_frame;
    uint32_t audio_failure_frame;
    const char *subtitle;
} NightConfig;

typedef struct MapPoint {
    int x;
    int y;
} MapPoint;

typedef struct SpritePlacement {
    int x;
    int y;
    int width;
    int height;
} SpritePlacement;

typedef struct Game {
    GameScreen screen;
    uint32_t screen_frames;
    uint32_t seed;
    bool dirty;

    int title_selection;
    int unlocked_night;
    int night;
    int hour;
    uint32_t hour_frames;
    uint32_t night_frames;

    int office_pan;
    bool camera_open;
    bool maintenance_open;
    bool vent_map_open;
    int selected_camera;
    SpringtrapVent selected_vent;
    SpringtrapVent sealed_vent;

    bool camera_failed;
    bool audio_failed;
    bool ventilation_failed;
    bool camera_failure_triggered;
    bool audio_failure_triggered;
    bool ventilation_failure_triggered;

    int maintenance_selection;
    bool repairing;
    int repair_target;
    uint32_t repair_frames;

    uint32_t idle_frames;
    uint32_t ventilation_error_frames;
    bool breathing_started;
    bool player_blinded;

    bool audio_lure_active;
    int audio_lure_target;
    uint32_t audio_lure_frames;
    uint32_t audio_lure_cooldown;

    MessageKind message;
    uint32_t message_frames;

    SpringtrapAI springtrap;
} Game;

static const NightConfig kNightConfigs[NIGHT_COUNT] = {
    {false, 0u, NO_FAILURE_FRAME, NO_FAILURE_FRAME, NO_FAILURE_FRAME,
     "SYSTEM ORIENTATION"},
    {true, 600u, 9000u, 12600u, 16200u,
     "SPRINGTRAP ACTIVE"},
    {true, 480u, 7200u, 10800u, 14400u,
     "SYSTEMS UNSTABLE"},
    {true, 360u, 5400u, 9000u, 12600u,
     "AGGRESSIVE MOVEMENT"},
    {true, 240u, 3600u, 7200u, 10800u,
     "FINAL SHIFT"},
};

/* Camera 04-10 use the nearest converted PSX feed as a temporary visual
 * until their full-resolution TIM assets are moved to the GX2 renderer. The
 * map, labels, AI and vent routes are already distinct and fully functional. */
static const TextureRle *const kCameraTextures[CAMERA_COUNT] = {
    &gCamera01Texture,
    &gCamera02Texture,
    &gCamera03Texture,
    &gCamera01Texture,
    &gCamera02Texture,
    &gCamera03Texture,
    &gCamera01Texture,
    &gCamera02Texture,
    &gCamera03Texture,
    &gCamera01Texture,
};

static const TextureRle *const kTitleFrames[TITLE_FRAME_COUNT] = {
    &gMenuSpringtrapTexture,
    &gMenuSpringtrapTexture2,
    &gMenuSpringtrapTexture3,
    &gMenuSpringtrapTexture4,
    &gMenuSpringtrapTexture5,
};

/* Positions follow the user's attached FNaF 3 camera map. */
static const MapPoint kCameraMapPoints[CAMERA_COUNT] = {
    {600, 338}, /* CAM 01 */
    {712, 306}, /* CAM 02 */
    {782, 232}, /* CAM 03 */
    {782, 158}, /* CAM 04 */
    {699, 216}, /* CAM 05 */
    {606, 218}, /* CAM 06 */
    {600, 132}, /* CAM 07 */
    {681, 113}, /* CAM 08 */
    {718, 58},  /* CAM 09 */
    {785, 91},  /* CAM 10 */
};

static const SpritePlacement kSpringtrapPlacements[CAMERA_COUNT] = {
    {337, 86, 166, 236},
    {452, 79, 83, 244},
    {320, 238, 188, 86},
    {46, 75, 90, 236},
    {451, 77, 85, 238},
    {48, 141, 100, 174},
    {53, 80, 168, 207},
    {235, 73, 141, 244},
    {50, 82, 196, 221},
    {432, 79, 118, 239},
};

static uint32_t game_random(Game *game)
{
    game->seed = game->seed * 1664525u + 1013904223u;
    return game->seed;
}

static const NightConfig *night_config(int night)
{
    if (night < 1) {
        night = 1;
    }
    if (night > NIGHT_COUNT) {
        night = NIGHT_COUNT;
    }
    return &kNightConfigs[night - 1];
}
