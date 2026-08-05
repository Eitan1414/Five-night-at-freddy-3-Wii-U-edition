#pragma once

#include <stdbool.h>

void progress_save_init(int *unlocked_night);
void progress_save_shutdown(void);

void progress_save_complete_night(int completed_night,
                                  int *unlocked_night);

int progress_save_highest_unlocked_night(void);

const char *progress_save_load_status_text(void);
bool progress_save_load_status_is_error(void);

const char *progress_save_write_status_text(void);
bool progress_save_write_attempted(void);
bool progress_save_last_write_ok(void);
