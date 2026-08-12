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
/* MFA groups 220-225: the Play Audio counter is reset to 0 on use and gains
 * one every 1500 ms until it reaches 7. At the port's source cadence of 60 Hz
 * that is an exact 10.5-second recharge, not the old temporary five seconds. */
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
    MESSAGE_DANGER_LOUD
} MessageKind;

typedef enum NightIntroStage {
    NIGHT_INTRO_STAGE_BLACK = 0,
    NIGHT_INTRO_STAGE_STATIC,
    NIGHT_INTRO_STAGE_NUMBER,
    NIGHT_INTRO_STAGE_FADE
} NightIntroStage;

typedef enum GameButton {
    GAME_BUTTON_LEFT = 0,
    GAME_BUTTON_RIGHT,
    GAME_BUTTON_UP,
    GAME_BUTTON_DOWN,
    GAME_BUTTON_CONFIRM,
    GAME_BUTTON_BACK,
    GAME_BUTTON_START,
    GAME_BUTTON_PANEL,
    GAME_BUTTON_SELECT,
    GAME_BUTTON_MODE,
    GAME_BUTTON_SEAL,
    GAME_BUTTON_COUNT
} GameButton;

typedef struct MapPoint {
    int x;
    int y;
} MapPoint;

typedef struct NightConfig {
    uint32_t camera_failure_frame;
    uint32_t audio_failure_frame;
    uint32_t ventilation_failure_frame;
    uint32_t springtrap_move_interval;
    bool springtrap_enabled;
} NightConfig;

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
    PhantomSystem phantoms;
    bool springtrap_jumpscare_right;
} Game;

static const NightConfig kNightConfigs[NIGHT_COUNT] = {
    {NO_FAILURE_FRAME, NO_FAILURE_FRAME, NO_FAILURE_FRAME, 600u, false},
    {NO_FAILURE_FRAME, 2700u, 4500u, 480u, true},
    {2400u, 2100u, 3600u, 390u, true},
    {1800u, 1650u, 3000u, 330u, true},
    {1350u, 1200u, 2400u, 270u, true},
    {900u, 900u, 1800u, 210u, true},
};

static const MapPoint kCameraMapPoints[CAMERA_COUNT] = {
    {0, 2}, {1, 2}, {2, 2}, {2, 1}, {1, 1},
    {0, 1}, {0, 0}, {1, 0}, {2, 0}, {3, 0},
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

static const NightConfig *night_config(int night)
{
    int index = night - 1;
    if (index < 0) index = 0;
    if (index >= NIGHT_COUNT) index = NIGHT_COUNT - 1;
    return &kNightConfigs[index];
}

static uint32_t game_random(Game *game)
{
    game->seed = game->seed * 1664525u + 1013904223u;
    return game->seed;
}

static void set_message(Game *game, MessageKind message)
{
    game->message = message;
    game->message_frames = MESSAGE_FRAMES;
    game->dirty = true;
}

static void stop_ventilation_audio(Game *game)
{
    if (game->breathing_started) {
        audio_stop(AUDIO_CUE_BREATHING);
        game->breathing_started = false;
    }
}

static void set_ventilation_failure(Game *game)
{
    if (game->ventilation_failed) return;
    game->ventilation_failed = true;
    game->ventilation_error_frames = 0u;
    game->player_blinded = false;
    game->breathing_started = false;
    audio_play(AUDIO_CUE_ALARM, 0.78f, false);
    game->dirty = true;
}

static uint32_t springtrap_interval(const Game *game)
{
    const NightConfig *config = night_config(game->night);
    uint32_t interval = config->springtrap_move_interval;
    if (game->ventilation_failed && interval > 120u) interval -= 120u;
    if (game->hour >= 4 && interval > 90u) interval -= 90u;
    if (game->idle_frames >= 300u && interval > 60u) interval -= 60u;
    return interval < 90u ? 90u : interval;
}

static uint32_t repair_duration(const Game *game);
static void trigger_audio_lure(Game *game);
static void draw_office_tv(const Game *game);
static void draw_office_gamepad(const Game *game);
static void draw_camera_feed(const Game *game);
static void draw_vent_map(const Game *game);
static void draw_maintenance(const Game *game);
static void draw_ventilation_overlay(const Game *game);
static void draw_phantom_jumpscare(const Game *game);
static void draw_springtrap_jumpscare(const Game *game);
static void render_game(Game *game);
static void update_game(Game *game, const InputState *input);
