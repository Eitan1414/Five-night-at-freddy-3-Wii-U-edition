/* Wii U GamePad presentation for PC office-click secret triggers.
 *
 * This file is deliberately an overlay only. main_04.inc and main_05.inc split
 * a legacy renderer function across two include fragments, so replacing
 * draw_office_gamepad between them would place these definitions inside that
 * function. The final original-UI office renderer calls this overlay after the
 * normal GamePad office screen has been drawn.
 */

static void office_touch_draw_stage01_panel(const Game *game)
{
    if (game->night < 4 || game->office_pan > -80 ||
        game->camera_open || game->maintenance_open) {
        return;
    }

    const int origin_x = 286;
    const int origin_y = 124;
    const int cell_w = 82;
    const int cell_h = 70;
    const int gap = 7;

    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       origin_x - 18, origin_y - 18,
                       cell_w * 3 + gap * 2 + 36,
                       cell_h * 3 + gap * 2 + 36,
                       GRAPHICS_RGB(19, 18, 20));

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            const int x = origin_x + col * (cell_w + gap);
            const int y = origin_y + row * (cell_h + gap);
            const uint32_t shade = ((row + col) & 1) != 0
                ? GRAPHICS_RGB(70, 67, 68)
                : GRAPHICS_RGB(55, 52, 54);
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               x, y, cell_w, cell_h, shade);
            graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                                x, y, cell_w, cell_h, 2,
                                GRAPHICS_RGB(91, 86, 88));
        }
    }
}

static void office_touch_draw_shadow_figure(const Game *game)
{
    if (game->night < 5 || game->office_pan < 90 ||
        game->camera_open || game->maintenance_open) {
        return;
    }

    minigame_pc_extended_assets_init();
    if (gMiniShadowRight.width == 0u || gMiniShadowRight.height == 0u)
        return;

    const int scale = 4;
    const int width = (int) gMiniShadowRight.width * scale;
    const int height = (int) gMiniShadowRight.height * scale;
    const int x = 427 - width / 2;
    const int y = 305 - height;
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     x, y, width, height, &gMiniShadowRight);
}

static void office_touch_draw_overlay(const Game *game)
{
    office_touch_draw_stage01_panel(game);
    office_touch_draw_shadow_figure(game);
}
