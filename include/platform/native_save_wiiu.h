#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return values for native_save_try_init:
 *   1  -> running as the installed FNaF3 channel and native save is ready
 *   0  -> not the installed channel (for example the Aroma WUHB build)
 *  -1  -> installed channel detected, but native save initialisation failed
 */
int native_save_try_init(char *root_path, size_t root_path_size);

/* Flush/update the current native save after a successful write. */
bool native_save_commit(void);

/* Shut down nn_save / nn_act when the native backend was initialised. */
void native_save_shutdown(void);

bool native_save_is_active(void);

#ifdef __cplusplus
}
#endif
