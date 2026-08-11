/* High-resolution PC background bridge for the original monitor UI. */
static void pc_original_ui_texture_draw_rle(uint32_t targets,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            const TextureRle *texture)
{
    /*
     * Only replace a monitor background when the converter has a verified PC
     * source for that exact camera. Character/UI textures and unverified
     * cameras still use the existing path unchanged.
     */
    for (int camera_index = 0; camera_index < 10; ++camera_index) {
        if (texture == kCameraTextures[camera_index]) {
            const PcTiledTexture *pc_texture =
                pc_core_camera_texture(camera_index);
            if (pc_texture != NULL) {
                pc_tiled_texture_draw(targets, x, y, width, height, pc_texture);
                return;
            }
            break;
        }
    }

    texture_draw_rle(targets, x, y, width, height, texture);
}
