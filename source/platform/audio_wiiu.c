#include "platform/audio.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <coreinit/cache.h>
#include <sndcore2/core.h>
#include <sndcore2/voice.h>

extern const uint8_t vent_quiet1_bin[];
extern const uint32_t vent_quiet1_bin_size;
extern const uint8_t vent_quiet2_bin[];
extern const uint32_t vent_quiet2_bin_size;
extern const uint8_t vent_closer1_bin[];
extern const uint32_t vent_closer1_bin_size;
extern const uint8_t vent_louder2_bin[];
extern const uint32_t vent_louder2_bin_size;
extern const uint8_t alarm_bin[];
extern const uint32_t alarm_bin_size;
extern const uint8_t breathing_bin[];
extern const uint32_t breathing_bin_size;
extern const uint8_t wait_bin[];
extern const uint32_t wait_bin_size;
extern const uint8_t static_sound_bin[];
extern const uint32_t static_sound_bin_size;
extern const uint8_t scream3_bin[];
extern const uint32_t scream3_bin_size;

typedef struct AudioClip {
    const uint8_t *data;
    const uint32_t *byte_size;
} AudioClip;

typedef struct AudioVoiceSlot {
    AXVoice *voice;
    bool configured;
} AudioVoiceSlot;

static const AudioClip kClips[AUDIO_CUE_COUNT] = {
    {vent_quiet1_bin, &vent_quiet1_bin_size},
    {vent_quiet2_bin, &vent_quiet2_bin_size},
    {vent_closer1_bin, &vent_closer1_bin_size},
    {vent_louder2_bin, &vent_louder2_bin_size},
    {alarm_bin, &alarm_bin_size},
    {breathing_bin, &breathing_bin_size},
    {wait_bin, &wait_bin_size},
    {static_sound_bin, &static_sound_bin_size},
    {scream3_bin, &scream3_bin_size},
};

static AudioVoiceSlot sVoices[AUDIO_CUE_COUNT];
static bool sAvailable = false;

static uint16_t volume_to_ax(float volume)
{
    if (volume < 0.0f) {
        volume = 0.0f;
    }
    if (volume > 1.0f) {
        volume = 1.0f;
    }
    return (uint16_t) (volume * 49152.0f);
}

static void configure_voice(AudioCue cue, bool loop, float volume)
{
    if (cue < 0 || cue >= AUDIO_CUE_COUNT || sVoices[cue].voice == NULL) {
        return;
    }

    AXVoice *voice = sVoices[cue].voice;
    const AudioClip *clip = &kClips[cue];
    const uint32_t byte_size = *clip->byte_size;
    const uint32_t sample_count = byte_size / 2u;

    AXVoiceDeviceMixData mix[6];
    memset(mix, 0, sizeof(mix));
    mix[0].bus[0].volume = 0xC000;
    mix[1].bus[0].volume = 0xC000;

    AXVoiceVeData ve = {
        .volume = volume_to_ax(volume),
        .delta = 0,
    };
    AXVoiceOffsets offsets = {
        .dataType = AX_VOICE_FORMAT_LPCM16,
        .loopingEnabled = loop ? AX_VOICE_LOOP_ENABLED
                               : AX_VOICE_LOOP_DISABLED,
        .loopOffset = 0u,
        .endOffset = sample_count,
        .currentOffset = 0u,
        .data = clip->data,
    };

    DCFlushRange((void *) clip->data, byte_size);
    AXVoiceBegin(voice);
    AXSetVoiceType(voice, AX_VOICE_TYPE_UNKNOWN);
    AXSetVoiceVe(voice, &ve);
    AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_TV, 0, mix);
    AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_DRC, 0, mix);
    AXSetVoiceSrcType(voice, AX_VOICE_SRC_TYPE_LINEAR);
    (void) AXSetVoiceSrcRatio(voice, 0.5f);
    AXSetVoiceOffsets(voice, &offsets);
    AXVoiceEnd(voice);
    sVoices[cue].configured = true;
}

bool audio_init(void)
{
    memset(sVoices, 0, sizeof(sVoices));
    AXInitParams params = {
        .renderer = AX_INIT_RENDERER_32KHZ,
        .pipeline = AX_INIT_PIPELINE_SINGLE,
    };
    AXInitWithParams(&params);

    sAvailable = AXIsInit() != 0;
    if (!sAvailable) {
        return false;
    }

    for (int cue = 0; cue < AUDIO_CUE_COUNT; ++cue) {
        sVoices[cue].voice = AXAcquireVoice(31u, NULL, NULL);
        if (sVoices[cue].voice == NULL) {
            sAvailable = false;
            break;
        }
        configure_voice((AudioCue) cue, false, 1.0f);
    }
    if (!sAvailable) {
        audio_shutdown();
    }
    return sAvailable;
}

void audio_shutdown(void)
{
    for (int cue = 0; cue < AUDIO_CUE_COUNT; ++cue) {
        if (sVoices[cue].voice != NULL) {
            AXSetVoiceState(sVoices[cue].voice, AX_VOICE_STATE_STOPPED);
            AXFreeVoice(sVoices[cue].voice);
            sVoices[cue].voice = NULL;
            sVoices[cue].configured = false;
        }
    }
    if (AXIsInit()) {
        AXQuit();
    }
    sAvailable = false;
}

void audio_play(AudioCue cue, float volume, bool loop)
{
    if (!sAvailable || cue < 0 || cue >= AUDIO_CUE_COUNT ||
        sVoices[cue].voice == NULL) {
        return;
    }
    configure_voice(cue, loop, volume);
    AXSetVoiceCurrentOffset(sVoices[cue].voice, 0u);
    AXSetVoiceState(sVoices[cue].voice, AX_VOICE_STATE_PLAYING);
}

void audio_restart(AudioCue cue, float volume, bool loop)
{
    audio_stop(cue);
    audio_play(cue, volume, loop);
}

void audio_stop(AudioCue cue)
{
    if (!sAvailable || cue < 0 || cue >= AUDIO_CUE_COUNT ||
        sVoices[cue].voice == NULL) {
        return;
    }
    AXSetVoiceState(sVoices[cue].voice, AX_VOICE_STATE_STOPPED);
    AXSetVoiceCurrentOffset(sVoices[cue].voice, 0u);
}

void audio_stop_all(void)
{
    for (int cue = 0; cue < AUDIO_CUE_COUNT; ++cue) {
        audio_stop((AudioCue) cue);
    }
}

bool audio_is_available(void)
{
    return sAvailable;
}
