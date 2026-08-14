static void fnaf3_follow_me_update_game(Game *game, const InputState *input)
{
    if (game->screen == SCREEN_PLACEHOLDER &&
        sFinishing.mode == FINISHING_FOLLOW_ME) {
        follow_me_pc_update(game, input);
        return;
    }

    const FinishingMode before_mode = sFinishing.mode;
    fnaf3_finishing_update_game(game, input);
    if (before_mode != FINISHING_FOLLOW_ME &&
        sFinishing.mode == FINISHING_FOLLOW_ME) {
        follow_me_pc_reset(sFinishing.minigame_chapter);
        follow_me_pc_sync();
        game->dirty = true;
    }
}

static void fnaf3_follow_me_render_game(Game *game)
{
    if (!game->dirty) return;
    if (game->screen == SCREEN_PLACEHOLDER &&
        sFinishing.mode == FINISHING_FOLLOW_ME) {
        game->dirty = false;
        if (!sFollowMePc.initialized)
            follow_me_pc_reset(sFinishing.minigame_chapter);
        follow_me_pc_draw(game);
        return;
    }
    fnaf3_finishing_render_game(game);
}
