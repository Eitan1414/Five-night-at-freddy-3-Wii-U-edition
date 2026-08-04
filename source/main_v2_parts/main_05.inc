    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91, 239, 3,
                       "MINUS: MAINTENANCE", GRAPHICS_RGB(134, 181, 129));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91, 314, 2,
                       "MOVE THE OFFICE VIEW TO STAY ACTIVE",
                       GRAPHICS_RGB(76, 110, 75));
    if (game->idle_frames > 420u && !game->ventilation_failed) {
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           91, 365, 2,
                           "AIR QUALITY DROPPING...", GRAPHICS_RGB(192, 131, 70));
    }
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
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        584, 43, 263, 386, 3,
                        GRAPHICS_RGB(72, 91, 75));
    draw_camera_map_lines();

    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        char label[8];
        snprintf(label, sizeof(label), "C%02d", camera + 1);
        const int x = kCameraMapPoints[camera].x;
        const int y = kCameraMapPoints[camera].y;
        uint32_t colour = camera == game->selected_camera
            ? GRAPHICS_RGB(160, 211, 76)
            : GRAPHICS_RGB(194, 202, 194);
        if (night_config(game->night)->springtrap_enabled &&
            springtrap_ai_is_on_camera(&game->springtrap, camera)) {
            colour = COLOUR_RED;
        }
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            x, y, 53, 28, 2, colour);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           x + 7, y + 9, 1, label, colour);
    }

    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        679, 371, 82, 42, 2, COLOUR_WHITE);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       696, 386, 1, "YOU", COLOUR_WHITE);
}

static void draw_springtrap_camera_sprite(int camera)
{
    if (camera < 0 || camera >= CAMERA_COUNT) {
        return;
    }
    const SpritePlacement p = kSpringtrapPlacements[camera];
    const int centre_x = p.x + p.width / 2;
    const int centre_y = p.y + p.height / 2;
    const uint32_t suit = GRAPHICS_RGB(35, 58, 29);
    const uint32_t shadow = GRAPHICS_RGB(8, 15, 7);
    const uint32_t eye = GRAPHICS_RGB(218, 235, 201);
    graphics_draw_ellipse(GRAPHICS_TARGET_GAMEPAD,
                          centre_x, centre_y, p.width / 3, p.height / 2,
                          shadow);
    graphics_draw_ellipse(GRAPHICS_TARGET_GAMEPAD,
                          centre_x, p.y + p.height / 3,
                          p.width / 3, p.height / 4, suit);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       centre_x - p.width / 4, p.y,
                       p.width / 8, p.height / 3, suit);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       centre_x + p.width / 8, p.y - 4,
                       p.width / 8, p.height / 3 + 4, suit);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       centre_x - p.width / 6, p.y + p.height / 3,
                       p.width / 10, 5, eye);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       centre_x + p.width / 12, p.y + p.height / 3,
                       p.width / 10, 5, eye);
}

static const char *message_text(const Game *game)
{
    switch (game->message) {
        case MESSAGE_AUDIO_PLAYING: return "PLAYING AUDIO...";
        case MESSAGE_AUDIO_SUCCESS: return "AUDIO LURE SUCCESS";
        case MESSAGE_AUDIO_FAILED: return "AUDIO DEVICES ERROR";
        case MESSAGE_AUDIO_RECHARGING: return "AUDIO RECHARGING";
        case MESSAGE_AUDIO_NO_TARGET: return "NO ADJACENT TARGET";
        case MESSAGE_VENT_SEALED: return "VENT SEAL UPDATED";
        case MESSAGE_DANGER_SOFT: return "DANGER: DISTANT";
        case MESSAGE_DANGER_LOUD: return "DANGER: VERY CLOSE";
        case MESSAGE_NONE:
        default: return "A: PLAY AUDIO";
    }
}

static void draw_camera_feed(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    if (game->camera_failed) {
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            58, 67, 738, 306, 5, COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           123, 132, 5, "CAMERA SYSTEM ERROR", COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           191, 232, 3, "SIGNAL LOST", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           139, 300, 2,
                           "MINUS: OPEN MAINTENANCE", GRAPHICS_RGB(176, 118, 105));
        return;
    }

    char label[16];
    snprintf(label, sizeof(label), "CAM %02d", game->selected_camera + 1);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       23, 18, 3, label, COLOUR_GREEN);
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        18, 55, 552, 315, 4,
                        GRAPHICS_RGB(61, 112, 62));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       28, 65, 532, 295, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     28, 65, 532, 295,
                     kCameraTextures[game->selected_camera]);

    if (night_config(game->night)->springtrap_enabled &&
        springtrap_ai_is_on_camera(&game->springtrap,
                                   game->selected_camera)) {
        draw_springtrap_camera_sprite(game->selected_camera);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           320, 338, 2, "MOTION DETECTED", COLOUR_RED);
    }

    for (int y = 70; y < 360; y += 10) {
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           28, y, 532, 1, GRAPHICS_RGB(5, 10, 6));
    }
    for (int bar = 0; bar < 4; ++bar) {
        const uint32_t shifted = game->seed + game->night_frames *
            (uint32_t) (bar + 1) * 0x1021u;
        const int y = 72 + (int) (shifted % 280u);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           31 + (int) ((shifted >> 8) % 50u),
                           y, 380 + (int) ((shifted >> 16) % 100u),
                           1 + (bar & 1), GRAPHICS_RGB(25, 61, 30));
    }

    draw_camera_map(game);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       29, 392, 2, message_text(game),
                       game->message == MESSAGE_AUDIO_FAILED ||
                       game->message == MESSAGE_DANGER_LOUD
                           ? COLOUR_RED : COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       29, 432, 1,
                       "D-PAD SELECT  A LURE  R VENTS  X/Y CLOSE",
                       GRAPHICS_RGB(127, 171, 124));
}

static void draw_vent_map(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       26, 20, 3, "VENTILATION MAP", COLOUR_GREEN);
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        28, 62, 798, 338, 4,
                        GRAPHICS_RGB(61, 112, 62));

    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const int y = 91 + vent * 55;
        const bool selected = vent == (int) game->selected_vent;
        const bool sealed = vent == (int) game->sealed_vent;
        const bool occupied = springtrap_ai_is_in_vent(&game->springtrap) &&
            springtrap_ai_current_vent(&game->springtrap) == (SpringtrapVent) vent;
        uint32_t colour = selected ? COLOUR_WHITE : GRAPHICS_RGB(97, 137, 96);
        if (sealed) {
            colour = GRAPHICS_RGB(152, 215, 107);
        }
        if (occupied) {
            colour = COLOUR_RED;
        }
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            59, y, 730, 39, 3, colour);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           78, y + 13, 2,
                           springtrap_vent_label((SpringtrapVent) vent), colour);
        if (sealed) {
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               671, y + 13, 2, "SEALED", colour);
        }
    }

    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       59, 423, 1,
                       "D-PAD SELECT  A/L SEAL  R CAMERAS  X/Y CLOSE",
                       GRAPHICS_RGB(127, 171, 124));
}

static bool maintenance_failed(const Game *game, int item)
{
    if (item == MAINT_CAMERA) {
        return game->camera_failed;
    }
    if (item == MAINT_AUDIO) {
        return game->audio_failed;
    }
    if (item == MAINT_VENTILATION) {
        return game->ventilation_failed;
    }
    return false;
}

static void draw_maintenance(const Game *game)
{
    static const char *const labels[MAINTENANCE_ITEM_COUNT] = {
        "CAMERA SYSTEM", "AUDIO DEVICES", "VENTILATION",
        "REBOOT ALL", "EXIT"
    };
