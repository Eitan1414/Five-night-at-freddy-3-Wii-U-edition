#pragma once

#include <stdbool.h>
#include <stdint.h>

void progress_save_init(int *unlocked_night);
void progress_save_shutdown(void);

void progress_save_complete_night(int completed_night,
                                  int *unlocked_night);
void progress_save_complete_secret_minigame(int minigame);
void progress_save_complete_aggressive_nightmare(void);

int progress_save_highest_unlocked_night(void);
bool progress_save_is_night_completed(int night);
bool progress_save_extras_unlocked(void);
uint8_t progress_save_secret_minigames_mask(void);
bool progress_save_is_secret_minigame_completed(int minigame);
bool progress_save_good_ending_unlocked(void);
bool progress_save_aggressive_nightmare_completed(void);

const char *progress_save_load_status_text(void);
bool progress_save_load_status_is_error(void);

const char *progress_save_write_status_text(void);
bool progress_save_write_attempted(void);
bool progress_save_last_write_ok(void);
