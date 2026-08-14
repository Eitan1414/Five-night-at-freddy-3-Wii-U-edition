#pragma once

#include <stdbool.h>

typedef enum WiiUControlMode {
    WIIU_CONTROL_TOUCH_GAMEPAD = 0,
    WIIU_CONTROL_TV_ONLY = 1,
    WIIU_CONTROL_MIRROR = 2,
    WIIU_CONTROL_MODE_COUNT
} WiiUControlMode;

void wiiu_controls_init(void);
WiiUControlMode wiiu_controls_mode(void);
void wiiu_controls_set_mode(WiiUControlMode mode);
const char *wiiu_controls_mode_name(WiiUControlMode mode);
