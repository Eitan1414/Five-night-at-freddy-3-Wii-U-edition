/* High-resolution PC background bridge for the original monitor UI. */
static void pc_original_ui_texture_draw_rle(uint32_t targets,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            const TextureRle *texture)
{
    if (texture == kCameraTextures[0]) {
        pc_tiled_texture_draw(targets, x, y, width, height,
                              &gPcCamera01Texture);
        return;
    }
    if (texture == kCameraTextures[1]) {
        pc_tiled_texture_draw(targets, x, y, width, height,
                              &gPcCamera02Texture);
        return;
    }

    /* Non-migrated cameras and all UI/character textures stay untouched. */
    texture_draw_rle(targets, x, y, width, height, texture);
}
