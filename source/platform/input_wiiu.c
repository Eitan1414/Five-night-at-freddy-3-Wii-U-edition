#include "platform/input.h"

#include <string.h>

#include <vpad/input.h>

static uint32_t sLastHeld = 0u;
static bool sConnected = true;
static bool sLastTouched = false;
static int sLastTouchX = 0;
static int sLastTouchY = 0;

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
    if ((raw & VPAD_BUTTON_R) != 0u) {
        mapped |= GAME_BUTTON_MODE;
    }
    if ((raw & VPAD_BUTTON_L) != 0u) {
        mapped |= GAME_BUTTON_SEAL;
    }

    return mapped;
}

static void update_touch(InputState *state, const VPADStatus *status)
{
    VPADTouchData calibrated;
    memset(&calibrated, 0, sizeof(calibrated));
    VPADGetTPCalibratedPointEx(VPAD_CHAN_0,
                               VPAD_TP_854X480,
                               &calibrated,
                               &status->tpFiltered1);

    const bool valid = calibrated.validity == VPAD_VALID;
    const bool touched = valid && calibrated.touched != 0u;

    state->touch_valid = valid;
    state->touched = touched;
    state->touch_pressed = touched && !sLastTouched;
    state->touch_released = !touched && sLastTouched;

    if (valid) {
        state->touch_x = (int) calibrated.x;
        state->touch_y = (int) calibrated.y;
        sLastTouchX = state->touch_x;
        sLastTouchY = state->touch_y;
    } else {
        state->touch_x = sLastTouchX;
        state->touch_y = sLastTouchY;
    }

    sLastTouched = touched;
}

void input_init(void)
{
    sLastHeld = 0u;
    sConnected = true;
    sLastTouched = false;
    sLastTouchX = 0;
    sLastTouchY = 0;
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
        update_touch(state, &status);
        sLastHeld = state->held;
        sConnected = true;
        return;
    }

    state->held = sLastHeld;
    state->connected = sConnected;
    state->touch_x = sLastTouchX;
    state->touch_y = sLastTouchY;
    state->touched = sLastTouched;
    state->touch_valid = sConnected;

    if (error == VPAD_READ_INVALID_CONTROLLER ||
        error == VPAD_READ_UNINITIALIZED) {
        state->held = 0u;
        state->connected = false;
        state->touch_valid = false;
        state->touched = false;
        state->touch_released = sLastTouched;
        sLastHeld = 0u;
        sConnected = false;
        sLastTouched = false;
    }
}
