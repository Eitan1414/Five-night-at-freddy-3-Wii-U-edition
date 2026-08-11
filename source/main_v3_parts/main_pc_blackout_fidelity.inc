/*
 * Retail-PC ventilation blackout presentation.
 *
 * fivenights3-94.mfa uses the ventilation text object's alterable value B as
 * the common time base for blackouts and Hallucinationtrap. The exact
 * Clickteam shader/ink fade is not exposed by the reconstructed Wii U renderer,
 * so this layer preserves B as the source of truth and maps it to an SDL alpha
 * fade instead of the old horizontal black-band approximation.
 *
 * The PC presentation waits a few seconds after a natural ventilation failure,
 * then repeatedly fades vision to black and back. Phantom-caused errors enter
 * the same curve at the fade point; main_pc_system_fidelity applies that source
 * distinction when the Phantom jumpscare finishes.
 */
#define PC_VENT_FADE_DELAY_FRAMES 180u
#define PC_VENT_FADE_IN_FRAMES 90u
#define PC_VENT_BLACK_HOLD_FRAMES 60u
#define PC_VENT_FADE_OUT_FRAMES 90u
#define PC_VENT_CLEAR_HOLD_FRAMES 180u
#define PC_VENT_FADE_CYCLE_FRAMES \
    (PC_VENT_FADE_IN_FRAMES + PC_VENT_BLACK_HOLD_FRAMES + \
     PC_VENT_FADE_OUT_FRAMES + PC_VENT_CLEAR_HOLD_FRAMES)

static uint32_t pc_graphics_rgba(uint8_t red,
                                 uint8_t green,
                                 uint8_t blue,
                                 uint8_t alpha)
{
    return ((uint32_t)red << 24u) |
           ((uint32_t)green << 16u) |
           ((uint32_t)blue << 8u) |
           (uint32_t)alpha;
}

static uint8_t pc_ventilation_blackout_alpha(uint32_t b)
{
    if (b <= PC_VENT_FADE_DELAY_FRAMES) return 0u;

    const uint32_t phase =
        (b - PC_VENT_FADE_DELAY_FRAMES - 1u) % PC_VENT_FADE_CYCLE_FRAMES;

    if (phase < PC_VENT_FADE_IN_FRAMES) {
        return (uint8_t)((phase * 255u) / PC_VENT_FADE_IN_FRAMES);
    }

    uint32_t cursor = PC_VENT_FADE_IN_FRAMES;
    if (phase < cursor + PC_VENT_BLACK_HOLD_FRAMES) return 255u;

    cursor += PC_VENT_BLACK_HOLD_FRAMES;
    if (phase < cursor + PC_VENT_FADE_OUT_FRAMES) {
        const uint32_t fade = phase - cursor;
        return (uint8_t)(255u -
            (fade * 255u) / PC_VENT_FADE_OUT_FRAMES);
    }

    return 0u;
}

static bool pc_ventilation_is_blind(uint32_t b)
{
    /* Springtrap's MFA attack-stage condition is the actual blackout, not a
     * mildly dim frame. Keep a short near-black window around the full fade. */
    return pc_ventilation_blackout_alpha(b) >= 248u;
}

static uint8_t pc_ventilation_red_flash_alpha(uint32_t b)
{
    /* The alarm light strobes continuously while ventilation is offline. A
     * 48-frame triangle avoids a harsh binary Wii U flash while retaining the
     * PC office's red pulse. */
    const uint32_t phase = b % 48u;
    const uint32_t ramp = phase < 24u ? phase : 47u - phase;
    return (uint8_t)(26u + ramp * 3u);
}

static void pc_draw_ventilation_overlay(const Game *game)
{
    if (game == NULL || !game->ventilation_failed) return;

    /* In the original PC frame the red alarm belongs to the office lighting.
     * The Wii U keeps the office on TV while the interactive panels live on
     * the GamePad, so tint only the TV before applying the player's blackout. */
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       0, 0,
                       GRAPHICS_LOGICAL_WIDTH,
                       GRAPHICS_LOGICAL_HEIGHT,
                       pc_graphics_rgba(180u, 8u, 4u,
                           pc_ventilation_red_flash_alpha(
                               game->ventilation_error_frames)));

    const uint8_t blackout =
        pc_ventilation_blackout_alpha(game->ventilation_error_frames);
    if (blackout == 0u) return;

    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       0, 0,
                       GRAPHICS_LOGICAL_WIDTH,
                       GRAPHICS_LOGICAL_HEIGHT,
                       pc_graphics_rgba(0u, 0u, 0u, blackout));
}
