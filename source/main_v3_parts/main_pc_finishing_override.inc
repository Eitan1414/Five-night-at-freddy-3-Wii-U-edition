#define pc_finishing_render_override pc_finishing_render_override_base
#include "main_pc_finishing_override_base.inc"
#undef pc_finishing_render_override

static void pc_draw_bad_ending_clean(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_tiled_texture_draw(GRAPHICS_TARGET_BOTH, 107, 0, 640, 480,
                          &gPcBadEndingTexture);
    (void) game;
}

static void pc_draw_newspaper_clean(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
    pc_tiled_texture_draw(GRAPHICS_TARGET_BOTH, 107, 0, 640, 480,
                          &gPcNewspaperEndingTexture);
    (void) game;
}

static bool pc_finishing_render_override(Game *game)
{
    bool handled = false;
    if (sSecret.active) {
        handled = true;
    } else if (game->screen == SCREEN_PLACEHOLDER) {
        handled = sFinishing.mode == FINISHING_FOLLOW_ME ||
                  sFinishing.mode == FINISHING_NIGHT5_ENDING ||
                  sFinishing.mode == FINISHING_NIGHT6_ENDING;
    }
    if (!handled) return false;
    if (!game->dirty) return true;
    game->dirty = false;

    if (sSecret.active) {
        pc_secret_draw_active(game);
    } else if (sFinishing.mode == FINISHING_FOLLOW_ME) {
        pc_finishing_draw_follow_me(game);
    } else if (sFinishing.mode == FINISHING_NIGHT5_ENDING) {
        pc_draw_bad_ending_clean(game);
    } else {
        pc_draw_newspaper_clean(game);
    }

    achievement_draw_overlay();
    graphics_present(GRAPHICS_TARGET_BOTH);
    return true;
}
