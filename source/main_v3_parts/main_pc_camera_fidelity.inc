/*
 * Stable PC camera fidelity layer for real Wii U hardware.
 *
 * The compact compatibility textures are generated from the same verified PC
 * General Sprites bank as the high-resolution tiled textures, but use a single
 * TextureRle per feed.  Keeping the night monitor on this bank avoids the
 * camera-dependent crashes seen with the tiled path while preserving PC art.
 */
static uint16_t sPcHallucinationCameraMask = 0u;
static uint32_t sPcHallucinationFrames = 0u;

#define PC_MONITOR_PANEL_X 174
#define PC_MONITOR_PANEL_Y 38
#define PC_MONITOR_PANEL_W 674
#define PC_MONITOR_PANEL_H 423
#define PC_CAMERA_FEED_X 196
#define PC_CAMERA_FEED_Y 70
#define PC_CAMERA_FEED_W 432
#define PC_CAMERA_FEED_H 240

static bool pc_camera_fidelity_has_hallucination(int camera)
{
    if (camera < 0 || camera >= CAMERA_COUNT ||
        sPcHallucinationFrames == 0u) {
        return false;
    }
    return (sPcHallucinationCameraMask &
            (uint16_t)(1u << (uint32_t)camera)) != 0u;
}

static void pc_camera_draw_static(const Game *game)
{
    const int frame = (int)((game->night_frames / 2u) % 3u);
    const OriginalUiTextureId id = (OriginalUiTextureId)
        (ORIGINAL_UI_STATIC_0 + frame);
    original_ui_draw_texture(id, sOriginalUiPanelTargets,
                             PC_CAMERA_FEED_X, PC_CAMERA_FEED_Y,
                             PC_CAMERA_FEED_W, PC_CAMERA_FEED_H);
}

static void pc_camera_draw_compact_phantom(const Game *game,
                                           PhantomId phantom)
{
    switch (phantom) {
        case PHANTOM_BALLOON_BOY:
            /* BBpeek is a foreground object, not a full camera composite. */
            texture_draw_rle(sOriginalUiPanelTargets,
                             PC_CAMERA_FEED_X + 104,
                             PC_CAMERA_FEED_Y + 18,
                             245, 205,
                             &gPhantomBBCameraTexture);
            break;
        case PHANTOM_CHICA:
            /* MFA full-camera composite, General Sprites image 387. */
            texture_draw_rle(sOriginalUiPanelTargets,
                             PC_CAMERA_FEED_X,
                             PC_CAMERA_FEED_Y,
                             PC_CAMERA_FEED_W,
                             PC_CAMERA_FEED_H,
                             &gPhantomChicaCameraTexture);
            break;
        case PHANTOM_MANGLE:
            /* MFA full-camera composite, General Sprites image 38. */
            texture_draw_rle(sOriginalUiPanelTargets,
                             PC_CAMERA_FEED_X,
                             PC_CAMERA_FEED_Y,
                             PC_CAMERA_FEED_W,
                             PC_CAMERA_FEED_H,
                             &gPhantomMangleCameraTexture);
            break;
        case PHANTOM_PUPPET:
            /* MFA full-camera composite, General Sprites image 298. */
            texture_draw_rle(sOriginalUiPanelTargets,
                             PC_CAMERA_FEED_X,
                             PC_CAMERA_FEED_Y,
                             PC_CAMERA_FEED_W,
                             PC_CAMERA_FEED_H,
                             &gPhantomPuppetCameraTexture);
            break;
        default:
            break;
    }
    (void)game;
}

static void pc_camera_fidelity_redraw_viewport(const Game *game)
{
    const int camera = game->selected_camera;
    if (camera < 0 || camera >= CAMERA_COUNT)
        return;

    graphics_draw_rect(sOriginalUiPanelTargets,
                       PC_CAMERA_FEED_X, PC_CAMERA_FEED_Y,
                       PC_CAMERA_FEED_W, PC_CAMERA_FEED_H,
                       COLOUR_BLACK);

    if (game->camera_failed) {
        pc_camera_draw_static(game);
        graphics_draw_text(sOriginalUiPanelTargets,
                           PC_CAMERA_FEED_X + 118,
                           PC_CAMERA_FEED_Y + 106,
                           3, "VIDEO ERROR", COLOUR_RED);
        return;
    }

    const PhantomId phantom =
        phantoms_camera_overlay(&game->phantoms, camera);

    const bool springtrap_enabled =
        night_config(game->night)->springtrap_enabled;
    const bool real_springtrap = springtrap_enabled &&
        springtrap_ai_is_on_camera(&game->springtrap, camera);
    const bool hallucination = springtrap_enabled &&
        pc_camera_fidelity_has_hallucination(camera);

    const TextureRle *feed = NULL;
    if (real_springtrap || hallucination)
        feed = gOriginalCameraSpringtrapTextures[camera];
    if (feed == NULL)
        feed = kCameraTextures[camera];

    if (feed != NULL) {
        texture_draw_rle(sOriginalUiPanelTargets,
                         PC_CAMERA_FEED_X, PC_CAMERA_FEED_Y,
                         PC_CAMERA_FEED_W, PC_CAMERA_FEED_H,
                         feed);
    }

    /* Chica/Mangle/Puppet are exact PC full-camera composites; BB remains a
     * foreground object. All four use compact PC-derived RLE textures so the
     * real Wii U never touches the unstable tiled camera renderer. */
    if (phantom != PHANTOM_NONE)
        pc_camera_draw_compact_phantom(game, phantom);
}

static void pc_camera_draw_map(const Game *game)
{
    const OriginalUiTextureId map_id =
        ((game->night_frames / 12u) & 1u)
            ? ORIGINAL_UI_CAMERA_MAP_ALT
            : ORIGINAL_UI_CAMERA_MAP_BASE;

    original_ui_draw_texture(map_id, sOriginalUiPanelTargets,
                             591, 177, 252, 235);
    original_ui_draw_camera_nodes(game);
    original_ui_draw_radar(game);
}

static void original_ui_draw_camera_feed(const Game *game)
{
    /* The TV office is already drawn by draw_office_tv(). The GamePad needs its
     * own office copy only in mirrored mode. */
    if ((sOriginalUiPanelTargets & GRAPHICS_TARGET_GAMEPAD) != 0u)
        pc_draw_gamepad_office_background(game);

    graphics_draw_rect(sOriginalUiPanelTargets,
                       PC_MONITOR_PANEL_X, PC_MONITOR_PANEL_Y,
                       PC_MONITOR_PANEL_W, PC_MONITOR_PANEL_H,
                       GRAPHICS_RGB(0, 2, 1));
    graphics_draw_frame(sOriginalUiPanelTargets,
                        PC_MONITOR_PANEL_X, PC_MONITOR_PANEL_Y,
                        PC_MONITOR_PANEL_W, PC_MONITOR_PANEL_H,
                        2, GRAPHICS_RGB(52, 72, 52));

    pc_camera_fidelity_redraw_viewport(game);

    /* Camera-switch static belongs to the VIDEO FEED only.  The PC map and
     * controls remain readable during the burst. */
    if (!game->camera_failed && sOriginalUi.static_flash_frames > 0u)
        pc_camera_draw_static(game);

    pc_camera_draw_map(game);
    monitor_v2_draw_controls();

    if (sFinishing.phone_active) {
        original_ui_draw_texture(ORIGINAL_UI_MUTE_CALL,
                                 sOriginalUiPanelTargets,
                                 306, 100, 94, 22);
    }
}
