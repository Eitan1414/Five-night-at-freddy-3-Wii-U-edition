#pragma once

#include <stdbool.h>

#include "renderer/texture.h"

typedef enum MonitorV2TextureId {
    MONITOR_V2_PLAY_AUDIO = 0,
    MONITOR_V2_MAP_TOGGLE,
    MONITOR_V2_TRANSITION_0,
    MONITOR_V2_TRANSITION_1,
    MONITOR_V2_TRANSITION_2,
    MONITOR_V2_TRANSITION_3,
    MONITOR_V2_TRANSITION_4,
    MONITOR_V2_TRANSITION_5,
    MONITOR_V2_TRANSITION_6,
    MONITOR_V2_TRANSITION_7,
    MONITOR_V2_TRANSITION_8,
    MONITOR_V2_TRANSITION_9,
    MONITOR_V2_TRANSITION_10,
    MONITOR_V2_TEXTURE_COUNT
} MonitorV2TextureId;

bool monitor_v2_assets_init(void);
const TextureRle *monitor_v2_texture(MonitorV2TextureId id);
