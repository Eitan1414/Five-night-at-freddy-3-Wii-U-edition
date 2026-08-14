typedef struct PcVentMapUiNode {
    int button_x;
    int button_y;
    int button_w;
    int button_h;
    int seal_x;
    int seal_y;
    int seal_w;
    int seal_h;
} PcVentMapUiNode;

/*
 * CAM11-CAM15 are already drawn inside ORIGINAL_UI_VENT_MAP_*; never paint a
 * second text list over the retail-PC artwork.  These rectangles are aligned
 * to the five button and vent-door regions in the original PC vent-map image,
 * scaled into the existing 430x401 Wii U map rectangle at (397,68).
 *
 * They are presentation coordinates, not claimed MFA object coordinates. The
 * gameplay vent indices remain the decoded Springtrap vent graph.  Keeping the
 * distinction explicit prevents a reference-image alignment from accidentally
 * becoming a new source-of-truth for AI movement.
 */
static const PcVentMapUiNode kPcVentMapUiNodes[VENT_COUNT] = {
    /* CAM11 */ {425,  95, 57, 42, 405, 163, 34, 10},
    /* CAM12 */ {494, 210, 62, 40, 437, 240, 34, 10},
    /* CAM13 */ {577, 274, 62, 39, 536, 300, 34, 10},
    /* CAM14 */ {675, 182, 60, 40, 737, 274, 37, 10},
    /* CAM15 */ {698, 325, 58, 42, 654, 379, 38, 10},
};

static void pc_vent_map_draw_runtime_state(const Game *game)
{
    const SpringtrapVent sealing_target = pc_vent_seal_target();

    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const PcVentMapUiNode *node = &kPcVentMapUiNodes[vent];
        const bool selected = vent == (int)game->selected_vent;
        const bool sealing = pc_vent_seal_is_active() &&
            vent == (int)sealing_target;
        const bool sealed = vent == (int)game->sealed_vent;
        const bool occupied = sCompleteCheats.radar &&
            springtrap_ai_is_in_vent(&game->springtrap) &&
            springtrap_ai_current_vent(&game->springtrap)
                == (SpringtrapVent)vent;

        /* Retail PC highlights the currently focused vent-camera button.  A
         * one-pixel Wii U outline leaves the extracted label itself untouched. */
        if (selected || sealing || occupied) {
            uint32_t colour = GRAPHICS_RGB(194, 255, 77);
            if (sealing) colour = COLOUR_WHITE;
            if (occupied) colour = COLOUR_RED;
            graphics_draw_frame(sOriginalUiPanelTargets,
                                node->button_x - 2, node->button_y - 2,
                                node->button_w + 4, node->button_h + 4,
                                2, colour);
        }

        /* The PC vent diagram represents an open door with a green bar and a
         * sealed door with a red bar.  Recolour only that bar instead of the
         * old synthetic `SEALED` word at the side of the screen. */
        if (sealed) {
            graphics_draw_rect(sOriginalUiPanelTargets,
                               node->seal_x, node->seal_y,
                               node->seal_w, node->seal_h,
                               COLOUR_RED);
            graphics_draw_frame(sOriginalUiPanelTargets,
                                node->seal_x, node->seal_y,
                                node->seal_w, node->seal_h,
                                1, GRAPHICS_RGB(112, 20, 20));
        }
    }
}

static void pc_vent_map_draw_sealing_progress(void)
{
    if (!pc_vent_seal_is_active()) return;

    const uint32_t duration = pc_vent_seal_duration_frames();
    const uint32_t frames = pc_vent_seal_progress_frames();
    const int track_x = 206;
    const int track_y = 362;
    const int track_w = 150;
    const int marker_w = 11;
    int marker_x = track_x;
    if (duration > 0u) {
        const uint32_t travel = (uint32_t)(track_w - marker_w);
        marker_x += (int)((frames * travel) / duration);
    }

    /* The retail map displays `sealing vent` beside a moving square while the
     * closure is pending.  Until the exact text object is recovered from the
     * MFA image bank, preserve that behavior with the Wii U bitmap font rather
     * than inventing a different progress bar or instant-close animation. */
    graphics_draw_text(sOriginalUiPanelTargets,
                       206, 338, 1, "SEALING VENT...", COLOUR_WHITE);
    graphics_draw_frame(sOriginalUiPanelTargets,
                        track_x, track_y, track_w, 12, 1,
                        GRAPHICS_RGB(83, 93, 82));
    graphics_draw_rect(sOriginalUiPanelTargets,
                       marker_x, track_y + 1, marker_w, 10,
                       COLOUR_WHITE);
}

static void original_ui_draw_vent_map(const Game *game)
{
    /* The PC panel is an overlay in the office, not a replacement screen.
     * TV already has the office rendered behind the panel. Only draw the
     * separate GamePad office copy when the panel target includes GamePad. */
    if ((sOriginalUiPanelTargets & GRAPHICS_TARGET_GAMEPAD) != 0u)
        pc_draw_gamepad_office_background(game);

    graphics_draw_rect(sOriginalUiPanelTargets,
                       174, 38, 674, 423,
                       GRAPHICS_RGB(0, 2, 1));
    graphics_draw_frame(sOriginalUiPanelTargets,
                        174, 38, 674, 423, 2,
                        GRAPHICS_RGB(52, 72, 52));

    const OriginalUiTextureId map_id =
        ((game->night_frames / 10u) & 1u)
            ? ORIGINAL_UI_VENT_MAP_ALT
            : ORIGINAL_UI_VENT_MAP_BASE;
    original_ui_draw_texture(map_id, sOriginalUiPanelTargets,
                             397, 68, 430, 401);

    pc_vent_map_draw_runtime_state(game);
    pc_vent_map_draw_sealing_progress();

    /* D-pad/A are Wii U input adaptations.  Keep the prompt out of the PC map
     * artwork; Monitor V2 draws the authentic Map Toggle button afterwards. */
    graphics_draw_text(sOriginalUiPanelTargets,
                       206, 410, 1,
                       "D-PAD CAM   A SEAL",
                       GRAPHICS_RGB(113, 132, 110));

    /* Map-toggle flash stays inside the raised panel. */
    if (sOriginalUi.static_flash_frames > 0u) {
        const int frame = (int)((game->night_frames / 2u) % 3u);
        original_ui_draw_texture(
            (OriginalUiTextureId)(ORIGINAL_UI_STATIC_0 + frame),
            sOriginalUiPanelTargets,
            397, 68, 430, 318);
    }
}
