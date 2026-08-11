/* PC character bridge: replace PSX/placeholder office art in the final renderer. */

static void pc_draw_springtrap_office(const Game *game)
{
    const SpringtrapOfficeSide side = springtrap_ai_office_side(&game->springtrap);
    if (side == SPRINGTRAP_OFFICE_WINDOW && game->office_pan > 25) {
        texture_draw_rle(GRAPHICS_TARGET_TV,
                         646, 77, 164, 362,
                         &gPcSpringtrapOfficeWindowTexture);
    } else if (side == SPRINGTRAP_OFFICE_LEFT && game->office_pan < -25) {
        texture_draw_rle(GRAPHICS_TARGET_TV,
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
    if (phantoms_foxy_present(&game->phantoms) && game->office_pan < -15) {
        /* PC image-bank frame 170, referenced by the MFA's Foxy animation. */
        texture_draw_rle(GRAPHICS_TARGET_TV,
                         25, 122, 215, 315,
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
            /* Horizontal travel remains driven by the gameplay timer; the
             * body pose itself now comes from the two exact fwalk animations
             * stored in fivenights3-94.mfa. */
            const int x = 760 -
                (int) (game->phantoms.freddy_walk_frames * 5u);
            texture_draw_rle(GRAPHICS_TARGET_TV,
                             x, 126,
                             190, 255,
                             frame);
        }
    }

    if (game->phantoms.chica_office_waiting && game->office_pan < -10) {
        /* PC image-bank frame 399, the static Chica 2 office state. */
        texture_draw_rle(GRAPHICS_TARGET_TV,
                         -18, 82, 245, 340,
                         &gPcPhantomChicaOfficeTexture);
    }

    /*
     * Phantom Mangle's verified PC camera appearance is a complete CAM04
     * composite. The General Sprites bank does not expose a clean office
     * cut-out for the post-trigger garble state, so deliberately keep that
     * state effect-only rather than reintroducing the old PSX Mangle sprite.
     */
    if (phantoms_mangle_active(&game->phantoms)) {
        const uint32_t timer = game->phantoms.mangle_office_frames;
        if ((timer % 17u) < 4u) {
            graphics_draw_rect(GRAPHICS_TARGET_TV,
                               500, 74, 330, 3,
                               GRAPHICS_RGB(124, 142, 124));
            graphics_draw_rect(GRAPHICS_TARGET_TV,
                               535, 122, 280, 2,
                               GRAPHICS_RGB(55, 78, 57));
        }
    }

    if (phantoms_puppet_active(&game->phantoms)) {
        /* PC image-bank frame 320 (MFA object "phantom head"). */
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         102, 5, 650, 465,
                         &gPcPhantomPuppetTexture);
    }
}
