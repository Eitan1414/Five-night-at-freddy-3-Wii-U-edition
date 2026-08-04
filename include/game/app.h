#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform/input.h"

typedef enum AppScreen {
    APP_SCREEN_WARNING = 0,
    APP_SCREEN_TITLE,
    APP_SCREEN_OFFICE_TEST,
    APP_SCREEN_PLACEHOLDER
} AppScreen;

typedef struct App {
    AppScreen screen;
    int selected_item;
    int placeholder_item;
    int office_pan;
    bool camera_panel_open;
    bool needs_redraw;
    uint32_t state_frames;
    uint32_t effect_seed;
} App;

void app_init(App *app);
void app_update(App *app, const InputState *input);
void app_render(App *app);
