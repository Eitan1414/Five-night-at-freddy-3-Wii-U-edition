#pragma once

#include <stddef.h>
#include "renderer/texture.h"

typedef enum FollowMePcEnvSprite {
    FOLLOW_ME_ENV_TABLE_CLEAN = 0,
    FOLLOW_ME_ENV_TABLE_SPIKY_A = 1,
    FOLLOW_ME_ENV_TABLE_SPIKY_B = 2,
    FOLLOW_ME_ENV_TABLE_STAINED = 3,
    FOLLOW_ME_ENV_TABLE_FADED = 4,
    FOLLOW_ME_ENV_CURTAIN = 5,
    FOLLOW_ME_ENV_POSTERS = 6,
    FOLLOW_ME_ENV_DESK_FAN = 7,
    FOLLOW_ME_ENV_PARTY_TABLE = 8,
    FOLLOW_ME_ENV_DOOR_WALL = 9,
    FOLLOW_ME_ENV_STAGE = 10,
    FOLLOW_ME_ENV_ARCADE = 11,
    FOLLOW_ME_ENV_KEYPAD = 12,
    FOLLOW_ME_ENV_FOLLOW_TEXT = 13,
    FOLLOW_ME_ENV_WASD = 14,
    FOLLOW_ME_ENV_ERR = 15,
    FOLLOW_ME_ENV_SPRITE_COUNT = 16
} FollowMePcEnvSprite;

const TextureRle *follow_me_pc_env_sprite(FollowMePcEnvSprite sprite);
