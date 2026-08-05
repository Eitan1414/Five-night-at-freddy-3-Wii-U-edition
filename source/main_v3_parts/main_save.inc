#include "game/save_data.h"
#include "platform/storage.h"

static uint8_t g_completed_nights_mask = 0u;
static int g_highest_unlocked_night = 1;
static SaveLoadResult g_save_load_result = SAVE_LOAD_EMPTY;
static bool g_save_write_attempted = false;
static bool g_save_last_write_ok = false;

static void save_system_start(Game *game)
{
    SaveData data = {1u, 0u};
    if (!storage_init()) {
        g_save_load_result = SAVE_LOAD_UNAVAILABLE;
        g_highest_unlocked_night = game->unlocked_night;
        return;
    }

    g_save_load_result = save_data_load(&data);
    if (g_save_load_result == SAVE_LOAD_OK ||
        g_save_load_result == SAVE_LOAD_RECOVERED) {
        game->unlocked_night = data.unlocked_night;
        g_completed_nights_mask = data.completed_nights_mask;
    }
    g_highest_unlocked_night = game->unlocked_night;
}

static void save_system_stop(void)
{
    storage_shutdown();
}

static void autosave_completed_night(Game *game)
{
    if (game->night >= 1 && game->night <= NIGHT_COUNT) {
        g_completed_nights_mask |= (uint8_t) (1u << (game->night - 1));
    }
    if (game->night < NIGHT_COUNT &&
        game->unlocked_night < game->night + 1) {
        game->unlocked_night = game->night + 1;
    }
    if (g_highest_unlocked_night < game->unlocked_night)
        g_highest_unlocked_night = game->unlocked_night;

    const SaveData data = {
        (uint8_t) g_highest_unlocked_night,
        g_completed_nights_mask
    };
    g_save_write_attempted = true;
    g_save_last_write_ok = save_data_write(&data);
    if (g_save_last_write_ok) g_save_load_result = SAVE_LOAD_OK;
}

static const char *autosave_result_text(void)
{
    if (!g_save_write_attempted) return "";
    if (g_save_last_write_ok) return "PROGRESS SAVED";
    if (!storage_is_ready()) return "AUTOSAVE UNAVAILABLE";
    return "SAVE WRITE FAILED";
}

static uint32_t autosave_result_colour(void)
{
    return g_save_last_write_ok ? COLOUR_GREEN : COLOUR_RED;
}

#define reset_night reset_night_without_progress_guard
#define game_init game_init_without_autosave
#define begin_victory begin_victory_without_autosave
