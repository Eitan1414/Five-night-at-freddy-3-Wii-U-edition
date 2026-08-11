/* Final PC camera fidelity layer.
 *
 * The legacy monitor renderer still adds Wii U reconstruction effects after
 * drawing a camera: per-Springtrap scan bars, generic 1px scanlines and old
 * cut-out placement.  The character bridge already owns exact full-frame PC
 * composites for every Springtrap camera state, so redraw only the 532x295
 * viewport after the legacy/UI pass.  This keeps the monitor controls/map
 * intact while making the actual feed the original PC composite again.
 */

static void pc_camera_fidelity_draw_overlay_phantom(const Game *game,
                                                    PhantomId phantom)
{
    switch (phantom) {
        case PHANTOM_BALLOON_BOY: {
            const int bob = phantom_bob(game->night_frames, 24u, 7);
            if (game->phantoms.bb_seen_frames >= 22u) {
                texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                                 138 - bob, 52 - bob,
                                 334 + bob * 2, 314 + bob * 2,
                                 &gPcPhantomBBTexture);
            } else {
                texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                                 174 + bob / 2, 82 - bob,
                                 262, 262,
                                 &gPcPhantomBBTexture);
            }
            break;
        }
        case PHANTOM_PUPPET: {
            const int flicker =
                (int)((game->night_frames / 2u +
                       (uint32_t)phantom) % 5u) - 2;
            texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                             205 + flicker, 45 - flicker,
                             205, 320,
                             &gPcPhantomPuppetTexture);
            break;
        }
        default:
            break;
    }
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

    const PcTiledTexture *feed = NULL;
    if (springtrap_ai_is_on_camera(&game->springtrap, camera))
        feed = pc_springtrap_camera_texture(camera);
    if (feed == NULL)
        feed = pc_core_camera_texture(camera);

    if (feed != NULL) {
        pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                              28, 65, 532, 295, feed);
    }

    /* BB/Puppet are transparent PC actors rather than verified full-frame
     * composites in the current asset set. Reapply only those after restoring
     * the exact base/Springtrap feed. */
    pc_camera_fidelity_draw_overlay_phantom(game, phantom);
}

static void original_ui_draw_camera_feed(const Game *game)
{
    original_ui_draw_camera_feed_v2(game);
    pc_camera_fidelity_redraw_viewport(game);
}
