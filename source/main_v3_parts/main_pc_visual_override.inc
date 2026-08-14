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

/* Implemented later by main_authentic_office.inc after all character helpers
 * are available. Keeping one office compositor is what makes IDENTICAL truly
 * identical instead of copying only the panorama background to the DRC. */
static void draw_authentic_office_targets(const Game *game, uint32_t targets);

static void pc_draw_gamepad_office_background(const Game *game)
{
    draw_authentic_office_targets(game, GRAPHICS_TARGET_GAMEPAD);
}
