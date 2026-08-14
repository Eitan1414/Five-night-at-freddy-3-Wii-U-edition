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
 * it does not alter call playback. Keep the first percentage-based renderer
 * available for reference, but route the actual office overlay through the
 * MFA/WAV-backed timing pass below. */
static void (*const wiiu_base_phone_overlay)(void) = finishing_draw_phone_overlay;
#define finishing_draw_phone_overlay_base wiiu_base_phone_overlay
#define finishing_draw_phone_overlay wiiu_phone_overlay_with_estimated_subtitles
#include "main_v3_parts/main_phone_subtitles.inc"
#undef finishing_draw_phone_overlay
#undef finishing_draw_phone_overlay_base

#include "main_v3_parts/main_phone_subtitles_mfa_sync.inc"

#define finishing_draw_phone_overlay wiiu_phone_overlay_with_subtitles

/* The historical phone timer values predate the absolute 60 Hz frame clock and
 * outlive the embedded retail-PC WAVs. Clamp the runtime timer to the exact
 * 60 Hz durations measured from the MFA PCM resources so the call overlay and
 * mute state end with the voice instead of lingering after playback. */
static const uint32_t kMfaPhoneRuntimeFrames[NIGHT_COUNT] = {
    9787u, 7760u, 4580u, 3695u, 3264u, 2550u
};

static void wiiu_control_update_game_v2_mfa_phone(Game *game,
                                                   const InputState *input)
{
    wiiu_control_update_game_v2(game, input);

    if (!sFinishing.phone_active ||
        sFinishing.phone_night < 1 ||
        sFinishing.phone_night > NIGHT_COUNT) {
        return;
    }

    const uint32_t exact_frames =
        kMfaPhoneRuntimeFrames[sFinishing.phone_night - 1];
    sFinishing.phone_duration = exact_frames;
    if (sFinishing.phone_frames >= exact_frames)
        finishing_stop_phone();
}

#define wiiu_control_update_game_v2 wiiu_control_update_game_v2_mfa_phone
