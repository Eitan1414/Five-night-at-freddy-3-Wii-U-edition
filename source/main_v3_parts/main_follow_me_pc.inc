#include "assets/follow_me_pc_env_assets.h"
#include "assets/follow_me_mfa_visuals.h"

#define follow_me_pc_reset follow_me_pc_reset_legacy
#define follow_me_pc_move_player follow_me_pc_move_player_legacy
#define follow_me_pc_update_room_transitions follow_me_pc_update_room_transitions_legacy
#include "main_follow_me_pc_part01.inc"
#undef follow_me_pc_update_room_transitions
#undef follow_me_pc_move_player
#undef follow_me_pc_reset

#include "main_follow_me_pc_mfa_grid.inc"

/* Keep the recovered cropped renderer as a fallback for objects not yet fully
 * audited, while replacing the four playable animatronics below it with the
 * original 200x200 MFA canvases/hotspots. */
#define follow_me_pc_player_sprite follow_me_pc_player_sprite_legacy
#define follow_me_pc_draw_sprite follow_me_pc_draw_sprite_legacy
#include "main_follow_me_pc_part02.inc"
#undef follow_me_pc_draw_sprite
#undef follow_me_pc_player_sprite

#include "main_follow_me_pc_mfa_players.inc"
#include "main_follow_me_pc_part03.inc"
#include "main_follow_me_pc_part04.inc"
#include "main_follow_me_pc_part05.inc"
