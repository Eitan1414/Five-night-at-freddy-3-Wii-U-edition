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
#define CAMERA_FAILURE_FRAME 720u
#define VENTILATION_FAILURE_FRAME 1440u
#define AUDIO_FAILURE_FRAME 2160u
#define SPRINGTRAP_MOVE_FRAMES 540u
#define SPRINGTRAP_MOVE_FRAMES_VENT_ERROR 300u
#define AUDIO_LURE_TRAVEL_FRAMES 90u
#define AUDIO_LURE_COOLDOWN_FRAMES 300u
#define AUDIO_LURE_MESSAGE_FRAMES 150u

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
    AUDIO_LURE_MESSAGE_COOLDOWN
} AudioLureMessage;

typedef struct SystemsState {
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
