#include "platform/pc_minigame_sfx.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <coreinit/cache.h>
#include <sndcore2/core.h>
#include <sndcore2/voice.h>

#define DECLARE_PC_SFX_BIN(name) \
    extern const uint8_t name##_bin[]; \
    extern const uint8_t name##_bin_end[]

DECLARE_PC_SFX_BIN(get);
DECLARE_PC_SFX_BIN(get2);
DECLARE_PC_SFX_BIN(jump);
DECLARE_PC_SFX_BIN(jump2);
DECLARE_PC_SFX_BIN(jump3);
DECLARE_PC_SFX_BIN(jump4);
DECLARE_PC_SFX_BIN(land);
DECLARE_PC_SFX_BIN(run);
DECLARE_PC_SFX_BIN(long_glitched2);
DECLARE_PC_SFX_BIN(insuit);
DECLARE_PC_SFX_BIN(laugh);
DECLARE_PC_SFX_BIN(scare);
DECLARE_PC_SFX_BIN(stop);
DECLARE_PC_SFX_BIN(crazy_garble);

typedef struct PcSfxClip {
    const uint8_t *data;
    const uint8_t *end;
} PcSfxClip;

typedef struct PcSfxSlot {
    AXVoice *voice;
} PcSfxSlot;

static const PcSfxClip kPcSfxClips[PC_MINIGAME_SFX_COUNT] = {
    [PC_MINIGAME_SFX_GET] = {get_bin, get_bin_end},
    [PC_MINIGAME_SFX_GET2] = {get2_bin, get2_bin_end},
    [PC_MINIGAME_SFX_JUMP1] = {jump_bin, jump_bin_end},
    [PC_MINIGAME_SFX_JUMP2] = {jump2_bin, jump2_bin_end},
    [PC_MINIGAME_SFX_JUMP3] = {jump3_bin, jump3_bin_end},
    [PC_MINIGAME_SFX_JUMP4] = {jump4_bin, jump4_bin_end},
    [PC_MINIGAME_SFX_LAND] = {land_bin, land_bin_end},
    [PC_MINIGAME_SFX_RUN] = {run_bin, run_bin_end},
    [PC_MINIGAME_SFX_LONG_GLITCH] = {long_glitched2_bin, long_glitched2_bin_end},
    [PC_MINIGAME_SFX_INSUIT] = {insuit_bin, insuit_bin_end},
    [PC_MINIGAME_SFX_LAUGH] = {laugh_bin, laugh_bin_end},
    [PC_MINIGAME_SFX_SCARE] = {scare_bin, scare_bin_end},
    [PC_MINIGAME_SFX_STOP] = {stop_bin, stop_bin_end},
    [PC_MINIGAME_SFX_CRAZY_GARBLE] = {crazy_garble_bin, crazy_garble_bin_end},
};

static PcSfxSlot sPcSfx[PC_MINIGAME_SFX_COUNT];

static uint16_t pc_sfx_volume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    return (uint16_t)(volume * 49152.0f);
}

static bool pc_sfx_ensure_voice(PcMinigameSfx cue)
{
    if (cue < 0 || cue >= PC_MINIGAME_SFX_COUNT || !AXIsInit()) return false;
    if (sPcSfx[cue].voice != NULL) return true;
    sPcSfx[cue].voice = AXAcquireVoice(30u, NULL, NULL);
    return sPcSfx[cue].voice != NULL;
}

static void pc_sfx_configure(PcMinigameSfx cue, float volume, bool loop)
{
    AXVoice *voice = sPcSfx[cue].voice;
    const PcSfxClip *clip = &kPcSfxClips[cue];
    const uint32_t byte_size = (uint32_t)(clip->end - clip->data);
    const uint32_t sample_count = byte_size / 2u;

    AXVoiceDeviceMixData mix[6];
    memset(mix, 0, sizeof(mix));
    mix[0].bus[0].volume = 0xC000;
    mix[1].bus[0].volume = 0xC000;

    AXVoiceVeData ve = {.volume = pc_sfx_volume(volume), .delta = 0};
    AXVoiceOffsets offsets = {
        .dataType = AX_VOICE_FORMAT_LPCM16,
        .loopingEnabled = loop ? AX_VOICE_LOOP_ENABLED : AX_VOICE_LOOP_DISABLED,
        .loopOffset = 0u,
        .endOffset = sample_count,
        .currentOffset = 0u,
        .data = clip->data,
    };

    DCFlushRange((void *)clip->data, byte_size);
    AXVoiceBegin(voice);
    AXSetVoiceType(voice, AX_VOICE_TYPE_UNKNOWN);
    AXSetVoiceVe(voice, &ve);
    AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_TV, 0, mix);
    AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_DRC, 0, mix);
    AXSetVoiceSrcType(voice, AX_VOICE_SRC_TYPE_LINEAR);
    (void)AXSetVoiceSrcRatio(voice, 0.5f);
    AXSetVoiceOffsets(voice, &offsets);
    AXVoiceEnd(voice);
}

void pc_minigame_sfx_play(PcMinigameSfx cue, float volume, bool loop)
{
    if (!pc_sfx_ensure_voice(cue)) return;
    pc_sfx_configure(cue, volume, loop);
    AXSetVoiceCurrentOffset(sPcSfx[cue].voice, 0u);
    AXSetVoiceState(sPcSfx[cue].voice, AX_VOICE_STATE_PLAYING);
}

void pc_minigame_sfx_stop(PcMinigameSfx cue)
{
    if (cue < 0 || cue >= PC_MINIGAME_SFX_COUNT) return;
    AXVoice *voice = sPcSfx[cue].voice;
    if (voice == NULL) return;
    AXSetVoiceState(voice, AX_VOICE_STATE_STOPPED);
    AXSetVoiceCurrentOffset(voice, 0u);
}

void pc_minigame_sfx_stop_all(void)
{
    for (int cue = 0; cue < PC_MINIGAME_SFX_COUNT; ++cue)
        pc_minigame_sfx_stop((PcMinigameSfx)cue);
}

void pc_minigame_sfx_shutdown(void)
{
    for (int cue = 0; cue < PC_MINIGAME_SFX_COUNT; ++cue) {
        AXVoice *voice = sPcSfx[cue].voice;
        if (voice != NULL) {
            AXSetVoiceState(voice, AX_VOICE_STATE_STOPPED);
            AXFreeVoice(voice);
            sPcSfx[cue].voice = NULL;
        }
    }
}
