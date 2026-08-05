#include "platform/audio.h"

#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <coreinit/cache.h>
#include <sndcore2/core.h>
#include <sndcore2/voice.h>

#include "platform/storage.h"

#define DECLARE_AUDIO_BIN(name) \
    extern const uint8_t name##_bin[]; \
    extern const uint8_t name##_bin_end[]

#define AUDIO_ALIGNMENT 64u
#define AUDIO_EXTERNAL_MAX_BYTES (8u * 1024u * 1024u)

DECLARE_AUDIO_BIN(vent_quiet1);
DECLARE_AUDIO_BIN(vent_quiet2);
DECLARE_AUDIO_BIN(vent_closer1);
DECLARE_AUDIO_BIN(vent_louder2);
DECLARE_AUDIO_BIN(alarm);
DECLARE_AUDIO_BIN(breathing);
DECLARE_AUDIO_BIN(wait);
DECLARE_AUDIO_BIN(static_sound);
DECLARE_AUDIO_BIN(scream3);
DECLARE_AUDIO_BIN(garble1);
DECLARE_AUDIO_BIN(mask);
DECLARE_AUDIO_BIN(echo1);
DECLARE_AUDIO_BIN(echo3b);
DECLARE_AUDIO_BIN(echo4b);
DECLARE_AUDIO_BIN(phone_night1);
DECLARE_AUDIO_BIN(phone_night2);
DECLARE_AUDIO_BIN(phone_night3);
DECLARE_AUDIO_BIN(phone_night4);
DECLARE_AUDIO_BIN(phone_night5);
DECLARE_AUDIO_BIN(phone_night6);
DECLARE_AUDIO_BIN(six_am);
DECLARE_AUDIO_BIN(select);
DECLARE_AUDIO_BIN(end);
DECLARE_AUDIO_BIN(crank1);
DECLARE_AUDIO_BIN(crank2);
DECLARE_AUDIO_BIN(lever1);
DECLARE_AUDIO_BIN(lever2);
DECLARE_AUDIO_BIN(stare);
DECLARE_AUDIO_BIN(titlemusic);
DECLARE_AUDIO_BIN(startday);

typedef struct AudioClip {
    const uint8_t *data;
    const uint8_t *end;
} AudioClip;

typedef struct AudioVoiceSlot {
    AXVoice *voice;
    bool configured;
} AudioVoiceSlot;

static const AudioClip kEmbeddedClips[AUDIO_CUE_COUNT] = {
    {vent_quiet1_bin, vent_quiet1_bin_end},
    {vent_quiet2_bin, vent_quiet2_bin_end},
    {vent_closer1_bin, vent_closer1_bin_end},
    {vent_louder2_bin, vent_louder2_bin_end},
    {alarm_bin, alarm_bin_end},
    {breathing_bin, breathing_bin_end},
    {wait_bin, wait_bin_end},
    {static_sound_bin, static_sound_bin_end},
    {scream3_bin, scream3_bin_end},
    {garble1_bin, garble1_bin_end},
    {mask_bin, mask_bin_end},
    {echo1_bin, echo1_bin_end},
    {echo3b_bin, echo3b_bin_end},
    {echo4b_bin, echo4b_bin_end},
    {phone_night1_bin, phone_night1_bin_end},
    {phone_night2_bin, phone_night2_bin_end},
    {phone_night3_bin, phone_night3_bin_end},
    {phone_night4_bin, phone_night4_bin_end},
    {phone_night5_bin, phone_night5_bin_end},
    {phone_night6_bin, phone_night6_bin_end},
    {six_am_bin, six_am_bin_end},
    {select_bin, select_bin_end},
    {end_bin, end_bin_end},
    {crank1_bin, crank1_bin_end},
    {crank2_bin, crank2_bin_end},
    {lever1_bin, lever1_bin_end},
    {lever2_bin, lever2_bin_end},
    {stare_bin, stare_bin_end},
    {titlemusic_bin, titlemusic_bin_end},
    {startday_bin, startday_bin_end},
};

static const char *const kExternalPaths[AUDIO_CUE_COUNT] = {
    [AUDIO_CUE_PHONE_NIGHT_1] = "audio/phone_night1.bin",
    [AUDIO_CUE_PHONE_NIGHT_2] = "audio/phone_night2.bin",
    [AUDIO_CUE_PHONE_NIGHT_3] = "audio/phone_night3.bin",
    [AUDIO_CUE_PHONE_NIGHT_4] = "audio/phone_night4.bin",
    [AUDIO_CUE_PHONE_NIGHT_5] = "audio/phone_night5.bin",
    [AUDIO_CUE_PHONE_NIGHT_6] = "audio/phone_night6.bin",
    [AUDIO_CUE_SIX_AM] = "audio/six_am.bin",
    [AUDIO_CUE_SELECT] = "audio/select.bin",
    [AUDIO_CUE_END] = "audio/end.bin",
    [AUDIO_CUE_CAMERA_OPEN] = "audio/crank1.bin",
    [AUDIO_CUE_CAMERA_CLOSE] = "audio/crank2.bin",
    [AUDIO_CUE_MAINTENANCE_CLOSE] = "audio/lever1.bin",
    [AUDIO_CUE_MAINTENANCE_OPEN] = "audio/lever2.bin",
    [AUDIO_CUE_GAME_OVER_AMBIENCE] = "audio/stare.bin",
    [AUDIO_CUE_TITLE_MUSIC] = "audio/titlemusic.bin",
    [AUDIO_CUE_START_DAY] = "audio/startday.bin",
};

static AudioClip sClips[AUDIO_CUE_COUNT];
static uint8_t *sOwnedAudio[AUDIO_CUE_COUNT];
static AudioVoiceSlot sVoices[AUDIO_CUE_COUNT];
static bool sAvailable = false;

static uint16_t volume_to_ax(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    return (uint16_t) (volume * 49152.0f);
}

static void release_external_audio(void)
{
    for (int cue = 0; cue < AUDIO_CUE_COUNT; ++cue) {
        free(sOwnedAudio[cue]);
        sOwnedAudio[cue] = NULL;
        sClips[cue] = kEmbeddedClips[cue];
    }
}

static void load_external_audio(AudioCue cue)
{
    if (cue < 0 || cue >= AUDIO_CUE_COUNT) return;
    const char *path = kExternalPaths[cue];
    if (path == NULL) return;

    size_t byte_size = 0u;
    if (!storage_file_size(path, &byte_size) || byte_size < 2u ||
        byte_size > AUDIO_EXTERNAL_MAX_BYTES) {
        return;
    }
    byte_size &= ~(size_t) 1u;
    const size_t allocation_size =
        (byte_size + AUDIO_ALIGNMENT - 1u) & ~(AUDIO_ALIGNMENT - 1u);
    uint8_t *data = (uint8_t *) memalign(AUDIO_ALIGNMENT, allocation_size);
    if (data == NULL) return;

    size_t bytes_read = 0u;
    if (!storage_read(path, data, byte_size, &bytes_read) ||
        bytes_read != byte_size) {
        free(data);
        return;
    }

    sOwnedAudio[cue] = data;
    sClips[cue].data = data;
    sClips[cue].end = data + byte_size;
}

static void configure_voice(AudioCue cue, bool loop, float volume)
{
    if (cue < 0 || cue >= AUDIO_CUE_COUNT || sVoices[cue].voice == NULL) return;

    AXVoice *voice = sVoices[cue].voice;
    const AudioClip *clip = &sClips[cue];
    const uint32_t byte_size = (uint32_t) (clip->end - clip->data);
    const uint32_t sample_count = byte_size / 2u;

    AXVoiceDeviceMixData mix[6];
    memset(mix, 0, sizeof(mix));
    mix[0].bus[0].volume = 0xC000;
    mix[1].bus[0].volume = 0xC000;

    AXVoiceVeData ve = {.volume = volume_to_ax(volume), .delta = 0};
    AXVoiceOffsets offsets = {
        .dataType = AX_VOICE_FORMAT_LPCM16,
        .loopingEnabled = loop ? AX_VOICE_LOOP_ENABLED : AX_VOICE_LOOP_DISABLED,
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
    memset(sOwnedAudio, 0, sizeof(sOwnedAudio));
    memcpy(sClips, kEmbeddedClips, sizeof(sClips));

    if (storage_init()) {
        for (int cue = 0; cue < AUDIO_CUE_COUNT; ++cue)
            load_external_audio((AudioCue) cue);
    }

    AXInitParams params = {
        .renderer = AX_INIT_RENDERER_32KHZ,
        .pipeline = AX_INIT_PIPELINE_SINGLE,
    };
    AXInitWithParams(&params);
    sAvailable = AXIsInit() != 0;
    if (!sAvailable) {
        release_external_audio();
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
    if (!sAvailable) audio_shutdown();
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
    if (AXIsInit()) AXQuit();
    release_external_audio();
    sAvailable = false;
}

void audio_play(AudioCue cue, float volume, bool loop)
{
    if (!sAvailable || cue < 0 || cue >= AUDIO_CUE_COUNT || sVoices[cue].voice == NULL) return;
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
    if (!sAvailable || cue < 0 || cue >= AUDIO_CUE_COUNT || sVoices[cue].voice == NULL) return;
    AXSetVoiceState(sVoices[cue].voice, AX_VOICE_STATE_STOPPED);
    AXSetVoiceCurrentOffset(sVoices[cue].voice, 0u);
}

void audio_stop_all(void)
{
    for (int cue = 0; cue < AUDIO_CUE_COUNT; ++cue) audio_stop((AudioCue) cue);
}

bool audio_is_available(void)
{
    return sAvailable;
}
