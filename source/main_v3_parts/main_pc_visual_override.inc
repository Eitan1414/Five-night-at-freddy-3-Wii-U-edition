/*
 * Wii U-safe PC visual bridge.
 *
 * The build generates two PC-derived banks: a large tiled bank and a compact
 * single-RLE compatibility bank.  The tiled bank is useful for offline/high-
 * resolution work, but on real Wii U hardware it caused black office frames
 * and camera-specific crashes.  The historical symbols already resolve to
 * compact textures generated from the same PC General Sprites sources, so the
 * final night renderer deliberately uses those stable objects directly.
 */
static void pc_original_ui_texture_draw_rle(uint32_t targets,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            const TextureRle *texture)
{
    texture_draw_rle(targets, x, y, width, height, texture);
}

static void pc_draw_gamepad_office_background(const Game *game)
{
    const int texture_x = -133 + game->office_pan;
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);

    /* gOfficeTexture is mapped by pc_compat_visuals.h to the compact texture
     * generated from PC General Sprites office image 203. */
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     texture_x, 24, 1120, 430,
                     &gOfficeTexture);

    /* Keep the subtle PC monitor/CRT feel without covering the office. */
    for (int y = 30; y < 454; y += 12) {
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           0, y, GRAPHICS_LOGICAL_WIDTH, 1,
                           GRAPHICS_RGB(4, 8, 5));
    }
}
