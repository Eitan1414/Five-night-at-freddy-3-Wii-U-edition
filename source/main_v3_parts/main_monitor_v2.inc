/* Exact PC monitor controls, eleven-frame panel animation and authentic camera/vent frames. */

#include "assets/pc_camera_frame_assets.h"

typedef struct MonitorV2State {
    bool assets_ready;
    bool transition_active;
    int transition_frame;
    int transition_step;
    uint32_t transition_ticks;
} MonitorV2State;

static MonitorV2State sMonitorV2;

static const TextureRle *monitor_v2_get(MonitorV2TextureId id)
{
    if (!sMonitorV2.assets_ready) return NULL;
    return monitor_v2_texture(id);
}

static void monitor_v2_draw_texture(MonitorV2TextureId id,
                                    uint32_t targets,
                                    int x, int y, int width, int height)
{
    const TextureRle *texture = monitor_v2_get(id);
    if (texture != NULL)
        texture_draw_rle(targets, x, y, width, height, texture);
}

static bool monitor_v2_touch_pressed_rect(const InputState *input,
                                          int x, int y,
                                          int width, int height)
{
    return input != NULL && input->touch_pressed &&
           input->touch_x >= x && input->touch_x < x + width &&
           input->touch_y >= y && input->touch_y < y + height;
}

static void monitor_v2_start_transition(Game *game, bool opening)
{
    sMonitorV2.transition_active = true;
    sMonitorV2.transition_frame = opening ? 10 : 0;
    sMonitorV2.transition_step = opening ? -1 : 1;
    sMonitorV2.transition_ticks = 0u;
    game->dirty = true;
}

static void monitor_v2_update_transition(Game *game)
{
    if (!sMonitorV2.assets_ready || !sMonitorV2.transition_active) return;

    ++sMonitorV2.transition_ticks;
    if (sMonitorV2.transition_ticks >= 2u) {
        sMonitorV2.transition_ticks = 0u;
        sMonitorV2.transition_frame += sMonitorV2.transition_step;
        if (sMonitorV2.transition_frame < 0 ||
            sMonitorV2.transition_frame > 10) {
            sMonitorV2.transition_active = false;
        }
    }
    game->dirty = true;
}

static InputState monitor_v2_route_touch_buttons(const Game *game,
                                                  const InputState *input)
{
    InputState routed = *input;
    if (game->screen != SCREEN_OFFICE || !game->camera_open ||
        !input->touch_pressed) {
        return routed;
    }

    if (!game->vent_map_open &&
        monitor_v2_touch_pressed_rect(input, 555, 9, 118, 64)) {
        routed.held |= GAME_BUTTON_CONFIRM;
        routed.pressed |= GAME_BUTTON_CONFIRM;
    }
    if (monitor_v2_touch_pressed_rect(input, 699, 7, 140, 70)) {
        routed.held |= GAME_BUTTON_MODE;
        routed.pressed |= GAME_BUTTON_MODE;
    }
    return routed;
}

static void monitor_v2_apply_touch_camera_node(Game *game,
                                                const InputState *input)
{
    if (game->screen != SCREEN_OFFICE || !game->camera_open ||
        game->vent_map_open || game->camera_failed ||
        input == NULL || !input->touch_pressed) {
        return;
    }

    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const MapPoint point = kOriginalCameraUiPoints[camera];
        if (!monitor_v2_touch_pressed_rect(input,
                                           point.x - 24, point.y - 14,
                                           48, 28)) {
            continue;
        }
        if (game->selected_camera != camera) {
            game->selected_camera = camera;
            sOriginalUi.static_flash_frames = 8u;
            content_play_select();
            game->dirty = true;
        }
        return;
    }
}

static void update_game(Game *game, const InputState *input)
{
    const GameScreen before_screen = game->screen;
    const bool before_camera = game->camera_open;
    const bool before_maintenance = game->maintenance_open;
    const InputState routed = monitor_v2_route_touch_buttons(game, input);

    original_ui_update_game_v1(game, &routed);
    monitor_v2_apply_touch_camera_node(game, input);

    if (sMonitorV2.assets_ready &&
        before_screen == SCREEN_OFFICE && game->screen == SCREEN_OFFICE &&
        (before_camera != game->camera_open ||
         before_maintenance != game->maintenance_open)) {
        const bool opening = game->camera_open || game->maintenance_open;
        monitor_v2_start_transition(game, opening);
    } else if (game->screen != SCREEN_OFFICE) {
        sMonitorV2.transition_active = false;
        sMonitorV2.transition_ticks = 0u;
    }

    monitor_v2_update_transition(game);
}

static void monitor_v2_draw_controls(void)
{
    if (!sMonitorV2.assets_ready || sOriginalUi.static_flash_frames > 0u)
        return;

    monitor_v2_draw_texture(MONITOR_V2_PLAY_AUDIO,
                            GRAPHICS_TARGET_GAMEPAD,
                            555, 9, 118, 64);
    monitor_v2_draw_texture(MONITOR_V2_MAP_TOGGLE,
                            GRAPHICS_TARGET_GAMEPAD,
                            699, 7, 140, 70);
}

static const TextureRle *pc_frame_from_set(const PcCameraFrameSet *set,
                                            const Game *game,
                                            int salt)
{
    if (set == NULL || set->frames == NULL || set->count == 0u) return NULL;
    const uint32_t phase = game->night_frames / 12u + (uint32_t) salt;
    return set->frames[phase % set->count];
}

static const TextureRle *pc_camera_frame(const Game *game,
                                         PhantomId *overlay_phantom)
{
    const int camera = game->selected_camera;
    if (overlay_phantom != NULL) *overlay_phantom = PHANTOM_NONE;
    if (camera < 0 || camera >= CAMERA_COUNT) return NULL;

    if (springtrap_ai_is_on_camera(&game->springtrap, camera)) {
        const SpringtrapCameraPosition position =
            springtrap_ai_camera_position(&game->springtrap);
        const PcCameraFrameSet *set = position == SPRINGTRAP_CAMERA_POSITION_BACK
            ? &gPcCameraBackSets[camera]
            : &gPcCameraExitSets[camera];
        return pc_frame_from_set(set, game, camera * 3 + (int) position);
    }

    const PhantomId phantom = phantoms_camera_overlay(
        &game->phantoms, camera);
    const bool has_full_phantom_frame =
        (phantom == PHANTOM_MANGLE && camera == 3) ||
        (phantom == PHANTOM_CHICA && camera == 6) ||
        (phantom == PHANTOM_PUPPET && camera == 7);
    if (has_full_phantom_frame) {
        const TextureRle *frame = pc_frame_from_set(
            &gPcCameraPhantomSets[camera], game, camera + (int) phantom * 5);
        if (frame != NULL) return frame;
    }

    if (overlay_phantom != NULL) *overlay_phantom = phantom;
    return pc_frame_from_set(&gPcCameraBaseSets[camera], game, camera);
}

static void original_ui_draw_camera_feed(const Game *game)
{
    PhantomId overlay_phantom = PHANTOM_NONE;
    const TextureRle *pc_frame = pc_camera_frame(game, &overlay_phantom);

    /* If the user payload is not installed yet, preserve the previous camera
     * renderer exactly so source checkouts remain buildable. */
    if (pc_frame == NULL) {
        original_ui_draw_camera_feed_v1(game);
        monitor_v2_draw_controls();
        return;
    }

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    original_ui_draw_toolbar();
    if (sFinishing.phone_active)
        original_ui_draw_texture(ORIGINAL_UI_MUTE_CALL,
                                 GRAPHICS_TARGET_GAMEPAD,
                                 306, 100, 94, 22);

    char label[16];
    snprintf(label, sizeof(label), "CAM %02d", game->selected_camera + 1);
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 17, 13, 194, 33,
                       GRAPHICS_RGB(1, 5, 2));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 26, 23, 2,
                       label, COLOUR_GREEN);
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 18, 55, 552, 315, 3,
                        GRAPHICS_RGB(61, 112, 62));

    if (game->camera_failed) {
        original_ui_draw_static(game, false);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 74, 155, 438, 70,
                           GRAPHICS_RGB(2, 2, 2));
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 107, 173, 4,
                           "VIDEO ERROR", COLOUR_RED);
    } else {
        /* The supplied PC frames are 825x650. Keep that aspect ratio instead
         * of stretching Springtrap on the Wii U's wider GamePad feed. */
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 28, 65, 532, 295,
                           COLOUR_BLACK);
        texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 107, 65, 374, 295,
                         pc_frame);

        /* BB has no full-frame room sprite in the supplied sheets, so retain
         * the authentic existing BB overlay on top of the supplied room. */
        if (overlay_phantom == PHANTOM_BALLOON_BOY)
            original_ui_draw_authentic_phantom(game);

        for (int y_scan = 70; y_scan < 360; y_scan += 9)
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               28, y_scan, 532, 1,
                               GRAPHICS_RGB(5, 10, 6));
    }

    original_ui_draw_camera_map(game);
    original_ui_draw_radar(game);

    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 17, 381, 545, 80,
                       GRAPHICS_RGB(1, 5, 2));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 17, 381, 545, 80, 2,
                        GRAPHICS_RGB(48, 83, 50));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 29, 394, 2,
                       message_text(game),
                       game->message == MESSAGE_AUDIO_FAILED
                           ? COLOUR_RED : COLOUR_GREEN);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 29, 435, 1,
                       "A: PLAY AUDIO   R: VENT MAP   X/Y: CLOSE",
                       GRAPHICS_RGB(171, 198, 166));

    if (sOriginalUi.static_flash_frames > 0u)
        original_ui_draw_static(game, true);
    monitor_v2_draw_controls();
}

static void original_ui_draw_vent_map(const Game *game)
{
    const int vent = (int) game->selected_vent;
    const bool valid_vent = vent >= 0 && vent < VENT_COUNT;
    const bool occupied = valid_vent &&
        springtrap_ai_is_in_vent(&game->springtrap) &&
        springtrap_ai_current_vent(&game->springtrap) == game->selected_vent;
    const TextureRle *pc_frame = valid_vent
        ? (occupied ? gPcVentSpringtrapTextures[vent]
                    : gPcVentEmptyTextures[vent])
        : NULL;

    if (pc_frame == NULL) {
        original_ui_draw_vent_map_v1(game);
        monitor_v2_draw_controls();
        return;
    }

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(1, 4, 2));
    original_ui_draw_toolbar();
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 25, 20, 3,
                       "VENTILATION MAP", COLOUR_GREEN);

    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 385, 55, 455, 413, 3,
                        GRAPHICS_RGB(62, 100, 65));
    const OriginalUiTextureId map_id =
        ((game->night_frames / 10u) & 1u)
            ? ORIGINAL_UI_VENT_MAP_ALT
            : ORIGINAL_UI_VENT_MAP_BASE;
    original_ui_draw_texture(map_id, GRAPHICS_TARGET_GAMEPAD,
                             397, 68, 430, 401);

    uint32_t feed_colour = GRAPHICS_RGB(106, 146, 105);
    if (game->sealed_vent == game->selected_vent)
        feed_colour = GRAPHICS_RGB(194, 255, 77);
    if (occupied && sCompleteCheats.radar) feed_colour = COLOUR_RED;

    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD, 20, 82, 342, 272,
                       GRAPHICS_RGB(1, 5, 2));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD, 20, 82, 342, 272, 3,
                        feed_colour);
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD, 24, 86, 334, 263,
                     pc_frame);

    char vent_label[24];
    snprintf(vent_label, sizeof(vent_label), "CAM %02d", vent + 11);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 27, 369, 2,
                       vent_label, feed_colour);
    if (game->sealed_vent == game->selected_vent)
        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 251, 369, 2,
                           "SEALED", feed_colour);
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 27, 407, 1,
                       "D-PAD: SELECT   A/L: SEAL",
                       GRAPHICS_RGB(181, 202, 176));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD, 27, 431, 1,
                       "R: CAMERA MAP   X/Y: CLOSE",
                       GRAPHICS_RGB(181, 202, 176));

    if (sOriginalUi.static_flash_frames > 0u)
        original_ui_draw_static(game, true);
    monitor_v2_draw_controls();
}

static void monitor_v2_draw_transition(void)
{
    if (!sMonitorV2.assets_ready || !sMonitorV2.transition_active ||
        sMonitorV2.transition_frame < 0 ||
        sMonitorV2.transition_frame > 10) {
        return;
    }

    const MonitorV2TextureId id = (MonitorV2TextureId)
        (MONITOR_V2_TRANSITION_0 + sMonitorV2.transition_frame);
    monitor_v2_draw_texture(id, GRAPHICS_TARGET_GAMEPAD,
                            0, 0, 854, 480);
}
