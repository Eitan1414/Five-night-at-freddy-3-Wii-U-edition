static void original_ui_draw_vent_map(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    original_ui_draw_toolbar();

    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 25, 20, 3,
                       "VENTILATION MAP", COLOUR_GREEN);
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 385, 55, 455, 413, 3,
                        GRAPHICS_RGB(62, 100, 65));
    const OriginalUiTextureId map_id =
        ((game->night_frames / 10u) & 1u)
            ? ORIGINAL_UI_VENT_MAP_ALT
            : ORIGINAL_UI_VENT_MAP_BASE;
    original_ui_draw_texture(map_id, GRAPHICS_TARGET_GAMEPAD,
                             397, 68, 430, 401);

    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const int y = 98 + vent * 57;
        const bool selected = vent == (int) game->selected_vent;
        const bool sealed = vent == (int) game->sealed_vent;
        const bool occupied = springtrap_ai_is_in_vent(&game->springtrap) &&
            springtrap_ai_current_vent(&game->springtrap)
                == (SpringtrapVent) vent;
        uint32_t colour = selected ? COLOUR_WHITE
                                   : GRAPHICS_RGB(106, 146, 105);
        if (sealed) colour = GRAPHICS_RGB(194, 255, 77);
        if (occupied && sCompleteCheats.radar) colour = COLOUR_RED;
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 24, y, 334, 40,
                           GRAPHICS_RGB(2, 7, 3));
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 24, y, 334, 40, 2,
                            colour);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 42, y + 13, 2,
                           springtrap_vent_label((SpringtrapVent) vent),
                           colour);
        if (sealed)
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 258, y + 13, 1,
                               "SEALED", colour);
    }

    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 27, 407, 1,
                       "A/L: SEAL VENT", GRAPHICS_RGB(181, 202, 176));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 27, 431, 1,
                       "R: CAMERA MAP   X/Y: CLOSE",
                       GRAPHICS_RGB(181, 202, 176));

    if (sOriginalUi.static_flash_frames > 0u)
        original_ui_draw_static(game, true);
}

