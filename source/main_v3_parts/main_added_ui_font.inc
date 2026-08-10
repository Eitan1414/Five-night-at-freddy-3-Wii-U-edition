static int added_ui_fnaf3_scale(int legacy_scale)
{
    if (legacy_scale >= 3) return 2;
    return 1;
}

static void added_ui_draw_text(uint32_t targets,
                               int x,
                               int y,
                               int scale,
                               const char *text,
                               uint32_t colour)
{
    graphics_draw_fnaf3_menu_text(targets,
                                  x,
                                  y,
                                  added_ui_fnaf3_scale(scale),
                                  text,
                                  colour);
}

static void added_ui_draw_header(const char *title)
{
    /* Keep the Extras header/background exactly as before; only replace text. */
    finishing_draw_header("");
    graphics_draw_fnaf3_menu_text(GRAPHICS_TARGET_BOTH,
                                  35, 14, 2,
                                  title,
                                  COLOUR_GREEN);
}
