/* High-resolution PC visual bridge for the original monitor UI. */
static void pc_original_ui_texture_draw_rle(uint32_t targets,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            const TextureRle *texture)
{
    /* Replace every verified monitor background with its PC composite. */
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

    /*
     * The old authentic-monitor layer placed PSX Springtrap cut-outs over a
     * camera background. General Sprites contains the real PC composite for
     * every CAM state, so replace the complete feed instead of layering the
     * lower-resolution PSX sprite.
     */
    for (int camera_index = 0; camera_index < 10; ++camera_index) {
        if (texture == gOriginalCameraSpringtrapTextures[camera_index]) {
            const PcTiledTexture *pc_texture =
                pc_springtrap_camera_texture(camera_index);
            if (pc_texture != NULL) {
                pc_tiled_texture_draw(targets,
                                      28, 65, 532, 295,
                                      pc_texture);
                return;
            }
            break;
        }
    }

    /* Exact PC full-frame Phantom camera appearances. */
    if (texture == &gPhantomChicaCameraTexture) {
        pc_tiled_texture_draw(targets,
                              28, 65, 532, 295,
                              &gPcPhantomChicaCameraTexture);
        return;
    }
    if (texture == &gPhantomMangleCameraTexture) {
        pc_tiled_texture_draw(targets,
                              28, 65, 532, 295,
                              &gPcPhantomMangleCameraTexture);
        return;
    }

    /* Transparent PC Phantom objects used by monitor hallucinations. */
    if (texture == &gPhantomBBCameraTexture ||
        texture == &gPhantomBBOfficeTexture) {
        texture_draw_rle(targets, x, y, width, height,
                         &gPcPhantomBBTexture);
        return;
    }
    if (texture == &gPhantomPuppetCameraTexture ||
        texture == &gPhantomPuppetOfficeTexture) {
        texture_draw_rle(targets, x, y, width, height,
                         &gPcPhantomPuppetTexture);
        return;
    }
    if (texture == &gPhantomChicaOfficeTexture) {
        texture_draw_rle(targets, x, y, width, height,
                         &gPcPhantomChicaOfficeTexture);
        return;
    }
    if (texture == &gPhantomFoxyOfficeTexture) {
        texture_draw_rle(targets, x, y, width, height,
                         &gPcPhantomFoxyOfficeTexture);
        return;
    }

    texture_draw_rle(targets, x, y, width, height, texture);
}
