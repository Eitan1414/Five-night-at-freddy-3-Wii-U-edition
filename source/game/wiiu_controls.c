#include "game/wiiu_controls.h"

#include <stddef.h>
#include <stdint.h>

#include "platform/storage.h"

#define CONTROL_FILE_SIZE 8u
#define CONTROL_VERSION 1u
#define CONTROL_LANGUAGE_MASK 0x0Fu
#define CONTROL_SUBTITLES_FLAG 0x80u

static WiiUControlMode sMode = WIIU_CONTROL_TOUCH_GAMEPAD;
static WiiULanguage sLanguage = WIIU_LANGUAGE_ENGLISH;
static bool sSubtitlesEnabled = false;
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

    const uint8_t preferences =
        ((uint8_t)sLanguage & CONTROL_LANGUAGE_MASK) |
        (sSubtitlesEnabled ? CONTROL_SUBTITLES_FLAG : 0u);

    uint8_t bytes[CONTROL_FILE_SIZE] = {
        'F', '3', 'C', 'T',
        CONTROL_VERSION,
        (uint8_t)sMode,
        preferences,
        0u,
    };
    bytes[7] = control_checksum(bytes);
    (void)storage_write_atomic(kControlPath, bytes, sizeof(bytes));
}

void wiiu_controls_init(void)
{
    sMode = WIIU_CONTROL_TOUCH_GAMEPAD;
    sLanguage = WIIU_LANGUAGE_ENGLISH;
    sSubtitlesEnabled = false;
    if (!storage_is_ready()) return;

    uint8_t bytes[CONTROL_FILE_SIZE];
    size_t bytes_read = 0u;
    if (!storage_read(kControlPath, bytes, sizeof(bytes), &bytes_read) ||
        bytes_read != sizeof(bytes)) {
        return;
    }

    const uint8_t stored_language = bytes[6] & CONTROL_LANGUAGE_MASK;
    if (bytes[0] != 'F' || bytes[1] != '3' ||
        bytes[2] != 'C' || bytes[3] != 'T' ||
        bytes[4] != CONTROL_VERSION ||
        bytes[7] != control_checksum(bytes) ||
        bytes[5] >= (uint8_t)WIIU_CONTROL_MODE_COUNT ||
        stored_language >= (uint8_t)WIIU_LANGUAGE_COUNT) {
        return;
    }

    sMode = (WiiUControlMode)bytes[5];
    sLanguage = (WiiULanguage)stored_language;
    sSubtitlesEnabled = (bytes[6] & CONTROL_SUBTITLES_FLAG) != 0u;
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

WiiULanguage wiiu_controls_language(void)
{
    return sLanguage;
}

void wiiu_controls_set_language(WiiULanguage language)
{
    if (language < WIIU_LANGUAGE_ENGLISH ||
        language >= WIIU_LANGUAGE_COUNT ||
        language == sLanguage) {
        return;
    }
    sLanguage = language;
    write_controls();
}

const char *wiiu_controls_language_name(WiiULanguage language)
{
    switch (language) {
        case WIIU_LANGUAGE_ENGLISH: return "ENGLISH";
        case WIIU_LANGUAGE_FRENCH: return "FRANCAIS";
        default: return "ENGLISH";
    }
}

bool wiiu_controls_subtitles_enabled(void)
{
    return sSubtitlesEnabled;
}

void wiiu_controls_set_subtitles_enabled(bool enabled)
{
    if (enabled == sSubtitlesEnabled) return;
    sSubtitlesEnabled = enabled;
    write_controls();
}
