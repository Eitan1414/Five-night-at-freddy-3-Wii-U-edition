#pragma once

#include <stdbool.h>

typedef enum AudioCue {
    AUDIO_CUE_VENT_QUIET_1 = 0,
    AUDIO_CUE_VENT_QUIET_2,
    AUDIO_CUE_VENT_CLOSER_1,
    AUDIO_CUE_VENT_LOUDER_2,
    AUDIO_CUE_ALARM,
    AUDIO_CUE_BREATHING,
    AUDIO_CUE_WAIT,
    AUDIO_CUE_STATIC,
    AUDIO_CUE_SCREAM,
    AUDIO_CUE_GARBLE,
    AUDIO_CUE_MASK,
    AUDIO_CUE_LURE_ECHO_1,
    AUDIO_CUE_LURE_ECHO_3B,
    AUDIO_CUE_LURE_ECHO_4B,
    AUDIO_CUE_COUNT
} AudioCue;

bool audio_init(void);
void audio_shutdown(void);
void audio_play(AudioCue cue, float volume, bool loop);
void audio_stop(AudioCue cue);
void audio_stop_all(void);
void audio_restart(AudioCue cue, float volume, bool loop);
bool audio_is_available(void);
