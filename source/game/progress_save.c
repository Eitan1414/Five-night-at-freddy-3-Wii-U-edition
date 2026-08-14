#include "game/progress_save.h"

#include <stdint.h>

#include "game/save_data.h"
#include "platform/storage.h"

#define PROGRESS_NIGHT_COUNT 6
#define PROGRESS_STORY_NIGHT_COUNT 5
#define PROGRESS_SECRET_MINIGAME_COUNT 6
#define PROGRESS_SECRET_COMPLETE_MASK 0x3Fu
#define PROGRESS_ACHIEVEMENT_AGGRESSIVE 0x01u
#define PROGRESS_BADGES_MASK 0x03FFu
#define PROGRESS_NORMAL_BADGES_MASK 0x01FFu

static uint8_t s_completed_nights_mask = 0u;
static uint8_t s_secret_minigames_mask = 0u;
static uint8_t s_achievement_flags = 0u;
static uint16_t s_achievements_mask = 0u;
static int s_highest_unlocked_night = 1;
static int s_continue_night = 1;
static SaveLoadResult s_load_result = SAVE_LOAD_EMPTY;
static bool s_write_attempted = false;
static bool s_last_write_ok = false;

static int clamp_night(int night)
{
    if (night < 1) return 1;
    if (night > PROGRESS_NIGHT_COUNT) return PROGRESS_NIGHT_COUNT;
    return night;
}

static int clamp_story_night(int night)
{
    if (night < 1) return 1;
    if (night > PROGRESS_STORY_NIGHT_COUNT) return PROGRESS_STORY_NIGHT_COUNT;
    return night;
}

static int normal_achievement_count_from_mask(uint16_t mask)
{
    int count = 0;
    uint16_t normal = mask & PROGRESS_NORMAL_BADGES_MASK;
    while (normal != 0u) {
        count += (normal & 1u) != 0u ? 1 : 0;
        normal >>= 1u;
    }
    return count;
}

static void migrate_legacy_progress_to_achievements(void)
{
    /* Night-completion badges are recoverable from the original save fields,
       so existing players do not lose credit when this system is introduced. */
    for (int night = 1; night <= PROGRESS_NIGHT_COUNT; ++night) {
        if ((s_completed_nights_mask & (uint8_t) (1u << (night - 1))) != 0u)
            s_achievements_mask |= (uint16_t) (1u << (night - 1));
    }
    if ((s_achievement_flags & PROGRESS_ACHIEVEMENT_AGGRESSIVE) != 0u)
        s_achievements_mask |= (uint16_t) (1u << ACH_NIGHT_6_AGGRESSIVE);

    if (normal_achievement_count_from_mask(s_achievements_mask) >= ACH_NORMAL_COUNT)
        s_achievements_mask |= (uint16_t) (1u << ACH_UTINE);
    s_achievements_mask &= PROGRESS_BADGES_MASK;
}

static void write_current_progress(void)
{
    const SaveData data = {
        .unlocked_night = (uint8_t) s_highest_unlocked_night,
        .continue_night = (uint8_t) s_continue_night,
        .completed_nights_mask = s_completed_nights_mask,
        .secret_minigames_mask = s_secret_minigames_mask,
        .achievement_flags = s_achievement_flags,
        .achievements_mask = s_achievements_mask,
    };
    s_write_attempted = true;
    s_last_write_ok = save_data_write(&data);
    if (s_last_write_ok) s_load_result = SAVE_LOAD_OK;
}

void progress_save_init(int *unlocked_night)
{
    s_completed_nights_mask = 0u;
    s_secret_minigames_mask = 0u;
    s_achievement_flags = 0u;
    s_achievements_mask = 0u;
    s_highest_unlocked_night = unlocked_night != NULL
        ? clamp_night(*unlocked_night) : 1;
    s_continue_night = clamp_story_night(s_highest_unlocked_night);
    s_load_result = SAVE_LOAD_EMPTY;
    s_write_attempted = false;
    s_last_write_ok = false;

    if (!storage_init()) {
        s_load_result = SAVE_LOAD_UNAVAILABLE;
        if (unlocked_night != NULL)
            *unlocked_night = s_highest_unlocked_night;
        return;
    }

    SaveData data = {
        .unlocked_night = 1u,
        .continue_night = 1u,
        .completed_nights_mask = 0u,
        .secret_minigames_mask = 0u,
        .achievement_flags = 0u,
        .achievements_mask = 0u,
    };
    s_load_result = save_data_load(&data);
    if (s_load_result == SAVE_LOAD_OK ||
        s_load_result == SAVE_LOAD_RECOVERED) {
        s_highest_unlocked_night = clamp_night(data.unlocked_night);
        s_continue_night = clamp_story_night(data.continue_night);
        s_completed_nights_mask = data.completed_nights_mask;
        s_secret_minigames_mask = data.secret_minigames_mask;
        s_achievement_flags = data.achievement_flags;
        s_achievements_mask = data.achievements_mask & PROGRESS_BADGES_MASK;
        migrate_legacy_progress_to_achievements();
    }

    if (unlocked_night != NULL)
        *unlocked_night = s_highest_unlocked_night;
}

void progress_save_shutdown(void)
{
    storage_shutdown();
}

void progress_save_start_new_game(void)
{
    /* PC New Game restarts only the five-night story Continue chain. Bonus
       unlocks, stars, Extras, secret minigames and Nightmare remain intact. */
    s_continue_night = 1;
    write_current_progress();
}

void progress_save_complete_night(int completed_night,
                                  int *unlocked_night)
{
    const int night = clamp_night(completed_night);
    s_completed_nights_mask |= (uint8_t) (1u << (night - 1));

    int next_unlocked = night < PROGRESS_NIGHT_COUNT ? night + 1 : night;
    if (next_unlocked > s_highest_unlocked_night)
        s_highest_unlocked_night = next_unlocked;

    /* Nights 1-5 form the story Continue chain. Night 6/Nightmare is a
       separate menu item and must never replace Continue. */
    if (night < PROGRESS_STORY_NIGHT_COUNT)
        s_continue_night = night + 1;
    else if (night == PROGRESS_STORY_NIGHT_COUNT)
        s_continue_night = PROGRESS_STORY_NIGHT_COUNT;

    if (unlocked_night != NULL)
        *unlocked_night = s_highest_unlocked_night;

    write_current_progress();
}

void progress_save_complete_secret_minigame(int minigame)
{
    if (minigame < 1 || minigame > PROGRESS_SECRET_MINIGAME_COUNT) return;
    const uint8_t bit = (uint8_t) (1u << (minigame - 1));
    if ((s_secret_minigames_mask & bit) != 0u) return;
    s_secret_minigames_mask |= bit;
    write_current_progress();
}

void progress_save_complete_aggressive_nightmare(void)
{
    if ((s_achievement_flags & PROGRESS_ACHIEVEMENT_AGGRESSIVE) != 0u) return;
    s_achievement_flags |= PROGRESS_ACHIEVEMENT_AGGRESSIVE;
    write_current_progress();
}

int progress_save_highest_unlocked_night(void)
{
    return s_highest_unlocked_night;
}

int progress_save_continue_night(void)
{
    return s_continue_night;
}

bool progress_save_is_night_completed(int night)
{
    if (night < 1 || night > PROGRESS_NIGHT_COUNT) return false;
    return (s_completed_nights_mask & (uint8_t) (1u << (night - 1))) != 0u;
}

bool progress_save_extras_unlocked(void)
{
    return progress_save_is_night_completed(5);
}

uint8_t progress_save_secret_minigames_mask(void)
{
    return s_secret_minigames_mask;
}

bool progress_save_is_secret_minigame_completed(int minigame)
{
    if (minigame < 1 || minigame > PROGRESS_SECRET_MINIGAME_COUNT) return false;
    return (s_secret_minigames_mask & (uint8_t) (1u << (minigame - 1))) != 0u;
}

bool progress_save_good_ending_unlocked(void)
{
    return (s_secret_minigames_mask & PROGRESS_SECRET_COMPLETE_MASK)
        == PROGRESS_SECRET_COMPLETE_MASK;
}

bool progress_save_aggressive_nightmare_completed(void)
{
    return (s_achievement_flags & PROGRESS_ACHIEVEMENT_AGGRESSIVE) != 0u;
}

bool progress_save_unlock_achievement(AchievementId id)
{
    if (id < 0 || id >= ACH_COUNT) return false;
    const uint16_t bit = (uint16_t) (1u << (uint32_t) id);
    if ((s_achievements_mask & bit) != 0u) return false;
    s_achievements_mask |= bit;
    write_current_progress();
    return true;
}

bool progress_save_achievement_unlocked(AchievementId id)
{
    if (id < 0 || id >= ACH_COUNT) return false;
    return (s_achievements_mask &
            (uint16_t) (1u << (uint32_t) id)) != 0u;
}

uint16_t progress_save_achievements_mask(void)
{
    return s_achievements_mask;
}

int progress_save_normal_achievement_count(void)
{
    return normal_achievement_count_from_mask(s_achievements_mask);
}

bool progress_save_utine_unlocked(void)
{
    return progress_save_achievement_unlocked(ACH_UTINE);
}

const char *progress_save_load_status_text(void)
{
    switch (s_load_result) {
        case SAVE_LOAD_UNAVAILABLE: return "SD AUTOSAVE UNAVAILABLE";
        case SAVE_LOAD_EMPTY: return "NO SAVE DATA";
        case SAVE_LOAD_RECOVERED: return "BACKUP SAVE RECOVERED";
        case SAVE_LOAD_CORRUPT: return "SAVE CORRUPT - DEFAULT USED";
        case SAVE_LOAD_OK:
        default: return "AUTOSAVE READY";
    }
}

bool progress_save_load_status_is_error(void)
{
    return s_load_result == SAVE_LOAD_UNAVAILABLE ||
           s_load_result == SAVE_LOAD_CORRUPT;
}

const char *progress_save_write_status_text(void)
{
    if (!s_write_attempted) return "";
    if (s_last_write_ok) return "PROGRESS SAVED";
    if (!storage_is_ready()) return "AUTOSAVE UNAVAILABLE";
    return "SAVE WRITE FAILED";
}

bool progress_save_write_attempted(void)
{
    return s_write_attempted;
}

bool progress_save_last_write_ok(void)
{
    return s_last_write_ok;
}
