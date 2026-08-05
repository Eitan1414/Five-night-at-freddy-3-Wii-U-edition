#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct SaveData {
    uint8_t unlocked_night;
    uint8_t completed_nights_mask;
    uint8_t secret_minigames_mask;
    uint8_t achievement_flags;
} SaveData;

typedef enum SaveLoadResult {
    SAVE_LOAD_UNAVAILABLE = 0,
    SAVE_LOAD_EMPTY,
    SAVE_LOAD_OK,
    SAVE_LOAD_RECOVERED,
    SAVE_LOAD_CORRUPT
} SaveLoadResult;

SaveLoadResult save_data_load(SaveData *data);
bool save_data_write(const SaveData *data);
