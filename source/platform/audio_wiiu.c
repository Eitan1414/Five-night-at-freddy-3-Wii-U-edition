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
DECLARE_AUDIO_BIN(achievement);
DECLARE_AUDIO_BIN(utine);

/* Full original PC pack: these used to be external/silent-only cues. */
DECLARE_AUDIO_BIN(tablefan);
DECLARE_AUDIO_BIN(rainstorm2);
DECLARE_AUDIO_BIN(danger2b);
DECLARE_AUDIO_BIN(scanner4);
DECLARE_AUDIO_BIN(done);
DECLARE_AUDIO_BIN(collect);
DECLARE_AUDIO_BIN(feed);
DECLARE_AUDIO_BIN(glitch2);
DECLARE_AUDIO_BIN(crowd_children);
DECLARE_AUDIO_BIN(clock_chimes);
DECLARE_AUDIO_BIN(party_favor);
DECLARE_AUDIO_BIN(desolate_underworld);
DECLARE_AUDIO_BIN(crush);
DECLARE_AUDIO_BIN(mb1);
DECLARE_AUDIO_BIN(mb2);
DECLARE_AUDIO_BIN(mb4b);
DECLARE_AUDIO_BIN(mb5);
DECLARE_AUDIO_BIN(mb8);
DECLARE_AUDIO_BIN(mb9);

typedef struct AudioClip {
    const uint8_t *data;
    const uint8_t *end;
} AudioClip;

typedef struct AudioVoiceSlot {
    AXVoice *voice;
    bool configured;
} AudioVoiceSlot;

static const AudioClip kEmbeddedClips[AUDIO_CUE_COUNT] = {
    [AUDIO_CUE_VENT_QUIET_1] = {vent_quiet1_bin, vent_quiet1_bin_end},
    [AUDIO_CUE_VENT_QUIET_2] = {vent_quiet2_bin, vent_quiet2_bin_end},
    [AUDIO_CUE_VENT_CLOSER_1] = {vent_closer1_bin, vent_closer1_bin_end},
    [AUDIO_CUE_VENT_LOUDER_2] = {vent_louder2_bin, vent_louder2_bin_end},
    [AUDIO_CUE_ALARM] = {alarm_bin, alarm_bin_end},
    [AUDIO_CUE_BREATHING] = {breathing_bin, breathing_bin_end},
    [AUDIO_CUE_WAIT] = {wait_bin, wait_bin_end},
    [AUDIO_CUE_STATIC] = {static_sound_bin, static_sound_bin_end},
    [AUDIO_CUE_SCREAM] = {scream3_bin, scream3_bin_end},
    [AUDIO_CUE_GARBLE] = {garble1_bin, garble1_bin_end},
    [AUDIO_CUE_MASK] = {mask_bin, mask_bin_end},
    [AUDIO_CUE_LURE_ECHO_1] = {echo1_bin, echo1_bin_end},
    [AUDIO_CUE_LURE_ECHO_3B] = {echo3b_bin, echo3b_bin_end},
    [AUDIO_CUE_LURE_ECHO_4B] = {echo4b_bin, echo4b_bin_end},
    [AUDIO_CUE_PHONE_NIGHT_1] = {phone_night1_bin, phone_night1_bin_end},
    [AUDIO_CUE_PHONE_NIGHT_2] = {phone_night2_bin, phone_night2_bin_end},
    [AUDIO_CUE_PHONE_NIGHT_3] = {phone_night3_bin, phone_night3_bin_end},
    [AUDIO_CUE_PHONE_NIGHT_4] = {phone_night4_bin, phone_night4_bin_end},
    [AUDIO_CUE_PHONE_NIGHT_5] = {phone_night5_bin, phone_night5_bin_end},
    [AUDIO_CUE_PHONE_NIGHT_6] = {phone_night6_bin, phone_night6_bin_end},
    [AUDIO_CUE_SIX_AM] = {six_am_bin, six_am_bin_end},
    [AUDIO_CUE_SELECT] = {select_bin, select_bin_end},
    [AUDIO_CUE_END] = {end_bin, end_bin_end},
    [AUDIO_CUE_CAMERA_OPEN] = {crank1_bin, crank1_bin_end},
    [AUDIO_CUE_CAMERA_CLOSE] = {crank2_bin, crank2_bin_end},
    [AUDIO_CUE_MAINTENANCE_CLOSE] = {lever1_bin, lever1_bin_end},
    [AUDIO_CUE_MAINTENANCE_OPEN] = {lever2_bin, lever2_bin_end},
    [AUDIO_CUE_GAME_OVER_AMBIENCE] = {stare_bin, stare_bin_end},
    [AUDIO_CUE_TITLE_MUSIC] = {titlemusic_bin, titlemusic_bin_end},
    [AUDIO_CUE_START_DAY] = {startday_bin, startday_bin_end},
    [AUDIO_CUE_ACHIEVEMENT] = {achievement_bin, achievement_bin_end},
    [AUDIO_CUE_UTINE] = {utine_bin, utine_bin_end},
    [AUDIO_CUE_OFFICE_FAN] = {tablefan_bin, tablefan_bin_end},
    [AUDIO_CUE_RAIN_AMBIENCE] = {rainstorm2_bin, rainstorm2_bin_end},
    [AUDIO_CUE_DANGER] = {danger2b_bin, danger2b_bin_end},
    [AUDIO_CUE_REPAIR_SCANNER] = {scanner4_bin, scanner4_bin_end},
    [AUDIO_CUE_REPAIR_DONE] = {done_bin, done_bin_end},
    [AUDIO_CUE_MINIGAME_COLLECT] = {collect_bin, collect_bin_end},
    [AUDIO_CUE_MINIGAME_FEED] = {feed_bin, feed_bin_end},
    [AUDIO_CUE_MINIGAME_GLITCH] = {glitch2_bin, glitch2_bin_end},
    [AUDIO_CUE_MINIGAME_CROWD] = {crowd_children_bin, crowd_children_bin_end},
    [AUDIO_CUE_MINIGAME_CHIMES] = {clock_chimes_bin, clock_chimes_bin_end},
    [AUDIO_CUE_MINIGAME_PARTY_FAVOR] = {party_favor_bin, party_favor_bin_end},
    [AUDIO_CUE_ENDING_DESOLATE] = {desolate_underworld_bin, desolate_underworld_bin_end},
    [AUDIO_CUE_MINIGAME_CRUSH] = {crush_bin, crush_bin_end},
    [AUDIO_CUE_MINIGAME_BB_MUSIC] = {mb1_bin, mb1_bin_end},
    [AUDIO_CUE_MINIGAME_MANGLE_MUSIC] = {mb2_bin, mb2_bin_end},
    [AUDIO_CUE_MINIGAME_CHICA_MUSIC] = {mb4b_bin, mb4b_bin_end},
    [AUDIO_CUE_MINIGAME_STAGE01_MUSIC] = {mb5_bin, mb5_bin_end},
    [AUDIO_CUE_MINIGAME_SHADOW_MUSIC] = {mb8_bin, mb8_bin_end},
    [AUDIO_CUE_MINIGAME_HAPPIEST_MUSIC] = {mb9_bin, mb9_bin_end},
};

/*
 * Retail PC sounds are always taken from the verified embedded sound bank.
 * Only the two Wii U-specific notification sounds may be overridden on SD.
 */
static const char *const kExternalPaths[AUDIO_CUE_COUNT] = {
    [AUDIO_CUE_ACHIEVEMENT] = "audio/achievement.bin",
    [AUDIO_CUE_UTINE] = "audio/utine.bin",
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
        load_external_audio(AUDIO_CUE_ACHIEVEMENT);
        load_external_audio(AUDIO_CUE_UTINE);
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
    if (!sAvailable || cue < 0 || cue >= AUDIO_CUE_COUNT ||
        sVoices[cue].voice == NULL) return;
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
        sVoices[cue].voice == NULL) return;
    AXSetVoiceState(sVoices[cue].voice, AX_VOICE_STATE_STOPPED);
    AXSetVoiceCurrentOffset(sVoices[cue].voice, 0u);
}

void audio_stop_all(void)
{
    for (int cue = 0; cue < AUDIO_CUE_COUNT; ++cue)
        audio_stop((AudioCue) cue);
}

bool audio_is_available(void)
{
    return sAvailable;
}
