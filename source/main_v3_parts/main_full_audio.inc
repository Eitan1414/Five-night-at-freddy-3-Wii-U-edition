/* Full original FNaF 3 sound-pack integration layer. */

#include "platform/intro.h"

static AudioCue full_audio_secret_music(SecretMinigameKind kind)
{
    switch (kind) {
        case SECRET_MINIGAME_BB: return AUDIO_CUE_MINIGAME_BB_MUSIC;
        case SECRET_MINIGAME_MANGLE: return AUDIO_CUE_MINIGAME_MANGLE_MUSIC;
        case SECRET_MINIGAME_CHICA: return AUDIO_CUE_MINIGAME_CHICA_MUSIC;
        case SECRET_MINIGAME_STAGE01: return AUDIO_CUE_MINIGAME_STAGE01_MUSIC;
        case SECRET_MINIGAME_SHADOW_BONNIE: return AUDIO_CUE_MINIGAME_SHADOW_MUSIC;
        case SECRET_MINIGAME_HAPPIEST_DAY: return AUDIO_CUE_MINIGAME_HAPPIEST_MUSIC;
        default: return AUDIO_CUE_MINIGAME_BB_MUSIC;
    }
}

static void full_audio_stop_secret_music(void)
{
    for (int cue = AUDIO_CUE_MINIGAME_BB_MUSIC;
         cue <= AUDIO_CUE_MINIGAME_HAPPIEST_MUSIC; ++cue) {
        audio_stop((AudioCue) cue);
    }
}

static void full_audio_start_office_ambience(void)
{
    audio_restart(AUDIO_CUE_OFFICE_FAN, 0.30f, true);
    audio_restart(AUDIO_CUE_RAIN_AMBIENCE, 0.13f, true);
}

static void full_audio_stop_office_ambience(void)
{
    audio_stop(AUDIO_CUE_OFFICE_FAN);
    audio_stop(AUDIO_CUE_RAIN_AMBIENCE);
    audio_stop(AUDIO_CUE_DANGER);
    audio_stop(AUDIO_CUE_REPAIR_SCANNER);
}

static void full_audio_start_secret_minigame(void)
{
    full_audio_stop_office_ambience();
    full_audio_stop_secret_music();
    audio_play(AUDIO_CUE_MINIGAME_GLITCH, 0.72f, false);
    audio_restart(full_audio_secret_music(sSecret.kind), 0.48f, true);
}

static void full_audio_finish_secret_minigame(void)
{
    full_audio_stop_secret_music();
    audio_stop(AUDIO_CUE_END);
    audio_play(AUDIO_CUE_MINIGAME_CHIMES, 0.72f, false);
    if (sSecret.good_ending) {
        audio_play(AUDIO_CUE_MINIGAME_CROWD, 0.54f, false);
        audio_play(AUDIO_CUE_MINIGAME_PARTY_FAVOR, 0.78f, false);
    }
}

static void full_audio_handle_finishing_mode(FinishingMode before_mode,
                                              FinishingMode after_mode)
{
    if (before_mode == after_mode) return;

    if (before_mode == FINISHING_FOLLOW_ME ||
        before_mode == FINISHING_NIGHT5_ENDING ||
        before_mode == FINISHING_NIGHT6_ENDING) {
        audio_stop(AUDIO_CUE_ENDING_DESOLATE);
        audio_stop(AUDIO_CUE_RAIN_AMBIENCE);
    }

    if (after_mode == FINISHING_FOLLOW_ME) {
        full_audio_stop_office_ambience();
        audio_play(AUDIO_CUE_MINIGAME_GLITCH, 0.65f, false);
        audio_restart(AUDIO_CUE_ENDING_DESOLATE, 0.30f, true);
    } else if (after_mode == FINISHING_NIGHT5_ENDING) {
        full_audio_stop_office_ambience();
        audio_restart(AUDIO_CUE_ENDING_DESOLATE, 0.42f, true);
    } else if (after_mode == FINISHING_NIGHT6_ENDING) {
        full_audio_stop_office_ambience();
        audio_restart(AUDIO_CUE_RAIN_AMBIENCE, 0.34f, true);
    }
}

static void update_game(Game *game, const InputState *input)
{
    const GameScreen before_screen = game->screen;
    const MessageKind before_message = game->message;
    const bool before_repairing = game->repairing;
    const bool before_secret_active = sSecret.active;
    const bool before_secret_completed = sSecret.completed;
    const uint8_t before_collected = sSecret.collected;
    const int before_delivered = sSecret.delivered;
    const uint32_t before_follow_event = sFinishing.minigame_event_frames;
    const FinishingMode before_mode = sFinishing.mode;

    fnaf3_complete_update_game(game, input);

    if (!before_secret_active && sSecret.active) {
        full_audio_start_secret_minigame();
    } else if (before_secret_active && !sSecret.active) {
        full_audio_stop_secret_music();
        if (game->screen == SCREEN_OFFICE)
            full_audio_start_office_ambience();
    }

    if (sSecret.active && !sSecret.completed) {
        if (sSecret.delivered > before_delivered) {
            audio_play(AUDIO_CUE_MINIGAME_FEED, 0.72f, false);
        } else if (sSecret.collected != before_collected) {
            audio_play(AUDIO_CUE_MINIGAME_COLLECT, 0.72f, false);
        }
    }

    if (!before_secret_completed && sSecret.completed)
        full_audio_finish_secret_minigame();

    if (before_follow_event == 0u && sFinishing.minigame_event_frames > 0u)
        audio_play(AUDIO_CUE_MINIGAME_CRUSH, 0.72f, false);

    full_audio_handle_finishing_mode(before_mode, sFinishing.mode);

    if (!before_repairing && game->repairing) {
        audio_stop(AUDIO_CUE_WAIT);
        audio_restart(AUDIO_CUE_REPAIR_SCANNER, 0.52f, true);
    } else if (before_repairing && !game->repairing) {
        audio_stop(AUDIO_CUE_REPAIR_SCANNER);
        if (game->screen == SCREEN_OFFICE)
            audio_play(AUDIO_CUE_REPAIR_DONE, 0.82f, false);
    }

    if (before_message != game->message &&
        (game->message == MESSAGE_DANGER_SOFT ||
         game->message == MESSAGE_DANGER_LOUD)) {
        audio_restart(AUDIO_CUE_DANGER,
                      game->message == MESSAGE_DANGER_LOUD ? 0.74f : 0.48f,
                      false);
    }

    if (before_screen != game->screen) {
        if (before_screen == SCREEN_OFFICE)
            full_audio_stop_office_ambience();
        if (game->screen == SCREEN_OFFICE && !sSecret.active)
            full_audio_start_office_ambience();
    }
}

static void render_game(Game *game)
{
    fnaf3_complete_render_game(game);
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    WHBProcInit();

    /* The custom studio intro owns a short-lived SDL TV/GamePad renderer and
     * closes it before the normal FNaF 3 renderer is created. Missing intro
     * content is non-fatal and simply falls through to the title screen. */
    (void) intro_play();

    if (!graphics_init()) {
        WHBProcShutdown();
        return 1;
    }

    input_init();
    frame_clock_reset();
    memset(&sFinishing, 0, sizeof(sFinishing));
    memset(&sSecret, 0, sizeof(sSecret));
    memset(&sSecretTriggers, 0, sizeof(sSecretTriggers));
    memset(&sCompleteCheats, 0, sizeof(sCompleteCheats));
    sTitleMusicPlaying = false;

    Game game;
    game_init(&game);

    /* The SD card is mounted by game_init before full-pack audio is loaded. */
    (void) audio_init();

    render_game(&game);
    while (WHBProcIsRunning()) {
        InputState input;
        input_update(&input);
        update_game(&game, &input);
        render_game(&game);
        frame_clock_wait_next();
    }

    audio_stop_all();
    audio_shutdown();
    progress_save_shutdown();
    graphics_shutdown();
    WHBProcShutdown();
    return 0;
}
