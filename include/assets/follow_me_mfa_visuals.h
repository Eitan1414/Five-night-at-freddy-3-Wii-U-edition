#pragma once

#include "renderer/texture.h"
#include <stdint.h>

typedef struct FollowMeMfaTexture {
    TextureRle texture;
    int16_t hotspot_x;
    int16_t hotspot_y;
} FollowMeMfaTexture;

extern const FollowMeMfaTexture gFollowMfaChecker;
extern const FollowMeMfaTexture gFollowMfaBackdropWide;
extern const FollowMeMfaTexture gFollowMfaBackdropTall;
extern const FollowMeMfaTexture gFollowMfaStage;
extern const FollowMeMfaTexture gFollowMfaChicaDecor;
extern const FollowMeMfaTexture gFollowMfaBonnieDecor;
extern const FollowMeMfaTexture gFollowMfaArcade;
extern const FollowMeMfaTexture gFollowMfaTrash;
extern const FollowMeMfaTexture gFollowMfaPartyTable0;
extern const FollowMeMfaTexture gFollowMfaPartyTable12;
extern const FollowMeMfaTexture gFollowMfaPartyTable13;
extern const FollowMeMfaTexture gFollowMfaPartyTable14;
extern const FollowMeMfaTexture gFollowMfaPartyTable15;
extern const FollowMeMfaTexture gFollowMfaHallway;
extern const FollowMeMfaTexture gFollowMfaCurtain;
extern const FollowMeMfaTexture gFollowMfaRaindrop;
extern const FollowMeMfaTexture gFollowMfaBlood;
extern const FollowMeMfaTexture gFollowMfaErr;
extern const FollowMeMfaTexture gFollowMfaFollowText;
extern const FollowMeMfaTexture gFollowMfaControls;
extern const FollowMeMfaTexture gFollowMfaBlocked;
