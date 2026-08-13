static void draw_authentic_office_fan(const Game *game,
                                      int texture_x,
                                      uint32_t targets)
{
    const int frame = (int) ((game->night_frames / 4u) % 3u);
    const TextureRle *fan = office_assets_texture(
        (OfficeAssetTextureId) (OFFICE_ASSET_FAN_0 + frame));
    if (fan == NULL) return;

    /* Position is anchored in the original office panorama. */
    const int fan_x = texture_x + 812;
    const int fan_y = 178;
    texture_draw_rle(targets, fan_x, fan_y, 120, 97, fan);
}

static void draw_authentic_office_targets(const Game *game, uint32_t targets)
{
    const int texture_x = -133 + game->office_pan;
    const uint32_t previous_targets = sPcOfficeTargets;
    sPcOfficeTargets = targets;

    graphics_clear(targets, COLOUR_BLACK);

    /*
     * Use the compact Wii U-safe PC texture. It is generated from the same PC
     * General Sprites office image 203 as the former tiled panorama, but stays
     * in one validated TextureRle object and avoids the black-screen regression
     * observed on real Wii U hardware.
     */
    texture_draw_rle(targets,
                     texture_x, 24, 1120, 430,
                     &gOfficeTexture);

    draw_springtrap_office(game);
    draw_authentic_office_fan(game, texture_x, targets);
    draw_phantom_office(game);

    /*
     * Do not add the old Wii U prototype scanline grid here. The retail PC
     * office is the source artwork itself; camera/static effects and the
     * ventilation blackout are rendered by their dedicated PC/MFA layers.
     */

    /* Keep only the gameplay HUD information while the exact PC `night`, `AM`
     * and counter sprites are wired from General Sprites in the next asset
     * pass. Unlike the former prototype, these labels have no invented frame,
     * status box or green tint. */
    char night_text[24], clock_text[16];
    snprintf(night_text, sizeof(night_text), "night %d", game->night);
    snprintf(clock_text, sizeof(clock_text), "%d AM", game->hour);
    graphics_draw_text(targets, 741, 35, 1, clock_text, COLOUR_WHITE);
    graphics_draw_text(targets, 741, 54, 1, night_text, COLOUR_WHITE);

    /* Ventilation failure presentation is owned by pc_draw_ventilation_overlay:
     * red alarm pulse + MFA B-counter blackout. Do not draw the Wii U-only
     * `VENT ERROR` banner over the office. */

    sPcOfficeTargets = previous_targets;
}

static void draw_authentic_office_tv(const Game *game)
{
    draw_authentic_office_targets(game, GRAPHICS_TARGET_TV);
}
