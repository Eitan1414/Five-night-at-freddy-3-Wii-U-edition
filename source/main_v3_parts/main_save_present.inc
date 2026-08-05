static void present_with_save_status(uint32_t targets, const Game *game)
{
    if (game->screen == SCREEN_TITLE) {
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 145, 1,
                           save_load_status_text(), save_load_status_colour());
    }
    graphics_present(targets);
}

#define graphics_present(targets) present_with_save_status((targets), game)
