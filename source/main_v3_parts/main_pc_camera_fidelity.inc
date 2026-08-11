/* Final PC camera fidelity layer.
 *
 * The legacy monitor renderer still adds Wii U reconstruction effects after
 * drawing a camera: per-Springtrap scan bars, generic 1px scanlines and old
 * cut-out placement. The character bridge owns exact full-frame PC composites
 * for every Springtrap camera state and the verified full-frame Phantom camera
 * events, so redraw only the 532x295 viewport after the legacy/UI pass. This
 * keeps the monitor controls/map intact while making the actual feed PC art.
 */

static void pc_camera_fidelity_draw_overlay_phantom(const Game *game,
                                                    PhantomId phantom)
{
    (void) game;
    if (phantom != PHANTOM_BALLOON_BOY)
        return;

    /* General Sprites image 338 is the verified PC BB camera actor. Keep its
     * presentation stable; the previous bob + artificial zoom were Wii U
     * reconstruction effects that do not exist in the supplied PC object art. */
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     174, 82, 262, 262,
                     &gPcPhantomBBTexture);
}

static void pc_camera_fidelity_redraw_viewport(const Game *game)
{
    if (game->camera_failed || sOriginalUi.static_flash_frames > 0u)
        return;

    const int camera = game->selected_camera;
    if (camera < 0 || camera >= CAMERA_COUNT)
        return;

    const PhantomId phantom =
        phantoms_camera_overlay(&game->phantoms, camera);

    /* Full-frame phantom composites take precedence, matching the original
     * selected-object camera events. */
    if (phantom == PHANTOM_CHICA) {
        pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                              28, 65, 532, 295,
                              &gPcPhantomChicaCameraTexture);
        return;
    }
    if (phantom == PHANTOM_MANGLE) {
        pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                              28, 65, 532, 295,
                              &gPcPhantomMangleCameraTexture);
        return;
    }
    if (phantom == PHANTOM_PUPPET) {
        pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                              28, 65, 532, 295,
                              &gPcPhantomPuppetCameraTexture);
        return;
    }

    const PcTiledTexture *feed = NULL;
    if (springtrap_ai_is_on_camera(&game->springtrap, camera))
        feed = pc_springtrap_camera_texture(camera);
    if (feed == NULL)
        feed = pc_core_camera_texture(camera);

    if (feed != NULL) {
        pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                              28, 65, 532, 295, feed);
    }

    /* BB is the remaining transparent camera actor in the verified PC set. */
    pc_camera_fidelity_draw_overlay_phantom(game, phantom);
}

static void original_ui_draw_camera_feed(const Game *game)
{
    original_ui_draw_camera_feed_v2(game);
    pc_camera_fidelity_redraw_viewport(game);
}
