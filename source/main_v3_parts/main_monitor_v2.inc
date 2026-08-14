/* Preserve the exact 11-frame PC monitor animation and expose its final open shell. */
#include "main_monitor_v2_base.inc"

static void monitor_v2_draw_open_shell(void)
{
    if (!sMonitorV2.assets_ready) return;
    monitor_v2_draw_texture(MONITOR_V2_TRANSITION_0,
                            sOriginalUiPanelTargets,
                            174, 76, 674, 379);
}
