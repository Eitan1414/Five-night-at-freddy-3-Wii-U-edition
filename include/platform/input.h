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
    GAME_BUTTON_SELECT  = 1u << 8
} GameButton;

typedef struct InputState {
    uint32_t held;
    uint32_t pressed;
    uint32_t released;
    bool connected;
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
