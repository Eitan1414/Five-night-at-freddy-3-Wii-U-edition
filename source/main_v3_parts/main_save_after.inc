#undef reset_night
#undef game_init
#undef begin_victory

static void reset_night(Game *game, int night)
{
    if (game->unlocked_night < g_highest_unlocked_night)
        game->unlocked_night = g_highest_unlocked_night;
    reset_night_without_progress_guard(game, night);
}

static void game_init(Game *game)
{
    game_init_without_autosave(game);
    save_system_start(game);
}

static void begin_victory(Game *game)
{
    autosave_completed_night(game);
    begin_victory_without_autosave(game);
}

#define handle_title_input handle_title_input_without_save_menu
