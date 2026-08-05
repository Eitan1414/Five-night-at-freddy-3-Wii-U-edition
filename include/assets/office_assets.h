#pragma once

#include <stdbool.h>

#include "renderer/texture.h"

typedef enum OfficeAssetTextureId {
    OFFICE_ASSET_NORMAL = 0,
    OFFICE_ASSET_ALERT,
    OFFICE_ASSET_FAN_0,
    OFFICE_ASSET_FAN_1,
    OFFICE_ASSET_FAN_2,
    OFFICE_ASSET_TEXTURE_COUNT,
} OfficeAssetTextureId;

bool office_assets_init(void);
const TextureRle *office_assets_texture(OfficeAssetTextureId id);
