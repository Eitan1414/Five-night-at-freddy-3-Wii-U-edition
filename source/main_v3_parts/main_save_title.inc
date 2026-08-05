#undef draw_title

static void draw_title(const Game *game)
{
    draw_title_without_save_status(game);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 121, 145, 1,
                       save_load_status_text(), save_load_status_colour());
    graphics_present(GRAPHICS_TARGET_BOTH);
}
