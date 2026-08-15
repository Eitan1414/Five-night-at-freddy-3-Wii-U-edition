#include "assets/monitor_fidelity_assets.h"

/*
 * Stable PC camera fidelity layer for real Wii U hardware.
 *
 * The compact compatibility textures are generated from the same verified PC
 * General Sprites bank as the high-resolution tiled textures, but use a single
 * TextureRle per feed. Keeping the night monitor on this bank avoids the
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
/* Every retail-PC camera/Springtrap/full-Phantom composite in General Sprites
 * is 825x650. 432x340 preserves that source aspect (1.2706 vs 1.2692) instead
 * of the old 432x240 Wii U stretch. The original map then naturally overlays
 * the lower-right portion of the video feed. */
#define PC_CAMERA_FEED_H 340
#define PC_CAMERA_SOURCE_W 825
#define PC_CAMERA_SOURCE_H 650

/* fivenights3-94.mfa camera-map object `map` is image 532/574, 430x400,
 * instance (1770,560), hotspot (211,200). Its source top-left is therefore
 * (1559,360). The ten 60x40 camera buttons use image 81 normally and image 82
 * while selected. Image 82 is the exact lime PC highlight supplied below by
 * monitor_fidelity_assets; its hotspot is (29,19). */
#define PC_CAMERA_MAP_X 591
#define PC_CAMERA_MAP_Y 177
#define PC_CAMERA_MAP_W 252
#define PC_CAMERA_MAP_H 235
#define PC_CAMERA_MAP_SOURCE_X 1559
#define PC_CAMERA_MAP_SOURCE_Y 360
#define PC_CAMERA_MAP_SOURCE_W 430
#define PC_CAMERA_MAP_SOURCE_H 400

static const int kPcCameraNodeInstanceX[CAMERA_COUNT] = {
    1644, 1859, 1940, 1940, 1776, 1637, 1637, 1742, 1808, 1916
};
static const int kPcCameraNodeInstanceY[CAMERA_COUNT] = {
    676, 648, 606, 541, 557, 565, 499, 477, 434, 470
};

/* Exact MFA instances for image-bank handles 83..92 (CAM 01..CAM 10 text).
 * The first nine label images have hotspot (-2,-2); CAM10 uses (-1,-2). */
static const int kPcCameraLabelInstanceX[CAMERA_COUNT] = {
    1619, 1835, 1916, 1917, 1753, 1614, 1614, 1719, 1785, 1894
};
static const int kPcCameraLabelInstanceY[CAMERA_COUNT] = {
    662, 634, 592, 527, 543, 551, 485, 463, 421, 456
};
static const int kPcCameraLabelHotspotX[CAMERA_COUNT] = {
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -1
};

typedef struct PcShadowCupcakeLayout {
    int camera;
    uint8_t collected_bit;
    int source_x;
    int source_y;
    int hotspot_x;
    int hotspot_y;
    int source_w;
    int source_h;
    const TextureRle *texture;
} PcShadowCupcakeLayout;

/* fivenights3-94.mfa group 741 creates all four Shadow Cupcake actives as
 * children of "00 anchor 4". Their Create parameters contain the real monitor
 * workspace coordinates. The monitor workspace begins at X=1024, so the
 * camera-source X values below are the exact Create X minus 1024.
 *
 * cupcake 3 / CAM02: Create (1552,426), image 959, hotspot (15,30), 31x51
 * cupcake 1 / CAM03: Create (1678,360), image 999, hotspot (30,59), 61x99
 * cupcake 4 / CAM04: Create (1456,430), image 1000, hotspot (20,39), 41x67
 * cupcake 2 / CAM06: Create (1440,314), image 999, hotspot (30,59), 61x99
 *
 * collected_bit follows the existing PC trigger mask ordering CAM02/03/04/06.
 */
static const PcShadowCupcakeLayout kPcShadowCupcakes[4] = {
    {1, 0x01u, 528, 426, 15, 30, 31, 51,
     &gPcCompatShadowCupcakeSmallTexture},
    {2, 0x02u, 654, 360, 30, 59, 61, 99,
     &gPcCompatShadowCupcakeLargeTexture},
    {3, 0x04u, 432, 430, 20, 39, 41, 67,
     &gPcCompatShadowCupcakeMediumTexture},
    {5, 0x08u, 416, 314, 30, 59, 61, 99,
     &gPcCompatShadowCupcakeLargeTexture},
};

static const PcShadowCupcakeLayout *pc_shadow_cupcake_for_camera(int camera)
{
    for (unsigned index = 0u; index < 4u; ++index) {
        if (kPcShadowCupcakes[index].camera == camera)
            return &kPcShadowCupcakes[index];
    }
    return NULL;
}

static void pc_shadow_cupcake_drc_rect(const PcShadowCupcakeLayout *cupcake,
                                       int *x, int *y, int *w, int *h)
{
    if (cupcake == NULL) return;
    const int left = cupcake->source_x - cupcake->hotspot_x;
    const int top = cupcake->source_y - cupcake->hotspot_y;
    *x = PC_CAMERA_FEED_X + (left * PC_CAMERA_FEED_W) / PC_CAMERA_SOURCE_W;
    *y = PC_CAMERA_FEED_Y + (top * PC_CAMERA_FEED_H) / PC_CAMERA_SOURCE_H;
    *w = (cupcake->source_w * PC_CAMERA_FEED_W + PC_CAMERA_SOURCE_W - 1) /
         PC_CAMERA_SOURCE_W;
    *h = (cupcake->source_h * PC_CAMERA_FEED_H + PC_CAMERA_SOURCE_H - 1) /
         PC_CAMERA_SOURCE_H;
    if (*w < 1) *w = 1;
    if (*h < 1) *h = 1;
}

static bool pc_shadow_cupcake_is_visible(const Game *game,
                                         const PcShadowCupcakeLayout *cupcake)
{
    return cupcake != NULL && game->night == 3 &&
           game->selected_camera == cupcake->camera &&
           (sSecretTriggers.cupcake_mask & cupcake->collected_bit) == 0u;
}

static void pc_camera_draw_shadow_cupcake(const Game *game)
{
    const PcShadowCupcakeLayout *cupcake =
        pc_shadow_cupcake_for_camera(game->selected_camera);
    if (!pc_shadow_cupcake_is_visible(game, cupcake)) return;

    int x = 0, y = 0, w = 0, h = 0;
    pc_shadow_cupcake_drc_rect(cupcake, &x, &y, &w, &h);
    texture_draw_rle(sOriginalUiPanelTargets, x, y, w, h, cupcake->texture);
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
            /* MFA object BBpeek uses General Sprites image 70. The extracted
             * PNG is already an 825x650 transparent canvas, so its right-side
             * placement is encoded in the source image itself. Stretching a
             * cropped rectangle here used to move/resize BB twice. */
            texture_draw_rle(sOriginalUiPanelTargets,
                             PC_CAMERA_FEED_X,
                             PC_CAMERA_FEED_Y,
                             PC_CAMERA_FEED_W,
                             PC_CAMERA_FEED_H,
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
                           PC_CAMERA_FEED_Y + 156,
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

    /* The Shadow Cupcakes are separate Active objects in the PC MFA, created
     * over the camera feed before the monitor UI. Keep them independent from
     * camera backgrounds so collecting one removes only that object. */
    pc_camera_draw_shadow_cupcake(game);

    /* Chica/Mangle/Puppet are exact PC full-camera composites. BBpeek also
     * keeps its original 825x650 transparent coordinate canvas. All four use
     * compact PC-derived RLE textures so the real Wii U never touches the
     * unstable tiled camera renderer. */
    if (phantom != PHANTOM_NONE)
        pc_camera_draw_compact_phantom(game, phantom);
}

static int pc_camera_map_x(int source_x)
{
    return PC_CAMERA_MAP_X +
        ((source_x - PC_CAMERA_MAP_SOURCE_X) * PC_CAMERA_MAP_W +
         PC_CAMERA_MAP_SOURCE_W / 2) / PC_CAMERA_MAP_SOURCE_W;
}

static int pc_camera_map_y(int source_y)
{
    return PC_CAMERA_MAP_Y +
        ((source_y - PC_CAMERA_MAP_SOURCE_Y) * PC_CAMERA_MAP_H +
         PC_CAMERA_MAP_SOURCE_H / 2) / PC_CAMERA_MAP_SOURCE_H;
}

static int pc_camera_map_w(int source_w)
{
    return (source_w * PC_CAMERA_MAP_W + PC_CAMERA_MAP_SOURCE_W / 2) /
           PC_CAMERA_MAP_SOURCE_W;
}

static int pc_camera_map_h(int source_h)
{
    return (source_h * PC_CAMERA_MAP_H + PC_CAMERA_MAP_SOURCE_H / 2) /
           PC_CAMERA_MAP_SOURCE_H;
}

static void pc_camera_draw_exact_selection(const Game *game)
{
    const int camera = game->selected_camera;
    if (camera < 0 || camera >= CAMERA_COUNT) return;

    const TextureRle *selected =
        monitor_fidelity_texture(MONITOR_FIDELITY_NODE_SELECTED);
    const TextureRle *label = monitor_fidelity_texture(
        (MonitorFidelityTextureId)(MONITOR_FIDELITY_CAM01_LABEL + camera));
    if (selected == NULL || label == NULL) return;

    /* Node object image 82: 60x40, hotspot (29,19). */
    const int node_left = kPcCameraNodeInstanceX[camera] - 29;
    const int node_top = kPcCameraNodeInstanceY[camera] - 19;
    texture_draw_rle(sOriginalUiPanelTargets,
                     pc_camera_map_x(node_left),
                     pc_camera_map_y(node_top),
                     pc_camera_map_w(60),
                     pc_camera_map_h(40),
                     selected);

    /* CAM01..09 labels: 29x25 / hotspot(-2,-2). CAM10: 30x25 /
     * hotspot(-1,-2). Repaint only the selected label because image 82 is
     * opaque by design and therefore correctly covers the idle gray button. */
    const int label_width = camera == 9 ? 30 : 29;
    const int label_left =
        kPcCameraLabelInstanceX[camera] - kPcCameraLabelHotspotX[camera];
    const int label_top = kPcCameraLabelInstanceY[camera] + 2;
    texture_draw_rle(sOriginalUiPanelTargets,
                     pc_camera_map_x(label_left),
                     pc_camera_map_y(label_top),
                     pc_camera_map_w(label_width),
                     pc_camera_map_h(25),
                     label);
}

static void pc_camera_draw_map(const Game *game)
{
    const OriginalUiTextureId map_id =
        ((game->night_frames / 12u) & 1u)
            ? ORIGINAL_UI_CAMERA_MAP_ALT
            : ORIGINAL_UI_CAMERA_MAP_BASE;

    original_ui_draw_texture(map_id, sOriginalUiPanelTargets,
                             PC_CAMERA_MAP_X, PC_CAMERA_MAP_Y,
                             PC_CAMERA_MAP_W, PC_CAMERA_MAP_H);

    /* The old port drew a synthetic 42x20 lime outline. Retail PC instead
     * switches the selected camera button to image 82 (60x40) and leaves the
     * CAM label above it. Recreate that exact Clickteam object composition. */
    pc_camera_draw_exact_selection(game);
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

    /* Camera-switch static belongs to the VIDEO FEED only. The PC map and
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
