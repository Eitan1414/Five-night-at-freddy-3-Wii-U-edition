/* Keep the PC camera fidelity renderer, but retain the physical monitor frame after opening. */
#define original_ui_draw_camera_feed pc_camera_fidelity_draw_camera_feed_base
#include "main_pc_camera_fidelity_base.inc"
#undef original_ui_draw_camera_feed

static void original_ui_draw_camera_feed(const Game *game)
{
    pc_camera_fidelity_draw_camera_feed_base(game);
    if (!sMonitorV2.transition_active)
        monitor_v2_draw_open_shell();
}
