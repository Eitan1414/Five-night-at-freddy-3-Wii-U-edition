/* PC character bridge: replace PSX/placeholder office art in the final renderer. */

/* The authentic office renderer changes this target while drawing the TV or a
 * mirrored GamePad copy. Character code must never hard-wire BOTH: doing so
 * leaked Phantom Puppet onto the DRC in TOUCH/TV ONLY modes. */
static uint32_t sPcOfficeTargets = GRAPHICS_TARGET_TV;

static void pc_draw_springtrap_office(const Game *game)
{
    /* Night 1 has no Springtrap in the retail game.  springtrap_ai_reset()
     * still initialises a camera slot for source/API stability, so every
     * renderer must gate the character on the night configuration. */
    if (!night_config(game->night)->springtrap_enabled)
        return;

    const SpringtrapOfficeSide side = springtrap_ai_office_side(&game->springtrap);
    if (side == SPRINGTRAP_OFFICE_WINDOW && game->office_pan > 25) {
        texture_draw_rle(sPcOfficeTargets,
                         646, 77, 164, 362,
                         &gPcSpringtrapOfficeWindowTexture);
    } else if (side == SPRINGTRAP_OFFICE_LEFT && game->office_pan < -25) {
        texture_draw_rle(sPcOfficeTargets,
                         22, 72, 170, 374,
                         &gPcSpringtrapOfficeLeftTexture);
    }
}

static const TextureRle *pc_sequence_loop_frame(const JumpscareSequence *sequence,
                                                uint32_t timer)
{
    if (sequence == NULL || sequence->frames == NULL ||
        sequence->frame_count == 0u || sequence->ticks_per_frame == 0u) {
        return NULL;
    }
    const uint32_t frame =
        (timer / sequence->ticks_per_frame) % sequence->frame_count;
    return sequence->frames[frame];
}

static const TextureRle *pc_sequence_clamped_frame(const JumpscareSequence *sequence,
                                                   uint32_t timer)
{
    if (sequence == NULL || sequence->frames == NULL ||
        sequence->frame_count == 0u || sequence->ticks_per_frame == 0u) {
        return NULL;
    }
    uint32_t frame = timer / sequence->ticks_per_frame;
    if (frame >= sequence->frame_count) frame = sequence->frame_count - 1u;
    return sequence->frames[frame];
}

static void pc_draw_phantom_office(const Game *game)
{
    const int texture_x = -133 + game->office_pan;

    if (phantoms_foxy_present(&game->phantoms) && game->office_pan < -15) {
        /* MFA object "foxy": image 302. The object is created at hotspot
         * (698,768) relative to the original 2000x768 office panorama. Image
         * 302 has hotspot (156,708), giving top-left (542,60). The PC office
         * panorama is rendered at 1120x430, i.e. a uniform 0.56 scale. */
        texture_draw_rle(sPcOfficeTargets,
                         texture_x + 304, 58,
                         175, 397,
                         &gPcPhantomFoxyOfficeTexture);
    }

    if (phantoms_freddy_walking(&game->phantoms)) {
        const bool ducking = game->phantoms.freddy_duck_frames > 0u;
        const JumpscareSequence *sequence = ducking
            ? &gPcPhantomFreddyDuck
            : &gPcPhantomFreddyWalk;
        const uint32_t timer = ducking
            ? game->phantoms.freddy_duck_frames
            : game->phantoms.freddy_walk_frames;
        const TextureRle *frame = ducking
            ? pc_sequence_clamped_frame(sequence, timer)
            : pc_sequence_loop_frame(sequence, timer);
        if (frame != NULL) {
            /* fwalk's horizontal travel follows the decoded MFA path
             * accumulator instead of the old hand-authored 5 px/frame slide. */
            const int x = 760 -
                (int) (game->phantoms.freddy_path_eighths / 8u);
            texture_draw_rle(sPcOfficeTargets,
                             x, 126,
                             190, 255,
                             frame);
        }
    }

    if (game->phantoms.chica_office_waiting && game->office_pan < -10) {
        /* MFA object "chica 2": image 399. Creation hotspot (4,2) with image
         * hotspot (-202,-114) resolves to office top-left (206,116), scaled
         * with the same 0.56 PC-panorama transform. */
        texture_draw_rle(sPcOfficeTargets,
                         texture_x + 115, 89,
                         362, 366,
                         &gPcPhantomChicaOfficeTexture);
    }

    if (phantoms_mangle_active(&game->phantoms)) {
        /* MFA frame instance "mangle 2" starts at hotspot (1118,512); image
         * 202 hotspot is (83,103). This yields top-left (1035,409) in the
         * original 2000x768 office panorama. Draw the actual PC sprite and
         * let the AI/path timer own its lifetime; no synthetic glitch bars. */
        texture_draw_rle(sPcOfficeTargets,
                         texture_x + 580, 253,
                         94, 115,
                         &gPcPhantomMangleOfficeTexture);
    }

    if (phantoms_puppet_active(&game->phantoms)) {
        /* PC image-bank frame 320 (MFA object "phantom head"). */
        texture_draw_rle(sPcOfficeTargets,
                         102, 5, 650, 465,
                         &gPcPhantomPuppetTexture);
    }
}
