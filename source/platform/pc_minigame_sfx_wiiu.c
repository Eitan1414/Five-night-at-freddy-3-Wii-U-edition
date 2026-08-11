#include "platform/pc_minigame_sfx.h"

#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <coreinit/cache.h>
#include <sndcore2/core.h>
#include <sndcore2/voice.h>

#include "platform/storage.h"

#define PC_SFX_ALIGNMENT 64u
#define PC_SFX_MAX_BYTES (8u * 1024u * 1024u)

typedef struct PcSfxSlot {
    uint8_t *data;
    size_t size;
    AXVoice *voice;
} PcSfxSlot;

static PcSfxSlot sPcSfx[PC_MINIGAME_SFX_COUNT];

static const char *const kPcSfxPath[PC_MINIGAME_SFX_COUNT] = {
    [PC_MINIGAME_SFX_GET] = "audio/get.bin",
    [PC_MINIGAME_SFX_GET2] = "audio/get2.bin",
    [PC_MINIGAME_SFX_JUMP1] = "audio/jump.bin",
    [PC_MINIGAME_SFX_JUMP2] = "audio/jump2.bin",
    [PC_MINIGAME_SFX_JUMP3] = "audio/jump3.bin",
    [PC_MINIGAME_SFX_JUMP4] = "audio/jump4.bin",
    [PC_MINIGAME_SFX_LAND] = "audio/land.bin",
    [PC_MINIGAME_SFX_RUN] = "audio/run.bin",
    [PC_MINIGAME_SFX_LONG_GLITCH] = "audio/long_glitched2.bin",
    [PC_MINIGAME_SFX_INSUIT] = "audio/insuit.bin",
    [PC_MINIGAME_SFX_LAUGH] = "audio/laugh.bin",
    [PC_MINIGAME_SFX_SCARE] = "audio/scare.bin",
    [PC_MINIGAME_SFX_STOP] = "audio/stop.bin",
    [PC_MINIGAME_SFX_CRAZY_GARBLE] = "audio/crazy_garble.bin",
};

static uint16_t pc_sfx_volume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    return (uint16_t)(volume * 49152.0f);
}

static bool pc_sfx_load(PcMinigameSfx cue)
{
    if (cue < 0 || cue >= PC_MINIGAME_SFX_COUNT) return false;
    PcSfxSlot *slot = &sPcSfx[cue];
    if (slot->data != NULL && slot->voice != NULL) return true;
    if (!AXIsInit()) return false;

    const char *path = kPcSfxPath[cue];
    if (path == NULL || !storage_init()) return false;

    size_t byte_size = 0u;
    if (!storage_file_size(path, &byte_size) || byte_size < 2u ||
        byte_size > PC_SFX_MAX_BYTES) return false;
    byte_size &= ~(size_t)1u;

    const size_t allocation =
        (byte_size + PC_SFX_ALIGNMENT - 1u) & ~(PC_SFX_ALIGNMENT - 1u);
    uint8_t *data = (uint8_t *)memalign(PC_SFX_ALIGNMENT, allocation);
    if (data == NULL) return false;

    size_t read_size = 0u;
    if (!storage_read(path, data, byte_size, &read_size) ||
        read_size != byte_size) {
        free(data);
        return false;
    }

    AXVoice *voice = AXAcquireVoice(30u, NULL, NULL);
    if (voice == NULL) {
        free(data);
        return false;
    }

    slot->data = data;
    slot->size = byte_size;
    slot->voice = voice;
    return true;
}

static void pc_sfx_configure(PcMinigameSfx cue, float volume, bool loop)
{
    PcSfxSlot *slot = &sPcSfx[cue];
    const uint32_t sample_count = (uint32_t)(slot->size / 2u);

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
        .data = slot->data,
    };

    DCFlushRange(slot->data, slot->size);
    AXVoiceBegin(slot->voice);
    AXSetVoiceType(slot->voice, AX_VOICE_TYPE_UNKNOWN);
    AXSetVoiceVe(slot->voice, &ve);
    AXSetVoiceDeviceMix(slot->voice, AX_DEVICE_TYPE_TV, 0, mix);
    AXSetVoiceDeviceMix(slot->voice, AX_DEVICE_TYPE_DRC, 0, mix);
    AXSetVoiceSrcType(slot->voice, AX_VOICE_SRC_TYPE_LINEAR);
    (void)AXSetVoiceSrcRatio(slot->voice, 0.5f);
    AXSetVoiceOffsets(slot->voice, &offsets);
    AXVoiceEnd(slot->voice);
}

void pc_minigame_sfx_play(PcMinigameSfx cue, float volume, bool loop)
{
    if (!pc_sfx_load(cue)) return;
    PcSfxSlot *slot = &sPcSfx[cue];
    pc_sfx_configure(cue, volume, loop);
    AXSetVoiceCurrentOffset(slot->voice, 0u);
    AXSetVoiceState(slot->voice, AX_VOICE_STATE_PLAYING);
}

void pc_minigame_sfx_stop(PcMinigameSfx cue)
{
    if (cue < 0 || cue >= PC_MINIGAME_SFX_COUNT) return;
    PcSfxSlot *slot = &sPcSfx[cue];
    if (slot->voice == NULL) return;
    AXSetVoiceState(slot->voice, AX_VOICE_STATE_STOPPED);
    AXSetVoiceCurrentOffset(slot->voice, 0u);
}

void pc_minigame_sfx_stop_all(void)
{
    for (int cue = 0; cue < PC_MINIGAME_SFX_COUNT; ++cue)
        pc_minigame_sfx_stop((PcMinigameSfx)cue);
}

void pc_minigame_sfx_shutdown(void)
{
    for (int cue = 0; cue < PC_MINIGAME_SFX_COUNT; ++cue) {
        PcSfxSlot *slot = &sPcSfx[cue];
        if (slot->voice != NULL) {
            AXSetVoiceState(slot->voice, AX_VOICE_STATE_STOPPED);
            AXFreeVoice(slot->voice);
            slot->voice = NULL;
        }
        free(slot->data);
        slot->data = NULL;
        slot->size = 0u;
    }
}
