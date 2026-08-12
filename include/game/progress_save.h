#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum AchievementId {
    ACH_NIGHT_1 = 0,
    ACH_NIGHT_2,
    ACH_NIGHT_3,
    ACH_NIGHT_4,
    ACH_NIGHT_5,
    ACH_NIGHT_6,
    ACH_NIGHT_6_AGGRESSIVE,
    ACH_PHANTOM_JUMPSCARE,
    ACH_SPRINGTRAP_JUMPSCARE,
    ACH_UTINE,
    ACH_COUNT
} AchievementId;

#define ACH_NORMAL_COUNT 9

void progress_save_init(int *unlocked_night);
void progress_save_shutdown(void);

void progress_save_start_new_game(void);
void progress_save_complete_night(int completed_night,
                                  int *unlocked_night);
void progress_save_complete_secret_minigame(int minigame);
void progress_save_complete_aggressive_nightmare(void);

int progress_save_highest_unlocked_night(void);
int progress_save_continue_night(void);
bool progress_save_is_night_completed(int night);
bool progress_save_extras_unlocked(void);
uint8_t progress_save_secret_minigames_mask(void);
bool progress_save_is_secret_minigame_completed(int minigame);
bool progress_save_good_ending_unlocked(void);
bool progress_save_aggressive_nightmare_completed(void);

bool progress_save_unlock_achievement(AchievementId id);
bool progress_save_achievement_unlocked(AchievementId id);
uint16_t progress_save_achievements_mask(void);
int progress_save_normal_achievement_count(void);
bool progress_save_utine_unlocked(void);

const char *progress_save_load_status_text(void);
bool progress_save_load_status_is_error(void);

const char *progress_save_write_status_text(void);
bool progress_save_write_attempted(void);
bool progress_save_last_write_ok(void);
