/* Original PC monitor, map, maintenance and transition presentation layer. */

typedef struct OriginalUiState {
    bool assets_ready;
    bool transition_active;
    int transition_frame;
    int transition_step;
    uint32_t transition_ticks;
    uint32_t static_flash_frames;
} OriginalUiState;

static OriginalUiState sOriginalUi;

/* Final Wii U control modes can route the PC panel to TV, GamePad, or both
 * without duplicating the original camera/maintenance renderer. */
static uint32_t sOriginalUiPanelTargets = GRAPHICS_TARGET_GAMEPAD;

static const MapPoint kOriginalCameraUiPoints[CAMERA_COUNT] = {
    {656, 369}, {753, 356}, {817, 320}, {817, 269}, {742, 302},
    {653, 303}, {653, 253}, {716, 242}, {755, 201}, {817, 218},
};

static const TextureRle *original_ui_get(OriginalUiTextureId id)
{
    return original_ui_texture(id);
}

static void original_ui_draw_texture(OriginalUiTextureId id,
                                     uint32_t targets,
                                     int x, int y, int width, int height)
{
    const TextureRle *texture = original_ui_get(id);
    if (texture != NULL)
        texture_draw_rle(targets, x, y, width, height, texture);
}

static void original_ui_start_transition(Game *game, bool opening)
{
    sOriginalUi.transition_active = true;
    sOriginalUi.transition_frame = opening
        ? 0 : ORIGINAL_UI_TRANSITION_6 - ORIGINAL_UI_TRANSITION_0;
    sOriginalUi.transition_step = opening ? 1 : -1;
    sOriginalUi.transition_ticks = 0u;
    sOriginalUi.static_flash_frames = 8u;
    game->dirty = true;
}

static void original_ui_update_effects(Game *game)
{
    if (sOriginalUi.transition_active) {
        ++sOriginalUi.transition_ticks;
        if (sOriginalUi.transition_ticks >= 2u) {
            sOriginalUi.transition_ticks = 0u;
            sOriginalUi.transition_frame += sOriginalUi.transition_step;
            if (sOriginalUi.transition_frame < 0 ||
                sOriginalUi.transition_frame >
                    ORIGINAL_UI_TRANSITION_6 - ORIGINAL_UI_TRANSITION_0) {
                sOriginalUi.transition_active = false;
            }
        }
        game->dirty = true;
    }
    if (sOriginalUi.static_flash_frames > 0u) {
        --sOriginalUi.static_flash_frames;
        game->dirty = true;
    }
}

static void update_game(Game *game, const InputState *input)
{
    const GameScreen before_screen = game->screen;
    const bool before_camera = game->camera_open;
    const bool before_maintenance = game->maintenance_open;
    const bool before_vent_map = game->vent_map_open;
    const int before_camera_index = game->selected_camera;
    const SpringtrapVent before_vent = game->selected_vent;

    fnaf3_full_audio_update_game(game, input);

    if (before_screen == SCREEN_OFFICE && game->screen == SCREEN_OFFICE) {
        if (before_camera != game->camera_open ||
            before_maintenance != game->maintenance_open) {
            const bool opening = game->camera_open || game->maintenance_open;
            original_ui_start_transition(game, opening);
        }
        if (game->camera_open &&
            (before_vent_map != game->vent_map_open ||
             before_camera_index != game->selected_camera ||
             before_vent != game->selected_vent)) {
            sOriginalUi.static_flash_frames = 8u;
            game->dirty = true;
        }
    } else if (game->screen != SCREEN_OFFICE) {
        sOriginalUi.transition_active = false;
        sOriginalUi.static_flash_frames = 0u;
    }

    original_ui_update_effects(game);
}

static void original_ui_draw_toolbar(void)
{
    original_ui_draw_texture(ORIGINAL_UI_TOOLBAR,
                             sOriginalUiPanelTargets,
                             414, 0, 440, 122);
}

static void original_ui_draw_camera_nodes(const Game *game)
{
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const MapPoint point = kOriginalCameraUiPoints[camera];
        const bool selected = camera == game->selected_camera;
        const uint32_t colour = selected
            ? GRAPHICS_RGB(194, 255, 77)
            : GRAPHICS_RGB(215, 220, 215);
        char label[8];
        snprintf(label, sizeof(label), "C%02d", camera + 1);
        graphics_draw_rect(sOriginalUiPanelTargets,
                           point.x - 20, point.y - 9, 40, 18,
                           GRAPHICS_RGB(2, 4, 3));
        graphics_draw_frame(sOriginalUiPanelTargets,
                            point.x - 20, point.y - 9, 40, 18, 1,
                            colour);
        graphics_draw_text(sOriginalUiPanelTargets,
                           point.x - 15, point.y - 4, 1, label, colour);
    }
}

static void original_ui_draw_radar(const Game *game)
{
    if (!sCompleteCheats.radar || !game->camera_open || game->vent_map_open)
        return;

    int camera = -1;
    if (springtrap_ai_is_on_camera(&game->springtrap,
                                   springtrap_ai_camera(&game->springtrap))) {
        camera = springtrap_ai_camera(&game->springtrap);
    } else if (springtrap_ai_is_in_vent(&game->springtrap)) {
        camera = springtrap_vent_source_camera(
            springtrap_ai_current_vent(&game->springtrap));
    }

    if (camera >= 0 && camera < CAMERA_COUNT) {
        const MapPoint point = kOriginalCameraUiPoints[camera];
        graphics_draw_ellipse(sOriginalUiPanelTargets,
                              point.x, point.y, 8, 8, COLOUR_RED);
        graphics_draw_ellipse(sOriginalUiPanelTargets,
                              point.x, point.y, 3, 3, COLOUR_WHITE);
    } else if (springtrap_ai_office_side(&game->springtrap)
               != SPRINGTRAP_OFFICE_NONE) {
        graphics_draw_rect(sOriginalUiPanelTargets, 18, 403, 190, 28,
                           GRAPHICS_RGB(2, 5, 3));
        graphics_draw_text(sOriginalUiPanelTargets, 34, 413, 1,
                           "RADAR: OFFICE", COLOUR_RED);
    }
}

static void original_ui_draw_static(const Game *game, bool full_screen)
{
    const int frame = (int) ((game->night_frames / 2u) % 3u);
    const OriginalUiTextureId id = (OriginalUiTextureId)
        (ORIGINAL_UI_STATIC_0 + frame);
    if (full_screen) {
        original_ui_draw_texture(id, sOriginalUiPanelTargets,
                                 0, 0, 854, 480);
    } else {
        original_ui_draw_texture(id, sOriginalUiPanelTargets,
                                 28, 65, 532, 295);
    }
}

static void original_ui_draw_camera_map(const Game *game)
{
    graphics_draw_rect(sOriginalUiPanelTargets, 577, 155, 277, 305,
                       GRAPHICS_RGB(1, 3, 2));
    graphics_draw_frame(sOriginalUiPanelTargets, 577, 155, 277, 305, 2,
                        GRAPHICS_RGB(63, 94, 67));
    const OriginalUiTextureId map_id =
        ((game->night_frames / 12u) & 1u)
            ? ORIGINAL_UI_CAMERA_MAP_ALT
            : ORIGINAL_UI_CAMERA_MAP_BASE;
    original_ui_draw_texture(map_id, sOriginalUiPanelTargets,
                             591, 177, 252, 235);
    original_ui_draw_camera_nodes(game);
    graphics_draw_text(sOriginalUiPanelTargets, 601, 427, 1,
                       "D-PAD: CAMERA", GRAPHICS_RGB(188, 205, 184));
}

static void original_ui_draw_authentic_springtrap(const Game *game)
{
    static const int x[CAMERA_COUNT] = {
        337, 452, 320, 46, 451, 48, 53, 235, 50, 432
    };
    static const int y[CAMERA_COUNT] = {
        86, 79, 238, 75, 77, 141, 80, 73, 82, 79
    };
    static const int width[CAMERA_COUNT] = {
        166, 83, 188, 90, 85, 100, 168, 141, 196, 118
    };
    static const int height[CAMERA_COUNT] = {
        236, 244, 86, 236, 238, 174, 207, 244, 221, 239
    };

    const int camera = game->selected_camera;
    if (camera < 0 || camera >= CAMERA_COUNT ||
        !springtrap_ai_is_on_camera(&game->springtrap, camera)) {
        return;
    }

    const TextureRle *texture = gOriginalCameraSpringtrapTextures[camera];
    if (texture == NULL) {
        draw_springtrap_camera_sprite(camera);
        return;
    }

    const int jitter =
        (int) ((game->night_frames / 3u + (uint32_t) camera * 5u) % 3u) - 1;
    texture_draw_rle(sOriginalUiPanelTargets,
                     x[camera] + jitter,
                     y[camera] - jitter,
                     width[camera], height[camera], texture);

    if ((game->night_frames % 29u) < 2u) {
        graphics_draw_rect(sOriginalUiPanelTargets,
                           28, y[camera] + height[camera] / 2,
                           532, 2, GRAPHICS_RGB(105, 130, 104));
    }
}

static void original_ui_draw_authentic_phantom(const Game *game)
{
    const PhantomId phantom = phantoms_camera_overlay(
        &game->phantoms, game->selected_camera);
    if (phantom == PHANTOM_NONE) return;

    const int flicker =
        (int) ((game->night_frames / 2u + (uint32_t) phantom) % 5u) - 2;
    switch (phantom) {
        case PHANTOM_BALLOON_BOY: {
            const int bob = phantom_bob(game->night_frames, 24u, 7);
            if (game->phantoms.bb_seen_frames >= 22u) {
                texture_draw_rle(sOriginalUiPanelTargets,
                                 138 - bob, 52 - bob,
                                 334 + bob * 2, 314 + bob * 2,
                                 &gPhantomBBOfficeTexture);
            } else {
                texture_draw_rle(sOriginalUiPanelTargets,
                                 174 + bob / 2, 82 - bob,
                                 262, 262,
                                 &gPhantomBBCameraTexture);
            }
            break;
        }
        case PHANTOM_CHICA:
            texture_draw_rle(sOriginalUiPanelTargets,
                             350 + flicker, 210 - flicker,
                             184, 108,
                             &gPhantomChicaCameraTexture);
            break;
        case PHANTOM_MANGLE:
            texture_draw_rle(sOriginalUiPanelTargets,
                             56 + flicker, 64 - flicker,
                             400, 286,
                             &gPhantomMangleCameraTexture);
            break;
        case PHANTOM_PUPPET:
            texture_draw_rle(sOriginalUiPanelTargets,
                             205 + flicker, 45 - flicker,
                             205, 320,
                             &gPhantomPuppetCameraTexture);
            break;
        default:
            break;
    }

    if ((game->night_frames % 11u) < 3u) {
        const int offset = (int) (game->night_frames % 47u);
        graphics_draw_rect(sOriginalUiPanelTargets,
                           28, 86 + offset * 5 % 250,
                           532, 3, GRAPHICS_RGB(121, 143, 120));
    }
}

static void original_ui_draw_camera_feed(const Game *game)
{
    graphics_clear(sOriginalUiPanelTargets, GRAPHICS_RGB(1, 4, 2));
    original_ui_draw_toolbar();
    if (sFinishing.phone_active)
        original_ui_draw_texture(ORIGINAL_UI_MUTE_CALL,
                                 sOriginalUiPanelTargets,
                                 306, 100, 94, 22);

    char label[16];
    snprintf(label, sizeof(label), "CAM %02d", game->selected_camera + 1);
    graphics_draw_rect(sOriginalUiPanelTargets, 17, 13, 194, 33,
                       GRAPHICS_RGB(1, 5, 2));
    graphics_draw_text(sOriginalUiPanelTargets, 26, 23, 2,
                       label, COLOUR_GREEN);
    graphics_draw_frame(sOriginalUiPanelTargets, 18, 55, 552, 315, 3,
                        GRAPHICS_RGB(61, 112, 62));

    if (game->camera_failed) {
        original_ui_draw_static(game, false);
        graphics_draw_rect(sOriginalUiPanelTargets, 74, 155, 438, 70,
                           GRAPHICS_RGB(2, 2, 2));
        graphics_draw_text(sOriginalUiPanelTargets, 107, 173, 4,
                           "VIDEO ERROR", COLOUR_RED);
    } else {
        graphics_draw_rect(sOriginalUiPanelTargets, 28, 65, 532, 295,
                           COLOUR_BLACK);
        texture_draw_rle(sOriginalUiPanelTargets, 28, 65, 532, 295,
                         kCameraTextures[game->selected_camera]);
        original_ui_draw_authentic_springtrap(game);
        original_ui_draw_authentic_phantom(game);
        for (int y_scan = 70; y_scan < 360; y_scan += 9)
            graphics_draw_rect(sOriginalUiPanelTargets,
                               28, y_scan, 532, 1,
                               GRAPHICS_RGB(5, 10, 6));
    }

    original_ui_draw_camera_map(game);
    original_ui_draw_radar(game);

    graphics_draw_rect(sOriginalUiPanelTargets, 17, 381, 545, 80,
                       GRAPHICS_RGB(1, 5, 2));
    graphics_draw_frame(sOriginalUiPanelTargets, 17, 381, 545, 80, 2,
                        GRAPHICS_RGB(48, 83, 50));
    graphics_draw_text(sOriginalUiPanelTargets, 29, 394, 2,
                       message_text(game),
                       game->message == MESSAGE_AUDIO_FAILED
                           ? COLOUR_RED : COLOUR_GREEN);
    graphics_draw_text(sOriginalUiPanelTargets, 29, 435, 1,
                       "A: PLAY AUDIO   R: VENT MAP   X/Y: CLOSE",
                       GRAPHICS_RGB(171, 198, 166));

    if (sOriginalUi.static_flash_frames > 0u)
        original_ui_draw_static(game, true);
}

