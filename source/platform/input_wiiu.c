#include "platform/input.h"

#include <string.h>

#include <vpad/input.h>

static uint32_t sLastHeld = 0u;
static bool sConnected = true;

static uint32_t map_buttons(uint32_t raw)
{
    uint32_t mapped = 0u;

    if ((raw & (VPAD_BUTTON_UP | VPAD_STICK_L_EMULATION_UP)) != 0u) {
        mapped |= GAME_BUTTON_UP;
    }
    if ((raw & (VPAD_BUTTON_DOWN | VPAD_STICK_L_EMULATION_DOWN)) != 0u) {
        mapped |= GAME_BUTTON_DOWN;
    }
    if ((raw & (VPAD_BUTTON_LEFT | VPAD_STICK_L_EMULATION_LEFT)) != 0u) {
        mapped |= GAME_BUTTON_LEFT;
    }
    if ((raw & (VPAD_BUTTON_RIGHT | VPAD_STICK_L_EMULATION_RIGHT)) != 0u) {
        mapped |= GAME_BUTTON_RIGHT;
    }
    if ((raw & VPAD_BUTTON_A) != 0u) {
        mapped |= GAME_BUTTON_CONFIRM;
    }
    if ((raw & VPAD_BUTTON_B) != 0u) {
        mapped |= GAME_BUTTON_BACK;
    }
    if ((raw & (VPAD_BUTTON_X | VPAD_BUTTON_Y)) != 0u) {
        mapped |= GAME_BUTTON_PANEL;
    }
    if ((raw & VPAD_BUTTON_PLUS) != 0u) {
        mapped |= GAME_BUTTON_START;
    }
    if ((raw & VPAD_BUTTON_MINUS) != 0u) {
        mapped |= GAME_BUTTON_SELECT;
    }

    return mapped;
}

void input_init(void)
{
    sLastHeld = 0u;
    sConnected = true;
}

void input_update(InputState *state)
{
    if (state == NULL) {
        return;
    }

    memset(state, 0, sizeof(*state));

    VPADStatus status;
    VPADReadError error = VPAD_READ_SUCCESS;
    const int32_t samples = VPADRead(VPAD_CHAN_0, &status, 1, &error);

    if (samples > 0) {
        state->held = map_buttons(status.hold);
        state->pressed = map_buttons(status.trigger);
        state->released = map_buttons(status.release);
        state->connected = true;
        sLastHeld = state->held;
        sConnected = true;
        return;
    }

    state->held = sLastHeld;
    state->connected = sConnected;

    if (error == VPAD_READ_INVALID_CONTROLLER ||
        error == VPAD_READ_UNINITIALIZED) {
        state->held = 0u;
        state->connected = false;
        sLastHeld = 0u;
        sConnected = false;
    }
}
