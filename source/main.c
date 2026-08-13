#include "game/progress_save.h"
#include "game/wiiu_controls.h"
#include "assets/ending_assets.h"
#include "assets/original_ui_assets.h"
#include "assets/monitor_v2_assets.h"
#include "assets/office_assets.h"
#include "assets/achievement_assets.h"
#include "assets/phantom_chica_user_jumpscare.h"
#include "assets/pc_core_visuals.h"
#include "assets/pc_compat_visuals.h"
#include "assets/pc_character_visuals.h"
#include "assets/pc_finishing_visuals.h"
#include "platform/pc_minigame_sfx.h"
#include "platform/runtime_seed.h"

#include "main_v3_parts/main_finishing_prelude.inc"
#include "main_v3_parts/main_complete_prelude.inc"

#define main fnaf3_legacy_main
#define update_game fnaf3_legacy_update_game
#define render_game fnaf3_legacy_render_game

#include "main_v3_parts/main_00.inc"

/* Retail PC restart timing is six seconds for one system and twelve seconds
 * for Reboot All. Override the old fast Wii U scaffolding before the function
 * that consumes these constants is compiled. The final MFA system layer later
 * maps its randomized 0..10 progress onto these compatibility durations. */
#undef REPAIR_SINGLE_FRAMES
#undef REPAIR_ALL_FRAMES
#define REPAIR_SINGLE_FRAMES (6u * 60u)
#define REPAIR_ALL_FRAMES (12u * 60u)

#include "main_v3_parts/main_01.inc"

/* main_00/main_01 split process_phantom_event() across include boundaries.
 * Keep the blackout helpers after main_01 so they are emitted at translation-
 * unit scope, but before main_02 which consumes the fade helpers. */
#include "main_v3_parts/main_pc_blackout_fidelity.inc"
#include "main_v3_parts/main_phantom_visuals.inc"

/* The legacy layer used ten seconds without player input as a ventilation
 * failure. In the PC game the independent no-screen counter instead degrades
 * ventilation health and contributes to Springtrap aggression. */
#undef IDLE_VENT_FAILURE_FRAMES
#define IDLE_VENT_FAILURE_FRAMES UINT32_MAX
#include "main_v3_parts/main_02.inc"
#undef IDLE_VENT_FAILURE_FRAMES
#define IDLE_VENT_FAILURE_FRAMES 600u

/* Use the PC screamer sequences for gameplay timing, rendering and Extras. */
#ifdef gPhantomChicaRealJumpscare
#undef gPhantomChicaRealJumpscare
#endif
#define gSpringtrapJumpscareLeft gPcSpringtrapJumpscare
#define gSpringtrapJumpscareRight gPcSpringtrapJumpscare
#define gPhantomFoxyRealJumpscare gPcPhantomFoxyJumpscare
#define gPhantomBBRealJumpscare gPcPhantomBBJumpscare
#define gPhantomFreddyRealJumpscare gPcPhantomFreddyJumpscare
#define gPhantomChicaRealJumpscare gPcPhantomChicaJumpscare

/* main_04/main_05/main_06 are slices of one translation unit and some slices
 * intentionally end in the middle of a function. Keep them contiguous. */
#include "main_v3_parts/main_03.inc"
#include "main_v3_parts/main_04.inc"
#define graphics_present complete_graphics_present
#include "main_v3_parts/main_05.inc"
#undef graphics_present
#include "main_v3_parts/main_06.inc"

#undef render_game
#undef update_game
#undef main

/* The PC office replacements can only be declared after the legacy source
 * fragments above have closed all of their split function bodies. */
#include "main_v3_parts/main_pc_character_override.inc"

#define main fnaf3_finishing_main
#define update_game fnaf3_finishing_update_game
#define render_game fnaf3_finishing_render_game
#include "main_v3_parts/main_finishing.inc"
#undef render_game
#undef update_game
#undef main

#undef gPhantomChicaRealJumpscare
#undef gPhantomFreddyRealJumpscare
#undef gPhantomBBRealJumpscare
#undef gPhantomFoxyRealJumpscare
#undef gSpringtrapJumpscareRight
#undef gSpringtrapJumpscareLeft

#define main fnaf3_content_main
#define update_game fnaf3_content_update_game
#define render_game fnaf3_content_render_game
#include "main_v3_parts/main_secret_minigames_01.inc"
#include "main_v3_parts/main_secret_minigames_02.inc"
#undef render_game
#undef update_game
#undef main

#include "main_v3_parts/main_complete_title.inc"
#include "main_v3_parts/main_complete_achievements.inc"
#include "main_v3_parts/main_added_ui_font.inc"
#define graphics_draw_text added_ui_draw_text
#define finishing_draw_header added_ui_draw_header
#include "main_v3_parts/main_complete_cheats.inc"
#undef finishing_draw_header
#undef graphics_draw_text
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

#include "main_v3_parts/main_pc_visual_override.inc"
#define texture_draw_rle pc_original_ui_texture_draw_rle
#define update_game original_ui_update_game_v1
#define original_ui_draw_camera_feed original_ui_draw_camera_feed_v1
#include "main_v3_parts/main_original_ui_01.inc"
#undef original_ui_draw_camera_feed
#undef update_game
#undef texture_draw_rle

/* main_original_ui_02 renders the sealing progress before the final system
 * wrapper is defined below. Forward declarations keep that presentation layer
 * independent from the implementation order of the compatibility state. */
static bool pc_vent_seal_is_active(void);
static SpringtrapVent pc_vent_seal_target(void);
static uint32_t pc_vent_seal_progress_frames(void);
static uint32_t pc_vent_seal_duration_frames(void);

#define original_ui_draw_vent_map original_ui_draw_vent_map_v1
#include "main_v3_parts/main_original_ui_02.inc"
#undef original_ui_draw_vent_map

/* Keep the monitor V2 controls/transition, but expose its camera renderer as
 * a fallback so the final PC fidelity layer can restore the exact viewport. */
#define original_ui_draw_camera_feed original_ui_draw_camera_feed_v2
#include "main_v3_parts/main_monitor_v2.inc"
#undef original_ui_draw_camera_feed
#include "main_v3_parts/main_pc_camera_fidelity.inc"

#define draw_springtrap_office pc_draw_springtrap_office
#define draw_phantom_office pc_draw_phantom_office
#include "main_v3_parts/main_authentic_office.inc"
#undef draw_phantom_office
#undef draw_springtrap_office

/* PC endings/Follow Me remain in the finishing layer. */
#include "main_v3_parts/main_pc_finishing_override.inc"

/* Secret minigames use the original 3072x2304 Clickteam object layout. */
#include "main_v3_parts/main_pc_mfa_minigames.inc"
/* Second pass removes debug presentation and uses the remaining exact named
 * object instances from fivenights3-94.mfa. */
#include "main_v3_parts/main_pc_mfa_minigames_v2.inc"
/* Third pass switches to the original hidden hitbox starts, page-snapped
 * Clickteam camera and directly decoded Stage 01 / RWQ event deltas. */
#include "main_v3_parts/main_pc_mfa_minigames_v3.inc"
/* Fourth pass reproduces RWQ's five S-key view positions and the final
 * Active-11 glitch strips decoded from the MFA event sheet. */
#include "main_v3_parts/main_pc_mfa_minigames_v4.inc"
/* Fifth pass replaces free vertical traversal with the original obstacle
 * backdrop collision map, feeler offsets, seven-step jump and gravity. */
#include "main_v3_parts/main_pc_mfa_minigames_v5.inc"
/* Sixth pass avoids replaying V2 audio hooks on non-minigame screens. */
#include "main_v3_parts/main_pc_mfa_minigames_v6.inc"
/* Seventh pass reproduces the original cake/k1-k4/goodend event chain, keeps
 * the hidden bb route persistent and gates balloon collision by that flag. */
#include "main_v3_parts/main_pc_mfa_minigames_v7_state.inc"
#include "main_v3_parts/main_pc_mfa_minigames_v7_update.inc"
/* V7's non-minigame fallback must use the base MFA route; remap the legacy V2
 * wrapper only while this include is parsed so its audio hook cannot run twice. */
#define pc_mfa_v2_exact_update_game pc_mfa_exact_update_game
#include "main_v3_parts/main_pc_mfa_minigames_v7_render.inc"
#undef pc_mfa_v2_exact_update_game
/* V8 uses the exact PC sheet art for threshold animations and releases the
 * non-final 200-count cake sequence after V7 sets its completion flag. */
#include "main_v3_parts/main_pc_mfa_minigames_v8_render.inc"
#include "main_v3_parts/main_pc_mfa_minigames_v8_update.inc"
/* V9 keeps the extracted renderer active through the full Happiest Day finale
 * and removes the last generic Wii U presentation overlays. */
#include "main_v3_parts/main_pc_mfa_minigames_v9_render.inc"
/* V10 keeps the MFA update alive after goodend and removes the Wii U-only
 * confirmation prompt when a hidden night minigame finishes. */
#include "main_v3_parts/main_pc_mfa_minigames_v10_update.inc"
/* V11 keeps Happiest Day as a hidden sixth scene but matches the retail PC
 * Extras page by exposing only the five replayable secret minigames. */
#include "main_v3_parts/main_pc_mfa_minigames_v11_extras.inc"

static void pc_finishing_fallback_render_game(Game *game)
{
    if (pc_mfa_v11_extras_render_override(game)) return;
    if (pc_mfa_v9_secret_render_override(game)) return;
    if (pc_finishing_render_override(game)) return;
    fnaf3_full_audio_render_game(game);
}

static void pc_audio_shutdown_with_extra_sfx(void)
{
    pc_minigame_sfx_shutdown();
    audio_shutdown();
}

/* Final retail-PC maintenance/system counters. This wrapper deliberately sits
 * after the minigame/cheat layers so it can preserve those features while
 * replacing only the old deterministic night-system failures. */
#define pc_mfa_v8_exact_update_game pc_mfa_v11_exact_update_game
#include "main_v3_parts/main_pc_system_fidelity.inc"
#undef pc_mfa_v8_exact_update_game

/* Keep delayed vent closure outside the decoded AI/system counters. Its input
 * wrapper consumes only the legacy instant-seal action, then delegates every
 * gameplay frame through pc_system_fidelity_update_game. */
#include "main_v3_parts/main_pc_vent_seal_fidelity.inc"

/* Wii U-only presentation/control choices are layered last. They never alter
 * the PC AI/state model; they only route the panel displays and translate DRC
 * touch presses into the same gameplay actions. Remap their system call through
 * the vent-seal compatibility wrapper so physical and touch input share it. */
#define pc_system_fidelity_update_game pc_vent_seal_update_game
#include "main_v3_parts/main_wiiu_controls.inc"
#include "main_v3_parts/main_wiiu_controls_v2.inc"
#include "main_v3_parts/main_wiiu_controls_v3.inc"
#include "main_v3_parts/main_wiiu_controls_v4.inc"
#include "main_v3_parts/main_wiiu_controls_v5.inc"
#include "main_v3_parts/main_wiiu_controls_v6.inc"
#undef pc_system_fidelity_update_game

#define update_game wiiu_control_update_game_v6
#define fnaf3_full_audio_render_game wiiu_control_nonoffice_render
#define draw_office_tv draw_authentic_office_tv
#define draw_ventilation_overlay pc_draw_ventilation_overlay
#define audio_shutdown pc_audio_shutdown_with_extra_sfx
#include "main_v3_parts/main_original_ui_03.inc"
#undef audio_shutdown
#undef draw_ventilation_overlay
#undef draw_office_tv
#undef fnaf3_full_audio_render_game
#undef update_game
