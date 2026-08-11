/* PC character bridge: replace PSX/placeholder Springtrap and Phantom art. */

static void draw_springtrap_office(const Game *game)
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

static void draw_phantom_office(const Game *game)
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
     * PC Phantom Mangle is a camera composite rather than a clean office
     * sprite in General Sprites. Do not re-introduce the PSX office texture:
     * keep the original PC-style garble/failure effect after CAM 04 instead.
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

static void draw_springtrap_camera_sprite(int camera)
{
    const PcTiledTexture *texture = pc_springtrap_camera_texture(camera);
    if (texture != NULL) {
        pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                              28, 65, 532, 295,
                              texture);
    }
}

static void draw_phantom_camera_overlay(const Game *game)
{
    const PhantomId phantom = phantoms_camera_overlay(
        &game->phantoms, game->selected_camera);
    const int bob = phantom_bob(game->night_frames, 24u, 7);

    switch (phantom) {
        case PHANTOM_BALLOON_BOY:
            texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                             145 - bob, 55 - bob,
                             325 + bob * 2, 305 + bob * 2,
                             &gPcPhantomBBTexture);
            break;
        case PHANTOM_CHICA:
            pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                                  28, 65, 532, 295,
                                  &gPcPhantomChicaCameraTexture);
            break;
        case PHANTOM_MANGLE:
            pc_tiled_texture_draw(GRAPHICS_TARGET_GAMEPAD,
                                  28, 65, 532, 295,
                                  &gPcPhantomMangleCameraTexture);
            break;
        case PHANTOM_PUPPET:
            texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                             194, 38, 228, 328,
                             &gPcPhantomPuppetTexture);
            break;
        default:
            break;
    }
}
