/* Original PC Good Ending presentation from the supplied FNaF 3 sprite dump. */
static void secret_draw_good_ending(void)
{
    const TextureRle *ending = good_ending_pc_texture();
    if (ending == NULL) {
        secret_draw_good_ending_legacy();
        return;
    }

    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_BOTH, 107, 0, 640, 480, ending);

    /* Preserve the original image on the TV. The Wii U-only return prompt is
     * shown on the GamePad after the ending has had time to sit on screen. */
    if (sSecret.completed_frames >= 150u) {
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 248, 447, 358, 24,
                           GRAPHICS_RGB(2, 2, 2));
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 302, 454, 1,
                           "A: RETURN TO EXTRAS", COLOUR_WHITE);
    }
}
