#include "game/progress_save.h"
#include "assets/ending_assets.h"

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
#define fnaf3_content_render_game fnaf3_complete_content_render_game
#include "main_v3_parts/main_complete.inc"
#undef fnaf3_content_render_game
