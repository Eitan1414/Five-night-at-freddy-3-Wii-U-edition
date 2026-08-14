/* MFA-derived Follow Me room grid, collisions and movement.
 * Source: fivenights3-94.mfa, frame `cutscenes` (1024x768).
 *
 * The original game does NOT use continuous Generic movement here. Events
 * 5..8 move the 62x62 hit box by exactly 30 source pixels every 250 ms while
 * W/S/D/A is held, after testing four invisible movement-marker Actives.
 * Collision is evaluated in the original PC coordinate space; only rendering
 * is transformed to the centered 640x480 Wii U content area.
 */

static int sFollowMfaVertical = 2;
static int sFollowMfaHorizontal = 3;
static int sFollowMfaEntryDirection = 2;
static int sFollowMfaPlayerSourceX = 500;
static int sFollowMfaPlayerSourceY = 272;

/* Shadow Freddy is created by events 95..104. */
static int sFollowMfaShadowKind;
static int sFollowMfaShadowSourceX;
static int sFollowMfaShadowSourceY;
static uint32_t sFollowMfaShadowMoveClock;
static uint32_t sFollowMfaFollowTextTicks;

enum {
    FOLLOW_MFA_WALL_UP    = 1u << 0,
    FOLLOW_MFA_WALL_DOWN  = 1u << 1,
    FOLLOW_MFA_WALL_LEFT  = 1u << 2,
    FOLLOW_MFA_WALL_RIGHT = 1u << 3,
};

enum {
    FOLLOW_MFA_SHADOW_NONE = 0,
    FOLLOW_MFA_SHADOW_RIGHT,
    FOLLOW_MFA_SHADOW_DOWN,
    FOLLOW_MFA_SHADOW_UP,
};

typedef enum FollowMeMfaMoveDir {
    FOLLOW_MFA_MOVE_UP = 0,
    FOLLOW_MFA_MOVE_DOWN,
    FOLLOW_MFA_MOVE_RIGHT,
    FOLLOW_MFA_MOVE_LEFT,
} FollowMeMfaMoveDir;

typedef struct FollowMeMfaRect {
    int left;
    int top;
    int right;
    int bottom;
} FollowMeMfaRect;

static int follow_me_pc_grid_scale(int value)
{
    if (value >= 0) return (value * 5 + 4) / 8;
    return -(((-value) * 5 + 4) / 8);
}

static int follow_me_pc_grid_x(int source_x)
{
    return 107 + follow_me_pc_grid_scale(source_x);
}

static int follow_me_pc_grid_y(int source_y)
{
    return follow_me_pc_grid_scale(source_y);
}

static void follow_me_pc_mfa_sync_player(void)
{
    /* sFollowMePc.player_x/y remain the hit-box position in Wii U coordinates;
     * character Actives are rendered at hit-box + (1,1), as event 9 specifies. */
    sFollowMePc.player_x = follow_me_pc_grid_x(sFollowMfaPlayerSourceX);
    sFollowMePc.player_y = follow_me_pc_grid_y(sFollowMfaPlayerSourceY);
}

static bool follow_me_pc_mfa_rect_overlap(FollowMeMfaRect a,
                                          FollowMeMfaRect b)
{
    return a.left < b.right && a.right > b.left &&
           a.top < b.bottom && a.bottom > b.top;
}

static FollowMeMfaRect follow_me_pc_mfa_hitbox_rect(void)
{
    /* Handle 299, PC image 30: 62x62, hotspot (31,31). */
    FollowMeMfaRect rect = {
        sFollowMfaPlayerSourceX - 31,
        sFollowMfaPlayerSourceY - 31,
        sFollowMfaPlayerSourceX + 31,
        sFollowMfaPlayerSourceY + 31,
    };
    return rect;
}

static FollowMeMfaRect follow_me_pc_mfa_marker_rect(FollowMeMfaMoveDir dir)
{
    /* Event 9 positions the four markers relative to the hit box:
     * up (+1,-61), down (-1,+63), right (+79,-3), left (-83,+3).
     * Images: up/down=323 (94x62 hot47,31), left/right=325
     * (58x102 hot29,51). */
    FollowMeMfaRect rect;
    switch (dir) {
        case FOLLOW_MFA_MOVE_UP:
            rect.left = sFollowMfaPlayerSourceX - 46;
            rect.top = sFollowMfaPlayerSourceY - 92;
            rect.right = rect.left + 94;
            rect.bottom = rect.top + 62;
            break;
        case FOLLOW_MFA_MOVE_DOWN:
            rect.left = sFollowMfaPlayerSourceX - 48;
            rect.top = sFollowMfaPlayerSourceY + 32;
            rect.right = rect.left + 94;
            rect.bottom = rect.top + 62;
            break;
        case FOLLOW_MFA_MOVE_RIGHT:
            rect.left = sFollowMfaPlayerSourceX + 50;
            rect.top = sFollowMfaPlayerSourceY - 54;
            rect.right = rect.left + 58;
            rect.bottom = rect.top + 102;
            break;
        case FOLLOW_MFA_MOVE_LEFT:
        default:
            rect.left = sFollowMfaPlayerSourceX - 112;
            rect.top = sFollowMfaPlayerSourceY - 48;
            rect.right = rect.left + 58;
            rect.bottom = rect.top + 102;
            break;
    }
    return rect;
}

static unsigned follow_me_pc_mfa_walls(int vertical, int horizontal)
{
    /* Events 10..25: Make Visible actions for edge-barrier Actives. */
    if (vertical == 1 && horizontal == 5)
        return FOLLOW_MFA_WALL_UP | FOLLOW_MFA_WALL_LEFT |
               FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 2 && horizontal == 1)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_UP |
               FOLLOW_MFA_WALL_LEFT;
    if (vertical == 2 && horizontal >= 2 && horizontal <= 4)
        return FOLLOW_MFA_WALL_UP;
    if (vertical == 2 && horizontal == 5)
        return FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 3 && horizontal == 2)
        return FOLLOW_MFA_WALL_LEFT;
    if (vertical == 3 && horizontal == 3)
        return FOLLOW_MFA_WALL_DOWN;
    if (vertical == 3 && horizontal == 4)
        return FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 3 && horizontal == 5)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_RIGHT |
               FOLLOW_MFA_WALL_LEFT;
    if (vertical == 4 && (horizontal == 2 || horizontal == 4))
        return FOLLOW_MFA_WALL_LEFT | FOLLOW_MFA_WALL_RIGHT;
    if (vertical == 5 && horizontal == 2)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_LEFT;
    if (vertical == 5 && horizontal == 3)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_UP;
    if (vertical == 5 && horizontal == 4)
        return FOLLOW_MFA_WALL_DOWN | FOLLOW_MFA_WALL_RIGHT;
    return 0u;
}

static bool follow_me_pc_mfa_marker_hits_barrier(FollowMeMfaMoveDir dir,
                                                  FollowMeMfaRect marker)
{
    const unsigned walls = follow_me_pc_mfa_walls(
        sFollowMfaVertical, sFollowMfaHorizontal);
    FollowMeMfaRect barrier;
    unsigned required_wall;

    switch (dir) {
        case FOLLOW_MFA_MOVE_UP:
            required_wall = FOLLOW_MFA_WALL_UP;
            barrier = (FollowMeMfaRect){308, -8, 716, 80};
            break;
        case FOLLOW_MFA_MOVE_DOWN:
            required_wall = FOLLOW_MFA_WALL_DOWN;
            barrier = (FollowMeMfaRect){308, 684, 716, 772};
            break;
        case FOLLOW_MFA_MOVE_RIGHT:
            required_wall = FOLLOW_MFA_WALL_RIGHT;
            barrier = (FollowMeMfaRect){927, 213, 1023, 555};
            break;
        case FOLLOW_MFA_MOVE_LEFT:
        default:
            required_wall = FOLLOW_MFA_WALL_LEFT;
            barrier = (FollowMeMfaRect){4, 213, 100, 555};
            break;
    }

    /* Events 44..47 require both overlap and CND_EXTSHOWN on the barrier. */
    return (walls & required_wall) != 0u &&
           follow_me_pc_mfa_rect_overlap(marker, barrier);
}

static bool follow_me_pc_mfa_marker_hits_backdrop(FollowMeMfaRect marker)
{
    /* Backdrop2 image31: solid 310x88 obstacle, four instances. */
    static const FollowMeMfaRect wide[4] = {
        {-2, -8, 308, 80},
        {-2, 680, 308, 768},
        {714, -8, 1024, 80},
        {716, 682, 1026, 770},
    };
    /* Backdrop3 image42: solid 102x134 obstacle, four instances. */
    static const FollowMeMfaRect tall[4] = {
        {-2, 80, 100, 214},
        {-2, 550, 100, 684},
        {926, 79, 1028, 213},
        {926, 550, 1028, 684},
    };

    for (int index = 0; index < 4; ++index) {
        if (follow_me_pc_mfa_rect_overlap(marker, wide[index]) ||
            follow_me_pc_mfa_rect_overlap(marker, tall[index]))
            return true;
    }
    return false;
}

static bool follow_me_pc_mfa_mask_bit(const FollowMeMfaCollisionMask *mask,
                                      int x, int y)
{
    if (mask == NULL || x < 0 || y < 0 ||
        x >= (int)mask->width || y >= (int)mask->height)
        return false;
    const uint32_t bit = (uint32_t)y * (uint32_t)mask->width + (uint32_t)x;
    return (mask->bits[bit >> 3] & (uint8_t)(1u << (bit & 7u))) != 0u;
}

static bool follow_me_pc_mfa_rect_hits_mask(
    FollowMeMfaRect marker,
    const FollowMeMfaCollisionMask *mask,
    int object_x,
    int object_y)
{
    if (mask == NULL) return false;

    const int object_left = object_x - mask->hotspot_x;
    const int object_top = object_y - mask->hotspot_y;
    FollowMeMfaRect bounds = {
        object_left,
        object_top,
        object_left + (int)mask->width,
        object_top + (int)mask->height,
    };
    if (!follow_me_pc_mfa_rect_overlap(marker, bounds)) return false;

    int left = marker.left > bounds.left ? marker.left : bounds.left;
    int top = marker.top > bounds.top ? marker.top : bounds.top;
    int right = marker.right < bounds.right ? marker.right : bounds.right;
    int bottom = marker.bottom < bounds.bottom ? marker.bottom : bounds.bottom;

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            if (follow_me_pc_mfa_mask_bit(mask,
                                          x - object_left,
                                          y - object_top))
                return true;
        }
    }
    return false;
}

static const FollowMeMfaCollisionMask *follow_me_pc_mfa_party_collision(void)
{
    if (sFollowMfaVertical == 2 && sFollowMfaHorizontal == 2)
        return &gFollowMfaCollisionPartyTable0;
    if (sFollowMfaVertical == 2 && sFollowMfaHorizontal == 4)
        return &gFollowMfaCollisionPartyTable12;
    if (sFollowMfaVertical == 3 && sFollowMfaHorizontal == 2)
        return &gFollowMfaCollisionPartyTable13;
    if (sFollowMfaVertical == 3 && sFollowMfaHorizontal == 3)
        return &gFollowMfaCollisionPartyTable14;
    if (sFollowMfaVertical == 3 && sFollowMfaHorizontal == 4)
        return &gFollowMfaCollisionPartyTable15;
    return NULL;
}

static bool follow_me_pc_mfa_marker_hits_dynamic(FollowMeMfaRect marker)
{
    static const int table_positions[4][2] = {
        {256, 185}, {763, 185}, {256, 536}, {763, 536}
    };
    const FollowMeMfaCollisionMask *table =
        follow_me_pc_mfa_party_collision();
    if (table != NULL) {
        for (int index = 0; index < 4; ++index) {
            if (follow_me_pc_mfa_rect_hits_mask(marker, table,
                                                table_positions[index][0],
                                                table_positions[index][1]))
                return true;
        }
    }

    if (sFollowMfaVertical == 5 && sFollowMfaHorizontal == 3 &&
        follow_me_pc_mfa_rect_hits_mask(marker,
                                        &gFollowMfaCollisionTableFan,
                                        500, 184))
        return true;

    if (sFollowMfaVertical == 2 && sFollowMfaHorizontal == 1 &&
        follow_me_pc_mfa_rect_hits_mask(marker,
                                        &gFollowMfaCollisionGift,
                                        398, 342))
        return true;

    return false;
}

static bool follow_me_pc_mfa_marker_clear(FollowMeMfaMoveDir dir)
{
    const FollowMeMfaRect marker = follow_me_pc_mfa_marker_rect(dir);
    if (follow_me_pc_mfa_marker_hits_barrier(dir, marker)) return false;
    if (follow_me_pc_mfa_marker_hits_backdrop(marker)) return false;
    if (follow_me_pc_mfa_marker_hits_dynamic(marker)) return false;

    /* Events 7/8 additionally require the hallway Active to be hidden. It is
     * shown only in source cells (4,2) and (4,4), making those corridors
     * vertical-only. */
    if ((dir == FOLLOW_MFA_MOVE_RIGHT || dir == FOLLOW_MFA_MOVE_LEFT) &&
        sFollowMfaVertical == 4 &&
        (sFollowMfaHorizontal == 2 || sFollowMfaHorizontal == 4))
        return false;

    return true;
}

static bool follow_me_pc_mfa_input_enabled(void)
{
    /* Events 5..8 require Number of `man 1` == 0, Active297 A==0 and the
     * trip counter to remain before the fourth scare. */
    if (sFollowMePc.phase == FOLLOW_PHASE_HUNT ||
        sFollowMePc.phase == FOLLOW_PHASE_DISMANTLED ||
        sFollowMePc.phase == FOLLOW_PHASE_SUIT ||
        sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK)
        return false;
    if (sFollowMePc.phase == FOLLOW_PHASE_GHOST_CHASE &&
        sFollowMePc.chase_contacts >= 4)
        return false;
    return true;
}

static bool follow_me_pc_mfa_guide_for_cell(int *kind, int *x, int *y)
{
    if (sFollowMePc.chapter >= 5) return false;

    if (sFollowMfaVertical == 2 && sFollowMfaHorizontal == 4) {
        *kind = FOLLOW_MFA_SHADOW_RIGHT; *x = 816; *y = 376; return true;
    }
    if (((sFollowMfaVertical == 3 && sFollowMfaHorizontal == 3) ||
         (sFollowMfaVertical == 3 && sFollowMfaHorizontal == 2)) &&
        sFollowMfaEntryDirection != 4) {
        *kind = FOLLOW_MFA_SHADOW_RIGHT; *x = 816; *y = 376; return true;
    }
    if (sFollowMfaVertical == 2 && sFollowMfaHorizontal == 5) {
        *kind = FOLLOW_MFA_SHADOW_UP; *x = 520; *y = 202; return true;
    }
    if (((sFollowMfaVertical == 3 && sFollowMfaHorizontal == 4) ||
         (sFollowMfaVertical == 4 && sFollowMfaHorizontal == 4) ||
         (sFollowMfaVertical == 5 && sFollowMfaHorizontal == 4)) &&
        sFollowMfaEntryDirection != 3) {
        *kind = FOLLOW_MFA_SHADOW_UP; *x = 520; *y = 202; return true;
    }
    if (((sFollowMfaVertical == 5 && sFollowMfaHorizontal == 3) ||
         (sFollowMfaVertical == 5 && sFollowMfaHorizontal == 2)) &&
        sFollowMfaEntryDirection != 4) {
        *kind = FOLLOW_MFA_SHADOW_RIGHT; *x = 816; *y = 376; return true;
    }
    if (sFollowMfaVertical == 4 && sFollowMfaHorizontal == 2 &&
        sFollowMfaEntryDirection != 1) {
        *kind = FOLLOW_MFA_SHADOW_DOWN; *x = 514; *y = 564; return true;
    }
    return false;
}

static void follow_me_pc_mfa_spawn_guide(void)
{
    int kind = FOLLOW_MFA_SHADOW_NONE;
    int x = 0;
    int y = 0;

    sFollowMfaShadowKind = FOLLOW_MFA_SHADOW_NONE;
    if (!follow_me_pc_mfa_guide_for_cell(&kind, &x, &y)) return;

    sFollowMfaShadowKind = kind;
    sFollowMfaShadowSourceX = x;
    sFollowMfaShadowSourceY = y;
    sFollowMfaShadowMoveClock = 0u;
    sFollowMfaFollowTextTicks = 200u;
}

static void follow_me_pc_mfa_step_guide(void)
{
    if (sFollowMfaFollowTextTicks > 0u)
        --sFollowMfaFollowTextTicks;

    if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_NONE) return;

    /* Event 88: 30 source pixels every 290 ms. */
    sFollowMfaShadowMoveClock += 1000u;
    while (sFollowMfaShadowMoveClock >= 17400u) {
        sFollowMfaShadowMoveClock -= 17400u;
        if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_RIGHT)
            sFollowMfaShadowSourceX += 30;
        else if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_DOWN)
            sFollowMfaShadowSourceY += 30;
        else if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_UP)
            sFollowMfaShadowSourceY -= 30;
    }

    if (sFollowMfaShadowSourceX > 1055 ||
        sFollowMfaShadowSourceY < -80 ||
        sFollowMfaShadowSourceY > 850)
        sFollowMfaShadowKind = FOLLOW_MFA_SHADOW_NONE;
}

static void follow_me_pc_reset(int chapter)
{
    follow_me_pc_reset_legacy(chapter);

    sFollowMfaVertical = 2;
    sFollowMfaHorizontal = 3;
    sFollowMfaEntryDirection = 2;
    sFollowMfaShadowKind = FOLLOW_MFA_SHADOW_NONE;
    sFollowMfaShadowMoveClock = 0u;
    sFollowMfaFollowTextTicks = 0u;

    sFollowMfaPlayerSourceX = 500;
    sFollowMfaPlayerSourceY = 272;
    if (chapter == 2) {
        sFollowMfaPlayerSourceX = 342;
        sFollowMfaPlayerSourceY = 212;
    } else if (chapter == 3) {
        sFollowMfaPlayerSourceX = 658;
        sFollowMfaPlayerSourceY = 212;
    } else if (chapter == 4) {
        sFollowMfaVertical = 3;
        sFollowMfaHorizontal = 2;
        sFollowMfaPlayerSourceX = 268;
        sFollowMfaPlayerSourceY = 420;
    } else if (chapter >= 5) {
        sFollowMfaPlayerSourceX = 496;
        sFollowMfaPlayerSourceY = 494;
    }

    follow_me_pc_mfa_sync_player();
    sFollowMePc.room = FOLLOW_ROOM_SHOW_STAGE; /* legacy storage only */
    follow_me_pc_mfa_spawn_guide();
}

static void follow_me_pc_mfa_transition_if_needed(void)
{
    FollowMeMfaRect hitbox = follow_me_pc_mfa_hitbox_rect();
    const FollowMeMfaRect leave_right = {988, 212, 1058, 542};
    const FollowMeMfaRect leave_left = {-36, 204, 34, 534};
    const FollowMeMfaRect leave_top = {266, -40, 732, 38};
    const FollowMeMfaRect leave_bottom = {292, 732, 758, 810};
    bool changed = false;

    /* Events 26..29, in source order. */
    if (follow_me_pc_mfa_rect_overlap(hitbox, leave_right)) {
        ++sFollowMfaHorizontal;
        sFollowMfaEntryDirection = 2;
        sFollowMfaPlayerSourceX = 142;
        sFollowMfaPlayerSourceY = 377;
        changed = true;
    } else if (follow_me_pc_mfa_rect_overlap(hitbox, leave_left)) {
        --sFollowMfaHorizontal;
        sFollowMfaEntryDirection = 4;
        sFollowMfaPlayerSourceX = 879;
        sFollowMfaPlayerSourceY = 369;
        changed = true;
    } else if (follow_me_pc_mfa_rect_overlap(hitbox, leave_top)) {
        --sFollowMfaVertical;
        sFollowMfaEntryDirection = 1;
        sFollowMfaPlayerSourceX = 499;
        sFollowMfaPlayerSourceY = 664;
        changed = true;
    } else if (follow_me_pc_mfa_rect_overlap(hitbox, leave_bottom)) {
        /* Event 29 additionally requires Number of chapter-5 `man` == 0. */
        if (sFollowMePc.phase != FOLLOW_PHASE_GHOST_CHASE) {
            ++sFollowMfaVertical;
            sFollowMfaEntryDirection = 3;
            sFollowMfaPlayerSourceX = 525;
            sFollowMfaPlayerSourceY = 122;
            changed = true;
        }
    }

    if (changed) {
        follow_me_pc_mfa_sync_player();
        follow_me_pc_mfa_spawn_guide();
    }
}

static void follow_me_pc_mfa_err_bounce_if_needed(void)
{
    /* Event 76: in cell (2,5), chapters before 5 can move through the hidden
     * up barrier (it is not SHOWN, so marker A stays zero), but when the hit box
     * itself overlaps that barrier it is pushed down +60 and ERR is armed to
     * 200. This is what prevents entry to the safe room in Follow Me 1..4. */
    if (sFollowMePc.chapter >= 5 ||
        sFollowMfaVertical != 2 || sFollowMfaHorizontal != 5)
        return;

    const FollowMeMfaRect up_barrier = {308, -8, 716, 80};
    if (!follow_me_pc_mfa_rect_overlap(follow_me_pc_mfa_hitbox_rect(),
                                       up_barrier))
        return;

    sFollowMfaPlayerSourceY += 60;
    follow_me_pc_mfa_sync_player();
    sFollowMePc.phase = FOLLOW_PHASE_ERR;
    sFollowMePc.phase_frames = 0u;
}

static void follow_me_pc_move_player(const InputState *input)
{
    /* Shadow/FOLLOW ME events continue independently while ERR is active. */
    follow_me_pc_mfa_step_guide();

    const bool input_enabled = follow_me_pc_mfa_input_enabled();
    const bool held_up = input_enabled && input_is_held(input, GAME_BUTTON_UP);
    const bool held_down = input_enabled && input_is_held(input, GAME_BUTTON_DOWN);
    const bool held_right = input_enabled && input_is_held(input, GAME_BUTTON_RIGHT);
    const bool held_left = input_enabled && input_is_held(input, GAME_BUTTON_LEFT);
    const bool any_held = held_up || held_down || held_right || held_left;

    sFollowMePc.moving = any_held;
    if (any_held) ++sFollowMePc.move_frames;

    /* CND_EVERY(250 ms): at 60 Hz these events fire every 15 frames. Marker
     * positions are calculated by event9 AFTER W/S/D/A, so all four conditions
     * in this tick see the same pre-movement marker positions. */
    if ((sFollowMePc.frames % 15u) == 0u && input_enabled) {
        const bool clear_up = follow_me_pc_mfa_marker_clear(FOLLOW_MFA_MOVE_UP);
        const bool clear_down = follow_me_pc_mfa_marker_clear(FOLLOW_MFA_MOVE_DOWN);
        const bool clear_right = follow_me_pc_mfa_marker_clear(FOLLOW_MFA_MOVE_RIGHT);
        const bool clear_left = follow_me_pc_mfa_marker_clear(FOLLOW_MFA_MOVE_LEFT);

        /* Exact event order 5,6,7,8: W, S, D, A. Opposites may therefore
         * cancel and diagonals may move on both axes during the same 250ms tick. */
        if (held_up && clear_up) {
            sFollowMfaPlayerSourceY -= 30;
            sFollowMePc.direction = FOLLOW_DIR_UP;
        }
        if (held_down && clear_down) {
            sFollowMfaPlayerSourceY += 30;
            sFollowMePc.direction = FOLLOW_DIR_DOWN;
        }
        if (held_right && clear_right) {
            sFollowMfaPlayerSourceX += 30;
            sFollowMePc.direction = FOLLOW_DIR_RIGHT;
        }
        if (held_left && clear_left) {
            sFollowMfaPlayerSourceX -= 30;
            sFollowMePc.direction = FOLLOW_DIR_LEFT;
        }
        follow_me_pc_mfa_sync_player();
    }

    /* Events26..29 precede event76. For chapters1..4 the hidden up barrier is
     * encountered well before the top leave rectangle, so the resulting bounce
     * is equivalent while preserving the original safe-room gate. */
    follow_me_pc_mfa_transition_if_needed();
    follow_me_pc_mfa_err_bounce_if_needed();
}

static void follow_me_pc_update_room_transitions(void)
{
    /* The four original leave-object overlap events run in move_player(). */
}
