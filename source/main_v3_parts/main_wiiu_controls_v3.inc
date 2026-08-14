/* Final non-office display routing.
 *
 * Follow Me must bypass the older finishing renderer so the full audio/content
 * chain can reach the restored PC renderer beneath it. Other screens keep the
 * existing finishing/MFA fallback order.
 */
static bool wiiu_follow_me_active(const Game *game)
{
    return game != NULL && game->screen == SCREEN_PLACEHOLDER &&
           sFinishing.mode == FINISHING_FOLLOW_ME;
}

static void wiiu_control_nonoffice_render(Game *game)
{
    if (wiiu_follow_me_active(game))
        fnaf3_full_audio_render_game(game);
    else
        pc_finishing_fallback_render_game(game);

    if (wiiu_controls_mode() == WIIU_CONTROL_MIRROR)
        return;

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, COLOUR_BLACK);
    graphics_present(GRAPHICS_TARGET_GAMEPAD);
}

/* Phone subtitle presentation is layered after the original finishing code so
 * it does not alter call playback or timing. Capture the legacy overlay before
 * remapping later office render calls to the bilingual subtitle wrapper. */
static void (*const wiiu_base_phone_overlay)(void) = finishing_draw_phone_overlay;
#define finishing_draw_phone_overlay_base wiiu_base_phone_overlay
#define finishing_draw_phone_overlay wiiu_phone_overlay_with_subtitles
#include "main_v3_parts/main_phone_subtitles.inc"
#undef finishing_draw_phone_overlay
#undef finishing_draw_phone_overlay_base
#define finishing_draw_phone_overlay wiiu_phone_overlay_with_subtitles
