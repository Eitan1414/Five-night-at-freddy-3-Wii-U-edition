#include "game/wiiu_controls.h"

#include <stddef.h>
#include <stdint.h>

#include "platform/storage.h"

#define CONTROL_FILE_SIZE 8u
#define CONTROL_VERSION 1u

static WiiUControlMode sMode = WIIU_CONTROL_TOUCH_GAMEPAD;
static const char *const kControlPath = "wiiu_controls.dat";

static uint8_t control_checksum(const uint8_t *bytes)
{
    uint8_t value = 0x5Au;
    for (size_t i = 0u; i < 7u; ++i)
        value = (uint8_t)((value << 1u) | (value >> 7u)) ^ bytes[i];
    return value;
}

static void write_controls(void)
{
    if (!storage_is_ready()) return;

    uint8_t bytes[CONTROL_FILE_SIZE] = {
        'F', '3', 'C', 'T',
        CONTROL_VERSION,
        (uint8_t)sMode,
        0u,
        0u,
    };
    bytes[7] = control_checksum(bytes);
    (void)storage_write_atomic(kControlPath, bytes, sizeof(bytes));
}

void wiiu_controls_init(void)
{
    sMode = WIIU_CONTROL_TOUCH_GAMEPAD;
    if (!storage_is_ready()) return;

    uint8_t bytes[CONTROL_FILE_SIZE];
    size_t bytes_read = 0u;
    if (!storage_read(kControlPath, bytes, sizeof(bytes), &bytes_read) ||
        bytes_read != sizeof(bytes)) {
        return;
    }

    if (bytes[0] != 'F' || bytes[1] != '3' ||
        bytes[2] != 'C' || bytes[3] != 'T' ||
        bytes[4] != CONTROL_VERSION ||
        bytes[7] != control_checksum(bytes) ||
        bytes[5] >= (uint8_t)WIIU_CONTROL_MODE_COUNT) {
        return;
    }

    sMode = (WiiUControlMode)bytes[5];
}

WiiUControlMode wiiu_controls_mode(void)
{
    return sMode;
}

void wiiu_controls_set_mode(WiiUControlMode mode)
{
    if (mode < WIIU_CONTROL_TOUCH_GAMEPAD ||
        mode >= WIIU_CONTROL_MODE_COUNT ||
        mode == sMode) {
        return;
    }
    sMode = mode;
    write_controls();
}

const char *wiiu_controls_mode_name(WiiUControlMode mode)
{
    switch (mode) {
        case WIIU_CONTROL_TOUCH_GAMEPAD: return "TOUCH GAMEPAD";
        case WIIU_CONTROL_TV_ONLY: return "TV ONLY";
        case WIIU_CONTROL_MIRROR: return "IDENTICAL";
        default: return "TOUCH GAMEPAD";
    }
}
