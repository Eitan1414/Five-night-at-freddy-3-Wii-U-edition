static void draw_authentic_office_fan(const Game *game, int texture_x)
{
    const int frame = (int) ((game->night_frames / 4u) % 3u);
    const TextureRle *fan = office_assets_texture(
        (OfficeAssetTextureId) (OFFICE_ASSET_FAN_0 + frame));
    if (fan == NULL) return;

    /* Position is anchored in the original office panorama. */
    const int fan_x = texture_x + 812;
    const int fan_y = 178;
    texture_draw_rle(GRAPHICS_TARGET_TV, fan_x, fan_y, 120, 97, fan);
}

static void draw_authentic_office_tv(const Game *game)
{
    const int texture_x = -133 + game->office_pan;
    graphics_clear(GRAPHICS_TARGET_TV, COLOUR_BLACK);

    /*
     * The supplied PC General Sprites panorama (203.png) is now the source of
     * truth for the office background. It is generated at the exact 1120x430
     * render size and split into row-RLE tiles so every tile stays within the
     * renderer's 16-bit row-offset format.
     */
    pc_tiled_texture_draw(GRAPHICS_TARGET_TV,
                          texture_x, 24, 1120, 430,
                          &gPcOfficeTexture);

    draw_springtrap_office(game);
    draw_authentic_office_fan(game, texture_x);
    draw_phantom_office(game);

    for (int y = 30; y < 454; y += 12)
        graphics_draw_rect(GRAPHICS_TARGET_TV, 0, y, 854, 1,
                           GRAPHICS_RGB(4, 8, 5));

    char night_text[24], clock_text[16];
    snprintf(night_text, sizeof(night_text), "NIGHT %d", game->night);
    snprintf(clock_text, sizeof(clock_text), "%d AM", game->hour);
    graphics_draw_text(GRAPHICS_TARGET_TV, 18, 5, 2, night_text,
                       GRAPHICS_RGB(166, 191, 151));
    graphics_draw_rect(GRAPHICS_TARGET_TV, 718, 4, 118, 36,
                       GRAPHICS_RGB(1, 5, 2));
    graphics_draw_frame(GRAPHICS_TARGET_TV, 718, 4, 118, 36, 2,
                        GRAPHICS_RGB(65, 103, 63));
    graphics_draw_text(GRAPHICS_TARGET_TV, 744, 16, 2, clock_text,
                       COLOUR_WHITE);
    if (game->ventilation_failed) {
        graphics_draw_frame(GRAPHICS_TARGET_TV, 555, 9, 157, 34, 3,
                            COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_TV, 572, 20, 1,
                           "VENT ERROR", COLOUR_RED);
    }
}
