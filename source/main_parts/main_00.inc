#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <whb/proc.h>

#include "assets/camera01_texture.h"
#include "assets/camera02_texture.h"
#include "assets/camera03_texture.h"
#include "assets/camera_springtrap_textures.h"
#include "assets/menu_springtrap_texture.h"
#include "assets/menu_springtrap_texture2.h"
#include "assets/menu_springtrap_texture3.h"
#include "assets/menu_springtrap_texture4.h"
#include "assets/menu_springtrap_texture5.h"
#include "assets/office_texture.h"
#include "assets/warning_texture.h"
#include "game/app.h"
#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"
#include "renderer/texture.h"

#define CAMERA_COUNT 3
#define MAINTENANCE_ITEM_COUNT 5
#define TITLE_FRAME_COUNT 5
#define REPAIR_SINGLE_FRAMES 120u
#define REPAIR_ALL_FRAMES 240u
#define NIGHT_COUNT 5
#define NIGHT_INTRO_FRAMES 180u
#define FRAMES_PER_HOUR 3600u
#define VICTORY_MIN_FRAMES 180u
#define VICTORY_AUTO_FRAMES 720u
#define NO_FAILURE_FRAME UINT32_MAX
#define ATTACK_GRACE_FRAMES 600u
#define AUDIO_LURE_TRAVEL_FRAMES 90u
#define AUDIO_LURE_COOLDOWN_FRAMES 300u
#define AUDIO_LURE_MESSAGE_FRAMES 150u


typedef enum NightPhase {
    NIGHT_PHASE_INTRO = 0,
    NIGHT_PHASE_ACTIVE,
    NIGHT_PHASE_VICTORY,
    NIGHT_PHASE_GAME_OVER
} NightPhase;

typedef struct NightConfig {
    int number;
    bool springtrap_enabled;
    uint32_t springtrap_move_frames;
    uint32_t camera_failure_frame;
    uint32_t ventilation_failure_frame;
    uint32_t audio_failure_frame;
    uint32_t attack_grace_frames;
    const char *subtitle;
} NightConfig;

static const NightConfig kNightConfigs[NIGHT_COUNT] = {
    {1, false, 0u, NO_FAILURE_FRAME, NO_FAILURE_FRAME, NO_FAILURE_FRAME,
     ATTACK_GRACE_FRAMES, "SYSTEM ORIENTATION"},
    {2, true, 900u, 7200u, 10800u, 14400u,
     ATTACK_GRACE_FRAMES, "SPRINGTRAP ACTIVE"},
    {3, true, 720u, 5400u, 9000u, 12600u,
     540u, "SYSTEMS UNSTABLE"},
    {4, true, 540u, 3600u, 7200u, 10800u,
     480u, "AGGRESSIVE MOVEMENT"},
    {5, true, 420u, 2700u, 5400u, 8100u,
     420u, "FINAL SHIFT"},
};

static int g_title_night = 1;

static const NightConfig *night_config(int number)
{
    if (number < 1) {
        number = 1;
    }
    if (number > NIGHT_COUNT) {
        number = NIGHT_COUNT;
    }
    return &kNightConfigs[number - 1];
}

typedef enum MaintenanceItem {
    MAINTENANCE_CAMERA = 0,
    MAINTENANCE_AUDIO,
    MAINTENANCE_VENTILATION,
    MAINTENANCE_REBOOT_ALL,
    MAINTENANCE_EXIT
} MaintenanceItem;

typedef enum AudioLureMessage {
    AUDIO_LURE_MESSAGE_NONE = 0,
    AUDIO_LURE_MESSAGE_PLAYING,
    AUDIO_LURE_MESSAGE_SUCCESS,
    AUDIO_LURE_MESSAGE_ERROR,
    AUDIO_LURE_MESSAGE_COOLDOWN,
    AUDIO_LURE_MESSAGE_NO_TARGET
} AudioLureMessage;

typedef struct SystemsState {
    int night_number;
    NightPhase night_phase;
    uint32_t phase_frames;
    int current_hour;
    uint32_t hour_frames;
    uint32_t attack_frames;
    bool maintenance_open;
    int maintenance_selection;
    bool repairing;
    int repair_target;
    uint32_t repair_frames;
    uint32_t night_frames;
    bool camera_failed;
    bool audio_failed;
    bool ventilation_failed;
    bool camera_failure_triggered;
    bool audio_failure_triggered;
    bool ventilation_failure_triggered;
    int springtrap_camera;
    uint32_t springtrap_move_frames;
    bool audio_lure_active;
    int audio_lure_target;
    uint32_t audio_lure_frames;
    uint32_t audio_lure_cooldown_frames;
    AudioLureMessage audio_lure_message;
    uint32_t audio_lure_message_frames;
} SystemsState;

static const TextureRle *camera_texture(int camera_index)
{
    static const TextureRle *const textures[CAMERA_COUNT] = {
        &gCamera01Texture,
        &gCamera02Texture,
        &gCamera03Texture,
    };
    if (camera_index < 0 || camera_index >= CAMERA_COUNT) {
        camera_index = 0;
    }
    return textures[camera_index];
}

static const TextureRle *camera_springtrap_texture(int camera_index)
{
    static const TextureRle *const textures[CAMERA_COUNT] = {
        &gCamera01SpringtrapTexture,
        &gCamera02SpringtrapTexture,
        &gCamera03SpringtrapTexture,
    };
    if (camera_index < 0 || camera_index >= CAMERA_COUNT) {
        camera_index = 0;
    }
    return textures[camera_index];
}

static const char *camera_label(int camera_index)
{
    static const char *const labels[CAMERA_COUNT] = {
        "CAM 01",
        "CAM 02",
        "CAM 03",
    };
    if (camera_index < 0 || camera_index >= CAMERA_COUNT) {
        camera_index = 0;
    }
    return labels[camera_index];
}

static const TextureRle *title_texture(int frame)
{
    static const TextureRle *const frames[TITLE_FRAME_COUNT] = {
        &gMenuSpringtrapTexture,
        &gMenuSpringtrapTexture2,
        &gMenuSpringtrapTexture3,
        &gMenuSpringtrapTexture4,
        &gMenuSpringtrapTexture5,
    };
    if (frame < 0 || frame >= TITLE_FRAME_COUNT) {
        frame = 0;
    }
    return frames[frame];
}

static int title_frame_index(const App *app)
{
    const uint32_t phase = app->state_frames % 180u;
    if (phase < 144u) {
        return 0;
    }
    return 1 + (int) (((phase - 144u) / 9u) % 4u);
