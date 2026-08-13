/* Final non-office display routing.
 *
 * In TOUCH GAMEPAD and TV ONLY, the DRC is intentionally black whenever there
 * is no camera/maintenance touch panel. IDENTICAL keeps the normal renderer on
 * both displays. The Wii U Options screen is handled separately before this
 * fallback and therefore remains visible/touchable on the GamePad.
 *
 * The authentic PC Follow Me renderer was previously stranded on the old
 * pc-minigames branch. Reuse that renderer here before the older finishing
 * fallback gets a chance to draw its waypoint approximation.
 */
#include "assets/follow_me_pc_assets.h"
#include "main_follow_me_pc.inc"

static bool wiiu_follow_me_active(const Game *game)
{
    return game != NULL && game->screen == SCREEN_PLACEHOLDER &&
           sFinishing.mode == FINISHING_FOLLOW_ME;
}

static void wiiu_control_nonoffice_render(Game *game)
{
    if (wiiu_follow_me_active(game)) {
        if (game->dirty) {
            game->dirty = false;
            if (!sFollowMePc.initialized ||
                sFollowMePc.chapter != sFinishing.minigame_chapter) {
                follow_me_pc_reset(sFinishing.minigame_chapter);
                follow_me_pc_sync();
            }
            follow_me_pc_draw(game);
        }
    } else {
        pc_finishing_fallback_render_game(game);
    }

    if (wiiu_controls_mode() == WIIU_CONTROL_MIRROR)
        return;

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
    graphics_present(GRAPHICS_TARGET_GAMEPAD);
}
