/* Preserve the exact PC office renderer, then layer only the small Wii U panel affordances. */
static void wiiu_v9_draw_office_panel_hints(const Game *game, uint32_t targets);
#define draw_authentic_office_tv draw_authentic_office_tv_base
#include "main_authentic_office_base.inc"
#undef draw_authentic_office_tv

static void draw_authentic_office_tv(const Game *game)
{
    draw_authentic_office_tv_base(game);
    wiiu_v9_draw_office_panel_hints(game, GRAPHICS_TARGET_TV);
}
