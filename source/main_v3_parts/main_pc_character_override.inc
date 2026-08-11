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

static void pc_draw_phantom_office(const Game *game)
{
    if (phantoms_foxy_present(&game->phantoms) && game->office_pan < -15) {
        const int bob = phantom_bob(game->night_frames, 26u, 6);
        texture_draw_rle(GRAPHICS_TARGET_TV,
                         25 + bob / 2, 122 - bob,
                         215, 315,
                         &gPcPhantomFoxyOfficeTexture);
    }

    if (phantoms_freddy_walking(&game->phantoms)) {
        const TextureRle *frame = sequence_ping_pong_frame(
            &gPcPhantomFreddyWalk,
            game->phantoms.freddy_walk_frames);
        if (frame != NULL) {
            const int step = phantom_bob(
                game->phantoms.freddy_walk_frames, 18u, 7);
            const int x = 760 -
                (int) (game->phantoms.freddy_walk_frames * 5u);
            texture_draw_rle(GRAPHICS_TARGET_TV,
                             x, 126 - step,
                             190, 255 + step,
                             frame);
        }
    }

    if (game->phantoms.chica_office_waiting && game->office_pan < -10) {
        const int pulse = phantom_bob(game->night_frames, 20u, 8);
        texture_draw_rle(GRAPHICS_TARGET_TV,
                         -18 - pulse / 2, 82 - pulse,
                         245 + pulse, 340 + pulse,
                         &gPcPhantomChicaOfficeTexture);
    }

    /*
     * PC Phantom Mangle is represented by a complete CAM 04 composite rather
     * than a clean office cut-out in General Sprites. Once it reaches the
     * office, preserve its PC-style garble/system-failure effect instead of
     * putting the old PSX sprite back on top of the PC office panorama.
     */
    if (phantoms_mangle_active(&game->phantoms)) {
        const uint32_t timer = game->phantoms.mangle_office_frames;
        const int drift = phantom_bob(timer, 30u, 9);
        if ((timer % 17u) < 4u) {
            graphics_draw_rect(GRAPHICS_TARGET_TV,
                               500 + drift, 74 + drift,
                               330 - drift * 2, 3,
                               GRAPHICS_RGB(124, 142, 124));
            graphics_draw_rect(GRAPHICS_TARGET_TV,
                               535 - drift, 122,
                               280, 2,
                               GRAPHICS_RGB(55, 78, 57));
        }
    }

    if (phantoms_puppet_active(&game->phantoms)) {
        const int drift = phantom_bob(
            game->phantoms.puppet_office_frames, 48u, 18);
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         102 + drift / 2, 5 - drift / 2,
                         650 - drift, 465 + drift,
                         &gPcPhantomPuppetTexture);
    }
}
