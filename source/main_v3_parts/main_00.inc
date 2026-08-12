#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <whb/proc.h>

#include "assets/camera01_texture.h"
#include "assets/camera02_texture.h"
#include "assets/camera03_texture.h"
#include "assets/camera_extended_assets.h"
#include "assets/jumpscare_assets.h"
#include "assets/menu_springtrap_texture.h"
#include "assets/menu_springtrap_texture2.h"
#include "assets/menu_springtrap_texture3.h"
#include "assets/menu_springtrap_texture4.h"
#include "assets/menu_springtrap_texture5.h"
#include "assets/office_texture.h"
#include "assets/phantom_assets.h"
#include "assets/warning_texture.h"
#include "game/phantom_ai.h"
#include "game/springtrap_ai.h"
#include "platform/audio.h"
#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"
#include "renderer/texture.h"

#define CAMERA_COUNT 10
#define NIGHT_COUNT 6
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
/* MFA groups 220-225: Play Audio resets its counter to zero and restores one
 * point every 1500 ms until seven points are available again. */
#define AUDIO_LURE_COOLDOWN_FRAMES 630u
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
    SCREEN_SPRINGTRAP_JUMPSCARE,
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
    MESSAGE_AUDIO_IGNORED,
    MESSAGE_VENT_SEALED,
    MESSAGE_DANGER_SOFT,
    MESSAGE_DANGER_LOUD,
    MESSAGE_PHANTOM_MANGLE,
    MESSAGE_PHANTOM_PUPPET
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

    bool springtrap_jumpscare_right;
    SpringtrapAI springtrap;
    PhantomSystem phantoms;
} Game;

static const NightConfig kNightConfigs[NIGHT_COUNT] = {
    {false, 0u, NO_FAILURE_FRAME, NO_FAILURE_FRAME, NO_FAILURE_FRAME,
     "SYSTEM ORIENTATION"},
    {true, 360u, 9000u, 12600u, 16200u,
     "SPRINGTRAP ACTIVE"},
    {true, 300u, 7200u, 10800u, 14400u,
     "PHANTOMS ACTIVE"},
    {true, 240u, 5400u, 9000u, 12600u,
     "AGGRESSIVE MOVEMENT"},
    {true, 180u, 3600u, 7200u, 10800u,
     "FINAL SHIFT"},
    {true, 120u, 2700u, 5400u, 8100u,
     "AGGRESSIVE MODE"},
};

static const TextureRle *const kCameraTextures[CAMERA_COUNT] = {
    &gCamera01Texture,
    &gCamera02Texture,
    &gCamera03Texture,
    &gCamera04Texture,
    &gCamera05Texture,
    &gCamera06Texture,
    &gCamera07Texture,
    &gCamera08Texture,
    &gCamera09Texture,
    &gCamera10Texture,
};

static const TextureRle *const kTitleFrames[TITLE_FRAME_COUNT] = {
    &gMenuSpringtrapTexture,
    &gMenuSpringtrapTexture2,
    &gMenuSpringtrapTexture3,
    &gMenuSpringtrapTexture4,
    &gMenuSpringtrapTexture5,
};

static const MapPoint kCameraMapPoints[CAMERA_COUNT] = {
    {600, 338}, {712, 306}, {782, 232}, {782, 158}, {699, 216},
    {606, 218}, {600, 132}, {681, 113}, {718, 58}, {785, 91},
};

static uint32_t game_random(Game *game)
{
    game->seed = game->seed * 1664525u + 1013904223u;
    return game->seed;
}

static const NightConfig *night_config(int night)
{
    if (night < 1) night = 1;
    if (night > NIGHT_COUNT) night = NIGHT_COUNT;
    return &kNightConfigs[night - 1];
}

static void set_message(Game *game, MessageKind message)
{
    game->message = message;
    game->message_frames = MESSAGE_FRAMES;
    game->dirty = true;
}

static int title_frame_index(const Game *game)
{
    const uint32_t phase = game->screen_frames % 180u;
    if (phase < 144u) return 0;
    return 1 + (int) (((phase - 144u) / 9u) % 4u);
}

static uint32_t springtrap_interval(const Game *game)
{
    uint32_t interval = night_config(game->night)->move_interval;
    if (game->hour >= 4 && interval > 0u) interval = (interval * 4u) / 5u;
    if (game->hour >= 5 && interval > 0u) interval = (interval * 4u) / 5u;
    if (game->ventilation_failed && interval > 0u) interval = (interval * 3u) / 4u;
    return interval > 0u ? interval : 1u;
}

static void stop_lure_audio(void)
{
    audio_stop(AUDIO_CUE_LURE_ECHO_1);
    audio_stop(AUDIO_CUE_LURE_ECHO_3B);
    audio_stop(AUDIO_CUE_LURE_ECHO_4B);
}

static void stop_ventilation_audio(Game *game)
{
    audio_stop(AUDIO_CUE_ALARM);
    audio_stop(AUDIO_CUE_BREATHING);
    game->breathing_started = false;
}

static void set_ventilation_failure(Game *game)
{
    if (game->ventilation_failed) return;
    game->ventilation_failed = true;
    game->ventilation_error_frames = 0u;
    game->player_blinded = false;
    game->breathing_started = false;
    audio_restart(AUDIO_CUE_ALARM, 0.72f, true);
    game->dirty = true;
}

static void process_phantom_event(Game *game, PhantomEvent event)
{
    if ((event.flags & PHANTOM_EVENT_FORCE_CLOSE_PANEL) != 0u) {
        game->camera_open = false;
        game->maintenance_open = false;
        game->vent_map_open = false;
        audio_stop(AUDIO_CUE_STATIC);
        audio_stop(AUDIO_CUE_WAIT);
    }
    if ((event.flags & PHANTOM_EVENT_RELEASE_SPRINGTRAP) != 0u) {
        springtrap_ai_release_observation(&game->springtrap);
    }
    if ((event.flags & PHANTOM_EVENT_VENTILATION_FAILURE) != 0u) {
        set_ventilation_failure(game);
    }
    if ((event.flags & PHANTOM_EVENT_AUDIO_FAILURE) != 0u) {
        game->audio_failed = true;
        game->audio_lure_active = false;
        stop_lure_audio();
        set_message(game, MESSAGE_PHANTOM_MANGLE);
    }
    if ((event.flags & PHANTOM_EVENT_GARBLE_STARTED) != 0u) {
        audio_restart(AUDIO_CUE_GARBLE, 0.92f, true);
    }
    if ((event.flags & PHANTOM_EVENT_GARBLE_ENDED) != 0u) {
        audio_stop(AUDIO_CUE_GARBLE);
    }
    if ((event.flags & PHANTOM_EVENT_MASK_STARTED) != 0u) {
        audio_restart(AUDIO_CUE_MASK, 0.82f, true);
        set_message(game, MESSAGE_PHANTOM_PUPPET);
    }
    if ((event.flags & PHANTOM_EVENT_MASK_ENDED) != 0u) {
        audio_stop(AUDIO_CUE_MASK);
    }