/* Exact PC monitor controls and eleven-frame panel animation. */

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
        const bool finished_opening = sMonitorV2.transition_step < 0;
        sMonitorV2.transition_frame += sMonitorV2.transition_step;
        if (sMonitorV2.transition_frame < 0 ||
            sMonitorV2.transition_frame > 10) {
            sMonitorV2.transition_active = false;
            /* PC MFA groups 721-724 reroll Phantom Foxy when the monitor's
             * opening animation actually finishes, not on the button press. */
            if (finished_opening && game->camera_open) {
                process_phantom_event(
                    game,
                    phantoms_on_monitor_animation_finished(&game->phantoms));
            }
        }
    }
    game->dirty = true;
}

static void update_game(Game *game, const InputState *input)
{
    const GameScreen before_screen = game->screen;
    const bool before_camera = game->camera_open;
    const bool before_maintenance = game->maintenance_open;

    original_ui_update_game_v1(game, input);

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

static void monitor_v2_draw_camera_controls(void)
{
    if (!sMonitorV2.assets_ready) return;

    /* PC layout: the two square controls are stacked below the camera image,
     * immediately left of the floor map. */
    monitor_v2_draw_texture(MONITOR_V2_PLAY_AUDIO,
                            GRAPHICS_TARGET_GAMEPAD,
                            360, 312, 118, 64);
    monitor_v2_draw_texture(MONITOR_V2_MAP_TOGGLE,
                            GRAPHICS_TARGET_GAMEPAD,
                            349, 382, 140, 70);
}

static void monitor_v2_draw_vent_controls(void)
{
    if (!sMonitorV2.assets_ready) return;

    /* The ventilation map uses the map-toggle control, not Play Audio. */
    monitor_v2_draw_texture(MONITOR_V2_MAP_TOGGLE,
                            GRAPHICS_TARGET_GAMEPAD,
                            349, 382, 140, 70);
}

/* Keep the historical helper name for the final camera fidelity layer. */
static void monitor_v2_draw_controls(void)
{
    monitor_v2_draw_camera_controls();
}

static void original_ui_draw_camera_feed(const Game *game)
{
    original_ui_draw_camera_feed_v1(game);
    monitor_v2_draw_camera_controls();
}

static void original_ui_draw_vent_map(const Game *game)
{
    original_ui_draw_vent_map_v1(game);
    monitor_v2_draw_vent_controls();
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

    /* Never stretch the transition across the whole GamePad.  The office must
     * remain visible around the physical monitor/panel just like on PC. */
    monitor_v2_draw_texture(id, GRAPHICS_TARGET_GAMEPAD,
                            174, 76, 674, 379);
}
