    graphics_draw_text(GRAPHICS_TARGET_BOTH, 245, 315, 2, night_config(game->night)->subtitle, GRAPHICS_RGB(84, 124, 82));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 306, 397, 1, "A: SKIP INTRO", GRAPHICS_RGB(72, 102, 71));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_springtrap_office(const Game *game)
{
    const SpringtrapOfficeSide side = springtrap_ai_office_side(&game->springtrap);
    int x = 0;
    if (side == SPRINGTRAP_OFFICE_WINDOW && game->office_pan > 25) x = 690;
    else if (side == SPRINGTRAP_OFFICE_LEFT && game->office_pan < -25) x = 95;
    else return;
    graphics_draw_ellipse(GRAPHICS_TARGET_TV, x, 182, 61, 100, GRAPHICS_RGB(16, 29, 15));
    graphics_draw_ellipse(GRAPHICS_TARGET_TV, x, 165, 47, 72, GRAPHICS_RGB(42, 67, 34));
    graphics_draw_rect(GRAPHICS_TARGET_TV, x - 38, 76, 18, 68, GRAPHICS_RGB(34, 58, 30));
    graphics_draw_rect(GRAPHICS_TARGET_TV, x + 19, 67, 17, 76, GRAPHICS_RGB(34, 58, 30));
    graphics_draw_rect(GRAPHICS_TARGET_TV, x - 28, 157, 17, 9, COLOUR_WHITE);
    graphics_draw_rect(GRAPHICS_TARGET_TV, x + 13, 157, 17, 9, COLOUR_WHITE);
}

static void draw_phantom_office(const Game *game)
{
    if (phantoms_foxy_present(&game->phantoms) && game->office_pan < -15) {
        const int bob = phantom_bob(game->night_frames, 26u, 6);
        texture_draw_rle(GRAPHICS_TARGET_TV, 42 + bob / 2, 152 - bob,
                         190, 290, &gPhantomFoxyOfficeTexture);
    }
    if (phantoms_freddy_walking(&game->phantoms)) {
        const int step = phantom_bob(game->phantoms.freddy_walk_frames, 18u, 7);
        int x = 760 - (int) (game->phantoms.freddy_walk_frames * 5u);
        texture_draw_rle(GRAPHICS_TARGET_TV, x, 150 - step,
                         160, 210 + step, &gPhantomFreddyOfficeTexture);
    }
    if (game->phantoms.chica_office_waiting && game->office_pan < -10) {
        const int pulse = phantom_bob(game->night_frames, 20u, 8);
        texture_draw_rle(GRAPHICS_TARGET_TV, -pulse / 2, 105 - pulse,
                         210 + pulse, 315 + pulse, &gPhantomChicaOfficeTexture);
    }
    if (phantoms_mangle_active(&game->phantoms)) {
        const uint32_t timer = game->phantoms.mangle_office_frames;
        const int bob = phantom_bob(timer, 30u, 9);
        if (((timer / 9u) % 5u) == 0u) {
            texture_draw_rle(GRAPHICS_TARGET_TV, 566 - bob, 77 + bob,
                             256, 250, &gPhantomMangleCameraTexture);
        } else {
            texture_draw_rle(GRAPHICS_TARGET_TV, 592 - bob / 2, 90 - bob,
                             220 + bob, 260 + bob, &gPhantomMangleOfficeTexture);
        }
        if ((timer % 17u) < 3u) {
            graphics_draw_rect(GRAPHICS_TARGET_TV, 556, 170 + bob,
                               280, 4, GRAPHICS_RGB(124, 142, 124));
        }
    }
    if (phantoms_puppet_active(&game->phantoms)) {
        const TextureRle *frame = sequence_ping_pong_frame(
            &gPhantomPuppetRealAnimation,
            game->phantoms.puppet_office_frames);
        const int drift = phantom_bob(game->phantoms.puppet_office_frames,
                                      48u, 18);
        if (frame != NULL) {
            texture_draw_rle(GRAPHICS_TARGET_BOTH, 82 + drift / 2,
                             5 - drift / 2, 690 - drift, 470 + drift, frame);
        } else {
            texture_draw_rle(GRAPHICS_TARGET_BOTH, 112, 20, 630, 440,
                             &gPhantomPuppetOfficeTexture);
        }
    }
}

static void draw_office_tv(const Game *game)
{
    const int texture_x = -133 + game->office_pan;
    graphics_clear(GRAPHICS_TARGET_TV, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_TV, texture_x, 24, 1120, 430, &gOfficeTexture);
    draw_springtrap_office(game);
    draw_phantom_office(game);
    for (int y = 30; y < 454; y += 12)
        graphics_draw_rect(GRAPHICS_TARGET_TV, 0, y, 854, 1, GRAPHICS_RGB(4, 8, 5));
    char night_text[24], clock_text[16];
    snprintf(night_text, sizeof(night_text), "NIGHT %d", game->night);
    snprintf(clock_text, sizeof(clock_text), "%d AM", game->hour);
    graphics_draw_text(GRAPHICS_TARGET_TV, 18, 5, 2, night_text, GRAPHICS_RGB(166, 191, 151));
    graphics_draw_rect(GRAPHICS_TARGET_TV, 718, 4, 118, 36, GRAPHICS_RGB(1, 5, 2));
    graphics_draw_frame(GRAPHICS_TARGET_TV, 718, 4, 118, 36, 2, GRAPHICS_RGB(65, 103, 63));
    graphics_draw_text(GRAPHICS_TARGET_TV, 744, 16, 2, clock_text, COLOUR_WHITE);
    if (game->ventilation_failed) {
        graphics_draw_frame(GRAPHICS_TARGET_TV, 555, 9, 157, 34, 3, COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_TV, 572, 20, 1, "VENT ERROR", COLOUR_RED);
    }
}

static void draw_office_gamepad(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(3, 7, 4));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 57, 57, 740, 356, 4, GRAPHICS_RGB(40, 77, 43));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 91, 91, 4, "OFFICE CONTROL", COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 91, 178, 3, "X/Y: CAMERA PANEL", GRAPHICS_RGB(134, 181, 129));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 91, 239, 3, "MINUS: MAINTENANCE", GRAPHICS_RGB(134, 181, 129));
    if (game->idle_frames > 420u && !game->ventilation_failed)
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 91, 365, 2, "AIR QUALITY DROPPING...", GRAPHICS_RGB(192, 131, 70));
}

static void draw_camera_map_lines(void)
{
    const uint32_t line = GRAPHICS_RGB(67, 79, 70);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 624, 317, 92, 4, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 706, 246, 4, 75, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 716, 239, 70, 4, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 740, 170, 4, 71, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 704, 185, 78, 4, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 631, 190, 73, 4, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 628, 140, 4, 54, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 630, 121, 52, 4, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 700, 86, 4, 37, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 732, 81, 55, 4, line);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 676, 137, 4, 51, line);
}

static void draw_camera_map(const Game *game)
{
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 584, 43, 263, 386, 3, GRAPHICS_RGB(72, 91, 75));
    draw_camera_map_lines();
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        char label[8]; snprintf(label, sizeof(label), "C%02d", camera + 1);
        const int x = kCameraMapPoints[camera].x, y = kCameraMapPoints[camera].y;
        uint32_t colour = camera == game->selected_camera ? GRAPHICS_RGB(160, 211, 76) : GRAPHICS_RGB(194, 202, 194);
        if (springtrap_ai_is_on_camera(&game->springtrap, camera)) colour = COLOUR_RED;
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, x, y, 53, 28, 2, colour);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, x + 7, y + 9, 1, label, colour);
    }
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 679, 371, 82, 42, 2, COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 696, 386, 1, "YOU", COLOUR_WHITE);
}

static void draw_springtrap_camera_sprite(int camera)
{
    static const int px[10] = {337,452,320,46,451,48,53,235,50,432};
    static const int py[10] = {86,79,238,75,77,141,80,73,82,79};
    static const int pw[10] = {166,83,188,90,85,100,168,141,196,118};
    static const int ph[10] = {236,244,86,236,238,174,207,244,221,239};
    int cx=px[camera]+pw[camera]/2, cy=py[camera]+ph[camera]/2;
    graphics_draw_ellipse(GRAPHICS_TARGET_GAMEPAD, cx, cy, pw[camera]/3, ph[camera]/2, GRAPHICS_RGB(8,15,7));
    graphics_draw_ellipse(GRAPHICS_TARGET_GAMEPAD, cx, py[camera]+ph[camera]/3, pw[camera]/3, ph[camera]/4, GRAPHICS_RGB(35,58,29));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, cx-pw[camera]/6, py[camera]+ph[camera]/3, pw[camera]/10, 5, COLOUR_WHITE);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, cx+pw[camera]/12, py[camera]+ph[camera]/3, pw[camera]/10, 5, COLOUR_WHITE);
}

static const char *message_text(const Game *game)
{
    switch (game->message) {
        case MESSAGE_AUDIO_PLAYING: return "PLAYING AUDIO...";
        case MESSAGE_AUDIO_SUCCESS: return "AUDIO LURE SUCCESS";
        case MESSAGE_AUDIO_FAILED: return "AUDIO DEVICES ERROR";
        case MESSAGE_AUDIO_RECHARGING: return "AUDIO RECHARGING";
        case MESSAGE_AUDIO_NO_TARGET: return "SPRINGTRAP CANNOT HEAR IT";
        case MESSAGE_AUDIO_IGNORED: return "SPRINGTRAP IGNORED THE LURE";
        case MESSAGE_VENT_SEALED: return "VENT SEAL UPDATED";
        case MESSAGE_DANGER_SOFT: return "DANGER: DISTANT";
        case MESSAGE_DANGER_LOUD: return "DANGER: VERY CLOSE";
        case MESSAGE_PHANTOM_MANGLE: return "PHANTOM MANGLE: AUDIO ERROR";
        case MESSAGE_PHANTOM_PUPPET: return "PHANTOM PUPPET";
        default: return "A: PLAY AUDIO";
    }
}

static void draw_phantom_camera_overlay(const Game *game)
{
    PhantomId phantom = phantoms_camera_overlay(&game->phantoms, game->selected_camera);
    const int bob = phantom_bob(game->night_frames, 24u, 7);
    switch (phantom) {
        case PHANTOM_BALLOON_BOY:
            if (game->phantoms.bb_seen_frames >= 22u)
                texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 142 - bob, 56 - bob,
                                 326 + bob * 2, 306 + bob * 2,
                                 &gPhantomBBOfficeTexture);
            else
                texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 180 + bob / 2,
                                 88 - bob, 250, 250,
                                 &gPhantomBBCameraTexture);
            break;