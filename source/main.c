#include "game/progress_save.h"
#include "assets/ending_assets.h"
#include "assets/good_ending_pc_asset.h"
#include "assets/original_ui_assets.h"
#include "assets/monitor_v2_assets.h"
#include "assets/camera_springtrap_assets.h"
#include "assets/camera_psx_background_assets.h"
#include "assets/office_assets.h"
#include "assets/minigame_pc_assets.h"
#include "assets/minigame_pc_extended_assets.h"
#include "assets/follow_me_pc_assets.h"

#include "main_v3_parts/main_finishing_prelude.inc"
#include "main_v3_parts/main_complete_prelude.inc"

#define main fnaf3_legacy_main
#define update_game fnaf3_legacy_update_game
#define render_game fnaf3_legacy_render_game

/* main_00.inc owns the canonical kCameraTextures table and is split from
 * main_01.inc inside legacy code. Redirect the ten symbols while that table is
 * compiled instead of inserting a second declaration between split fragments. */
#define gCamera01Texture gPsxCamera01Texture
#define gCamera02Texture gPsxCamera02Texture
#define gCamera03Texture gPsxCamera03Texture
#define gCamera04Texture gPsxCamera04Texture
#define gCamera05Texture gPsxCamera05Texture
#define gCamera06Texture gPsxCamera06Texture
#define gCamera07Texture gPsxCamera07Texture
#define gCamera08Texture gPsxCamera08Texture
#define gCamera09Texture gPsxCamera09Texture
#define gCamera10Texture gPsxCamera10Texture
#define process_phantom_event fnaf3_legacy_process_phantom_event
#include "main_v3_parts/main_00.inc"
#undef gCamera10Texture
#undef gCamera09Texture
#undef gCamera08Texture
#undef gCamera07Texture
#undef gCamera06Texture
#undef gCamera05Texture
#undef gCamera04Texture
#undef gCamera03Texture
#undef gCamera02Texture
#undef gCamera01Texture
#include "main_v3_parts/main_01.inc"
#undef process_phantom_event
#include "main_v3_parts/main_puppet_pc_events.inc"

#include "main_v3_parts/main_phantom_visuals.inc"
#define update_active_game fnaf3_legacy_update_active_game
#include "main_v3_parts/main_02.inc"
#undef update_active_game
#include "main_v3_parts/main_puppet_pc_input.inc"
#include "main_v3_parts/main_03.inc"

/* main_05.inc opens draw_victory() and main_06.inc closes it. Keep all three
 * legacy renderer fragments contiguous at the C level; the touch UI is an
 * overlay and must be declared only after main_06 has returned to file scope. */
#define gPhantomPuppetRealAnimation gPhantomPuppetPcAnimation
#define sequence_ping_pong_frame puppet_pc_sequence_frame
#include "main_v3_parts/main_04.inc"
#undef sequence_ping_pong_frame
#undef gPhantomPuppetRealAnimation
#define graphics_present complete_graphics_present
#include "main_v3_parts/main_05.inc"
#undef graphics_present
#include "main_v3_parts/main_06.inc"
#include "main_v3_parts/main_office_touch_ui.inc"

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

/* Replace only the five post-night Follow Me sequences. Everything else still
 * delegates to the existing finishing/save layer. */
#include "main_v3_parts/main_follow_me_pc.inc"

/* Replace only the Nightmare / Night 6 newspaper presentation. The Bad Ending,
 * Follow Me and all normal finishing/save behaviour still delegate below. */
#include "main_v3_parts/main_night6_newspaper.inc"

/* Keep the original secret-minigame renderer compiled as a safe fallback,
 * but route gameplay and active rendering through the authentic PC pass.
 * The finishing calls inside this layer are redirected through Follow Me and
 * the Night 6 newspaper pass so every finishing layer can coexist. */
#define main fnaf3_content_main_legacy
#define update_game fnaf3_content_update_game
#define render_game fnaf3_content_render_game_legacy
#define fnaf3_finishing_update_game fnaf3_follow_me_update_game
#define fnaf3_finishing_render_game fnaf3_night6_render_game
#define secret_update_active secret_update_active_legacy
#include "main_v3_parts/main_secret_minigames_01.inc"
#undef secret_update_active

#include "main_v3_parts/main_minigame_pc_bb.inc"

/* Keep the simplified ending compiled only as a fallback. The active PC pass
 * below uses the original supplied ending artwork. */
#define secret_update_active secret_update_active_pc
#define secret_draw_good_ending secret_draw_good_ending_legacy
#include "main_v3_parts/main_secret_minigames_02.inc"
#undef secret_draw_good_ending
#undef secret_update_active
#include "main_v3_parts/main_good_ending_pc.inc"
#undef fnaf3_finishing_render_game
#undef fnaf3_finishing_update_game
#undef render_game
#undef update_game
#undef main

static void secret_draw_active_authentic(void)
{
    if (sSecret.good_ending) {
        secret_draw_good_ending();
        graphics_present(GRAPHICS_TARGET_BOTH);
        return;
    }

    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB:
            secret_draw_bb_pc();
            break;
        case SECRET_MINIGAME_MANGLE:
            secret_draw_mangle_pc();
            break;
        case SECRET_MINIGAME_CHICA:
            secret_draw_chica_pc();
            break;
        case SECRET_MINIGAME_STAGE01:
            secret_draw_stage01_pc();
            break;
        case SECRET_MINIGAME_SHADOW_BONNIE:
            secret_draw_shadow_pc();
            break;
        case SECRET_MINIGAME_HAPPIEST_DAY:
            secret_draw_happiest_pc();
            break;
        default:
            break;
    }

    if (sSecret.completed) {
        graphics_draw_rect(GRAPHICS_TARGET_BOTH, 171, 176, 512, 132,
                           GRAPHICS_RGB(4, 4, 4));
        graphics_draw_frame(GRAPHICS_TARGET_BOTH, 171, 176, 512, 132, 4,
                            COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 245, 205, 4,
                           "MINIGAME COMPLETE", COLOUR_WHITE);
        graphics_draw_text(GRAPHICS_TARGET_BOTH, 280, 267, 1,
                           "PROGRESS SAVED - A TO RETURN", COLOUR_GREEN);
    }
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void fnaf3_content_render_game(Game *game)
{
    if (!game->dirty) return;
    if (sSecret.active) {
        game->dirty = false;
        secret_draw_active_authentic();
        return;
    }
    fnaf3_content_render_game_legacy(game);
}

int fnaf3_content_main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    WHBProcInit();
    if (!graphics_init()) {
        WHBProcShutdown();
        return 1;
    }
    input_init();
    frame_clock_reset();
    (void) audio_init();
    memset(&sFinishing, 0, sizeof(sFinishing));
    memset(&sSecret, 0, sizeof(sSecret));
    sTitleMusicPlaying = false;
    Game game;
    game_init(&game);
    fnaf3_content_render_game(&game);
    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);
        fnaf3_content_update_game(&game, &input);
        fnaf3_content_render_game(&game);
        frame_clock_wait_next();
    }
    progress_save_shutdown();
    audio_stop_all();
    audio_shutdown();
    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}

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

/* Add native Wii U GamePad touch handling for the PC click-based secret
 * minigame triggers without replacing the existing controller fallbacks. */
#include "main_v3_parts/main_complete_touch.inc"

#define main fnaf3_full_audio_main
#define update_game fnaf3_full_audio_update_game
#define render_game fnaf3_full_audio_render_game
#define fnaf3_complete_update_game fnaf3_touch_update_game
#include "main_v3_parts/main_full_audio.inc"
#undef fnaf3_complete_update_game
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
