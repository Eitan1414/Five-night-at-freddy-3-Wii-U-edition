#pragma once

#include <stdbool.h>

typedef enum PcMinigameSfx {
    PC_MINIGAME_SFX_GET = 0,
    PC_MINIGAME_SFX_GET2,
    PC_MINIGAME_SFX_JUMP1,
    PC_MINIGAME_SFX_JUMP2,
    PC_MINIGAME_SFX_JUMP3,
    PC_MINIGAME_SFX_JUMP4,
    PC_MINIGAME_SFX_LAND,
    PC_MINIGAME_SFX_RUN,
    PC_MINIGAME_SFX_LONG_GLITCH,
    PC_MINIGAME_SFX_INSUIT,
    PC_MINIGAME_SFX_LAUGH,
    PC_MINIGAME_SFX_SCARE,
    PC_MINIGAME_SFX_STOP,
    PC_MINIGAME_SFX_CRAZY_GARBLE,
    PC_MINIGAME_SFX_COUNT
} PcMinigameSfx;

void pc_minigame_sfx_play(PcMinigameSfx cue, float volume, bool loop);
void pc_minigame_sfx_stop(PcMinigameSfx cue);
void pc_minigame_sfx_stop_all(void);
void pc_minigame_sfx_shutdown(void);
