/* Final non-office display routing.
 *
 * In TOUCH GAMEPAD and TV ONLY, the DRC is intentionally black whenever there
 * is no camera/maintenance touch panel. IDENTICAL keeps the normal renderer on
 * both displays. The Wii U Options screen is handled separately before this
 * fallback and therefore remains visible/touchable on the GamePad.
 */
static void wiiu_control_nonoffice_render(Game *game)
{
    pc_finishing_fallback_render_game(game);
    if (wiiu_controls_mode() == WIIU_CONTROL_MIRROR)
        return;

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
    graphics_present(GRAPHICS_TARGET_GAMEPAD);
}
