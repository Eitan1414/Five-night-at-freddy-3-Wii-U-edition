#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum GameButton {
    GAME_BUTTON_UP      = 1u << 0,
    GAME_BUTTON_DOWN    = 1u << 1,
    GAME_BUTTON_LEFT    = 1u << 2,
    GAME_BUTTON_RIGHT   = 1u << 3,
    GAME_BUTTON_CONFIRM = 1u << 4,
    GAME_BUTTON_BACK    = 1u << 5,
    GAME_BUTTON_PANEL   = 1u << 6,
    GAME_BUTTON_START   = 1u << 7,
    GAME_BUTTON_SELECT  = 1u << 8,
    GAME_BUTTON_MODE    = 1u << 9,
    GAME_BUTTON_SEAL    = 1u << 10
} GameButton;

typedef struct InputState {
    uint32_t held;
    uint32_t pressed;
    uint32_t released;
    bool connected;

    /* Wii U GamePad touch state in the logical 854x480 coordinate space. */
    bool touch_held;
    bool touch_pressed;
    bool touch_released;
    int touch_x;
    int touch_y;
} InputState;

void input_init(void);
void input_update(InputState *state);

static inline bool input_is_held(const InputState *state, GameButton button)
{
    return state != NULL && (state->held & (uint32_t) button) != 0u;
}

static inline bool input_was_pressed(const InputState *state, GameButton button)
{
    return state != NULL && (state->pressed & (uint32_t) button) != 0u;
}

static inline bool input_touch_in_rect(const InputState *state,
                                       int x,
                                       int y,
                                       int width,
                                       int height)
{
    return state != NULL && state->touch_held &&
           state->touch_x >= x && state->touch_x < x + width &&
           state->touch_y >= y && state->touch_y < y + height;
}
