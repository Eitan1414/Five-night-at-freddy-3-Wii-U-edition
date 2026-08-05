        case PHANTOM_CHICA:
            texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 390, 230, 130, 90, &gPhantomChicaCameraTexture); break;
        case PHANTOM_MANGLE:
            texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 65, 90, 300, 245, &gPhantomMangleCameraTexture); break;
        case PHANTOM_PUPPET:
            texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 235, 80, 145, 260, &gPhantomPuppetCameraTexture); break;
        default: break;
    }
}

static void draw_camera_feed(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    if (game->camera_failed) {
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 58, 67, 738, 306, 5, COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 123, 132, 5, "CAMERA SYSTEM ERROR", COLOUR_RED);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 191, 232, 3, "SIGNAL LOST", COLOUR_WHITE);
        return;
    }
    char label[16]; snprintf(label, sizeof(label), "CAM %02d", game->selected_camera + 1);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 23, 18, 3, label, COLOUR_GREEN);
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 18, 55, 552, 315, 4, GRAPHICS_RGB(61, 112, 62));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 28, 65, 532, 295, COLOUR_BLACK);
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 28, 65, 532, 295, kCameraTextures[game->selected_camera]);
    if (springtrap_ai_is_on_camera(&game->springtrap, game->selected_camera)) {
        draw_springtrap_camera_sprite(game->selected_camera);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 320, 338, 2, "MOTION DETECTED", COLOUR_RED);
    }
    draw_phantom_camera_overlay(game);
    for (int y = 70; y < 360; y += 10)
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 28, y, 532, 1, GRAPHICS_RGB(5, 10, 6));
    draw_camera_map(game);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 29, 392, 2, message_text(game),
                       game->message == MESSAGE_AUDIO_FAILED ? COLOUR_RED : COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 29, 432, 1, "D-PAD SELECT  A LURE  R VENTS  X/Y CLOSE", GRAPHICS_RGB(127, 171, 124));
}

static void draw_vent_map(const Game *game)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 26, 20, 3, "VENTILATION MAP", COLOUR_GREEN);
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 28, 62, 798, 338, 4, GRAPHICS_RGB(61, 112, 62));
    for (int vent = 0; vent < VENT_COUNT; ++vent) {
        const int y = 91 + vent * 55;
        const bool selected = vent == (int) game->selected_vent;
        const bool sealed = vent == (int) game->sealed_vent;
        const bool occupied = springtrap_ai_is_in_vent(&game->springtrap) && springtrap_ai_current_vent(&game->springtrap) == (SpringtrapVent) vent;
        uint32_t colour = selected ? COLOUR_WHITE : GRAPHICS_RGB(97, 137, 96);
        if (sealed) colour = GRAPHICS_RGB(152, 215, 107);
        if (occupied) colour = COLOUR_RED;
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 59, y, 730, 39, 3, colour);
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 78, y + 13, 2, springtrap_vent_label((SpringtrapVent) vent), colour);
        if (sealed) graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 671, y + 13, 2, "SEALED", colour);
    }
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 59, 423, 1, "D-PAD SELECT  A/L SEAL  R CAMERAS  X/Y CLOSE", GRAPHICS_RGB(127, 171, 124));
}

static bool maintenance_failed(const Game *game, int item)
{
    if (item == MAINT_CAMERA) return game->camera_failed;
    if (item == MAINT_AUDIO) return game->audio_failed;
    if (item == MAINT_VENTILATION) return game->ventilation_failed;
    return false;
}

static void draw_maintenance(const Game *game)
{
    static const char *const labels[MAINTENANCE_ITEM_COUNT] = {
        "CAMERA SYSTEM", "AUDIO DEVICES", "VENTILATION", "REBOOT ALL", "EXIT"
    };
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 5, 3));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 38, 31, 778, 412, 4, GRAPHICS_RGB(54, 112, 59));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 81, 58, 4, "MAINTENANCE PANEL", COLOUR_GREEN);
    for (int item = 0; item < MAINTENANCE_ITEM_COUNT; ++item) {
        const int y = 126 + item * 55;
        const bool selected = item == game->maintenance_selection;
        if (selected) graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 76, y - 12, 701, 40, GRAPHICS_RGB(24, 60, 29));
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 101, y, 3, labels[item], selected ? COLOUR_WHITE : GRAPHICS_RGB(111, 148, 108));
        if (item < MAINT_REBOOT_ALL) {
            const bool failed = maintenance_failed(game, item);
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 602, y, 2, failed ? "ERROR" : "ONLINE", failed ? COLOUR_RED : COLOUR_GREEN);
        }
    }
    if (game->repairing) {
        const uint32_t duration = repair_duration(game);
        const int progress = (int) ((game->repair_frames * 626u) / duration);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 103, 406, 626, 13, GRAPHICS_RGB(9, 20, 11));
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 103, 406, progress, 13, GRAPHICS_RGB(89, 184, 85));
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 303, 375, 2, "REBOOTING...", COLOUR_GREEN);
    }
}

static void draw_ventilation_overlay(const Game *game)
{
    if (!game->ventilation_failed) return;
    const uint32_t phase = game->ventilation_error_frames % VENT_BLACKOUT_CYCLE_FRAMES;
    if (phase < VENT_DIM_START_FRAMES) return;
    if (phase >= VENT_BLACK_START_FRAMES) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 0, 854, 480, COLOUR_BLACK); return;
    }
    const uint32_t progress = phase - VENT_DIM_START_FRAMES;
    int spacing = progress > 120u ? 3 : progress > 60u ? 6 : 10;
    for (int y = 0; y < 480; y += spacing)
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, y, 854, spacing / 2 + 1, COLOUR_BLACK);
}

static const JumpscareSequence *phantom_jumpscare_sequence(PhantomId phantom)
{
    switch (phantom) {
        case PHANTOM_FOXY: return &gPhantomFoxyRealJumpscare;
        case PHANTOM_BALLOON_BOY: return &gPhantomBBRealJumpscare;
        case PHANTOM_FREDDY: return &gPhantomFreddyRealJumpscare;
        case PHANTOM_CHICA: return &gPhantomChicaRealJumpscare;
        default: return NULL;
    }
}

static void draw_jumpscare_sequence(uint32_t targets,
                                    const JumpscareSequence *sequence,
                                    uint32_t timer)
{
    if (sequence == NULL || sequence->frames == NULL ||
        sequence->frame_count == 0u || sequence->ticks_per_frame == 0u) {
        return;
    }

    uint32_t frame = timer / sequence->ticks_per_frame;
    if (frame >= sequence->frame_count) frame = sequence->frame_count - 1u;
    const TextureRle *texture = sequence->frames[frame];
    if (texture == NULL || texture->width == 0u || texture->height == 0u) return;

    int draw_height = GRAPHICS_LOGICAL_HEIGHT;
    int draw_width = (int) (((uint32_t) texture->width *
                            (uint32_t) draw_height) / texture->height);
    if (draw_width > GRAPHICS_LOGICAL_WIDTH) {
        draw_width = GRAPHICS_LOGICAL_WIDTH;
        draw_height = (int) (((uint32_t) texture->height *
                             (uint32_t) draw_width) / texture->width);
    }
    const int x = (GRAPHICS_LOGICAL_WIDTH - draw_width) / 2;
    const int y = (GRAPHICS_LOGICAL_HEIGHT - draw_height) / 2;

    graphics_clear(targets, COLOUR_BLACK);
    texture_draw_rle(targets, x, y, draw_width, draw_height, texture);
}

static void draw_springtrap_jumpscare(const Game *game)
{
    const JumpscareSequence *sequence = game->springtrap_jumpscare_right
        ? &gSpringtrapJumpscareRight
        : &gSpringtrapJumpscareLeft;
    draw_jumpscare_sequence(GRAPHICS_TARGET_BOTH, sequence, game->screen_frames);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_phantom_jumpscare(const Game *game)
{
    const PhantomId phantom = phantoms_active_jumpscare(&game->phantoms);
    if (phantom == PHANTOM_NONE) return;
    const JumpscareSequence *sequence = phantom_jumpscare_sequence(phantom);
    if (sequence == NULL) return;
    draw_jumpscare_sequence(GRAPHICS_TARGET_BOTH,
                            sequence,
                            phantoms_jumpscare_frames(&game->phantoms));
}

static void draw_office(const Game *game)
{
    draw_office_tv(game);
    if (game->maintenance_open) draw_maintenance(game);
    else if (game->camera_open) {
        if (game->vent_map_open) draw_vent_map(game); else draw_camera_feed(game);
    } else draw_office_gamepad(game);
    draw_ventilation_overlay(game);
    draw_phantom_jumpscare(game);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_victory(const Game *game)
{
    const bool six = game->screen_frames >= 90u;
