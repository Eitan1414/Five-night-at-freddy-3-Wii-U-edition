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

    /* Compact Wii U navigation aid.  Do not cover the extracted PC map with
     * the former full-height debug list. */
    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const int y = 116 + vent * 48;
        const bool selected = vent == (int) game->selected_vent;
        const bool sealed = vent == (int) game->sealed_vent;
        const bool occupied = springtrap_ai_is_in_vent(&game->springtrap) &&
            springtrap_ai_current_vent(&game->springtrap)
                == (SpringtrapVent)vent;

        uint32_t colour = selected
            ? GRAPHICS_RGB(194, 255, 77)
            : GRAPHICS_RGB(160, 178, 158);
        if (sealed) colour = GRAPHICS_RGB(194, 255, 77);
        if (occupied && sCompleteCheats.radar) colour = COLOUR_RED;

        if (selected)
            graphics_draw_text(sOriginalUiPanelTargets,
                               201, y + 2, 2, ">", colour);

        char label[16];
        snprintf(label, sizeof(label), "CAM %02d", 11 + vent);
        graphics_draw_text(sOriginalUiPanelTargets,
                           231, y + 4, 2, label, colour);
        if (sealed)
            graphics_draw_text(sOriginalUiPanelTargets,
                               324, y + 8, 1, "SEALED", colour);
    }

    graphics_draw_text(sOriginalUiPanelTargets,
                       206, 410, 1,
                       "D-PAD SELECT   A SEAL",
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
