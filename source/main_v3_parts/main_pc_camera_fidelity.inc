/* Final PC camera fidelity layer.
 *
 * The legacy monitor renderer still adds Wii U reconstruction effects after
 * drawing a camera: per-Springtrap scan bars, generic 1px scanlines and old
 * cut-out placement. The character bridge owns exact full-frame PC composites
 * for every Springtrap camera state and the verified full-frame Phantom camera
 * events, so redraw only the 532x295 viewport after the legacy/UI pass. This
 * keeps the monitor controls/map intact while making the actual feed PC art.
 *
 * fivenights3-94.mfa also uses Springtrap's normal camera composites for the
 * ventilation hallucinations: several cameras may show a fake Springtrap at
 * once, except the camera containing the real one. The maintenance fidelity
 * layer below this include owns the timer/random mask; these declarations live
 * here because the camera renderer is compiled earlier in the unified main TU.
 */
static uint16_t sPcHallucinationCameraMask = 0u;
static uint32_t sPcHallucinationFrames = 0u;

static void pc_camera_fidelity_draw_overlay_phantom(const Game *game,
                                                    PhantomId phantom)
{
    (void) game;
    if (phantom != PHANTOM_BALLOON_BOY)
        return;

    /* MFA object "BBpeek" uses General Sprites image 70 (825x650), not a
     * jumpscare frame. Preserve its source aspect ratio instead of the old
     * square Wii U reconstruction. Its exact Clickteam position is relative
     * to the monitor/camera-screen anchor, so keep the established viewport
     * anchor while using the correct PC image and ratio. */
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     174, 82, 300, 236,
                     &gPcPhantomBBTexture);
}

static bool pc_camera_fidelity_has_hallucination(int camera)
{
    if (camera < 0 || camera >= CAMERA_COUNT ||
        sPcHallucinationFrames == 0u) {
        return false;
    }
    return (sPcHallucinationCameraMask &
            (uint16_t)(1u << (uint32_t)camera)) != 0u;
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
    const bool real_springtrap =
        springtrap_ai_is_on_camera(&game->springtrap, camera);

    /* Real Springtrap always wins over Hallucinationtrap. The generated mask
     * already excludes his true camera, but keep this ordering as an explicit
     * guard against stale state during a move on the same frame. */
    if (real_springtrap || pc_camera_fidelity_has_hallucination(camera))
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
