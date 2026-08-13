#define wiiu_control_update_game_v6 wiiu_control_update_game_v6_base
#include "main_wiiu_controls_v6_base.inc"
#undef wiiu_control_update_game_v6

static void wiiu_control_update_game_v6(Game *game, const InputState *input)
{
    if (wiiu_follow_me_active(game)) {
        follow_me_pc_update(game, input);
        return;
    }
    wiiu_control_update_game_v6_base(game, input);
}
