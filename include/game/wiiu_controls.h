#pragma once

#include <stdbool.h>

typedef enum WiiUControlMode {
    WIIU_CONTROL_TOUCH_GAMEPAD = 0,
    WIIU_CONTROL_TV_ONLY = 1,
    WIIU_CONTROL_MIRROR = 2,
    WIIU_CONTROL_MODE_COUNT
} WiiUControlMode;

typedef enum WiiULanguage {
    WIIU_LANGUAGE_ENGLISH = 0,
    WIIU_LANGUAGE_FRENCH = 1,
    WIIU_LANGUAGE_COUNT
} WiiULanguage;

void wiiu_controls_init(void);
WiiUControlMode wiiu_controls_mode(void);
void wiiu_controls_set_mode(WiiUControlMode mode);
const char *wiiu_controls_mode_name(WiiUControlMode mode);

WiiULanguage wiiu_controls_language(void);
void wiiu_controls_set_language(WiiULanguage language);
const char *wiiu_controls_language_name(WiiULanguage language);

bool wiiu_controls_subtitles_enabled(void);
void wiiu_controls_set_subtitles_enabled(bool enabled);
