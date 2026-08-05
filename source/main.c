#include "game/progress_save.h"
#include "assets/ending_assets.h"
#include "assets/original_ui_assets.h"
#include "assets/monitor_v2_assets.h"
#include "assets/camera_springtrap_assets.h"
#include "assets/office_assets.h"

#include "main_v3_parts/main_finishing_prelude.inc"
#include "main_v3_parts/main_complete_prelude.inc"

#define main fnaf3_legacy_main
#define update_game fnaf3_legacy_update_game
#define render_game fnaf3_legacy_render_game

#include "main_v3_parts/main_00.inc"
#include "main_v3_parts/main_01.inc"
#include "main_v3_parts/main_phantom_visuals.inc"
#include "main_v3_parts/main_02.inc"
#include "main_v3_parts/main_03.inc"
#include "main_v3_parts/main_04.inc"

#define graphics_present complete_graphics_present
#include "main_v3_parts/main_05.inc"
#undef graphics_present

#include "main_v3_parts/main_06.inc"

#undef render_game
#undef update_game
#undef main

#define main fnaf3_finishing_main
#define update_game fnaf3_finishing_update_game
#define render_game fnaf3_finishing_render_game
#include "main_v3_parts/main_finishing.inc"
#undef render_game
#undef update_game
#undef main

#define main fnaf3_content_main
#define update_game fnaf3_content_update_game
#define render_game fnaf3_content_render_game
#include "main_v3_parts/main_secret_minigames_01.inc"
#include "main_v3_parts/main_secret_minigames_02.inc"
#undef render_game
#undef update_game
#undef main

#include "main_v3_parts/main_complete_title.inc"
#include "main_v3_parts/main_complete_cheats.inc"
#define main fnaf3_complete_main
#define update_game fnaf3_complete_update_game
#define render_game fnaf3_complete_render_game
#define fnaf3_content_render_game fnaf3_complete_content_render_game
#include "main_v3_parts/main_complete.inc"
#undef fnaf3_content_render_game
#undef render_game
#undef update_game
#undef main

#define main fnaf3_full_audio_main
#define update_game fnaf3_full_audio_update_game
#define render_game fnaf3_full_audio_render_game
#include "main_v3_parts/main_full_audio.inc"
#undef render_game
#undef update_game
#undef main

#define update_game original_ui_update_game_v1
#define original_ui_draw_camera_feed original_ui_draw_camera_feed_v1
#include "main_v3_parts/main_original_ui_01.inc"
#undef original_ui_draw_camera_feed
#undef update_game

#define original_ui_draw_vent_map original_ui_draw_vent_map_v1
#include "main_v3_parts/main_original_ui_02.inc"
#undef original_ui_draw_vent_map

#include "main_v3_parts/main_monitor_v2.inc"
#include "main_v3_parts/main_authentic_office.inc"
#define draw_office_tv draw_authentic_office_tv
#include "main_v3_parts/main_original_ui_03.inc"
#undef draw_office_tv
