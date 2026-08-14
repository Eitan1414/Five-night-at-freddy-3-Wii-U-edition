static uint32_t sFollowMfaFinaleRng = 0x5F3759DFu;
static int sFollowMfaFinaleLastScareState = -1;
static bool sFollowMfaFinaleSpiritsVisible = false;
static int sFollowMfaSpringState = 0;
static uint32_t sFollowMfaSpringTimer = 0u;
static bool sFollowMfaSpringLaughPlayed = false;

static bool follow_me_pc_finale_random2(void)
{
    /* The MFA asks Fusion for Random(2) independently in the up/down jitter
     * events. Preserve that probability/cadence; the exact Fusion RNG stream is
     * not part of the project data. */
    sFollowMfaFinaleRng = sFollowMfaFinaleRng * 1664525u + 1013904223u;
    return (sFollowMfaFinaleRng >> 31) != 0u;
}

static void follow_me_pc_update_hunt(void)
{
    /* MFA events 110..113: Purple Guy moves 30 PC pixels every 100 ms,
     * independently on X and Y, toward the hit box. */
    if ((sFollowMePc.phase_frames % 6u) == 0u) {
        const int step = follow_me_pc_grid_scale(30);
        if (sFollowMePc.purple_x < sFollowMePc.player_x)
            sFollowMePc.purple_x += step;
        else if (sFollowMePc.purple_x > sFollowMePc.player_x)
            sFollowMePc.purple_x -= step;
        if (sFollowMePc.purple_y < sFollowMePc.player_y)
            sFollowMePc.purple_y += step;
        else if (sFollowMePc.purple_y > sFollowMePc.player_y)
            sFollowMePc.purple_y -= step;
    }

    if (follow_me_pc_near(sFollowMePc.player_x, sFollowMePc.player_y,
                          sFollowMePc.purple_x, sFollowMePc.purple_y, 27)) {
        /* Events 114..117 switch to the take-apart/parts objects. No `crush`
         * sample is fired here in the MFA; that sample belongs to Night 5. */
        sFollowMePc.phase = FOLLOW_PHASE_DISMANTLED;
        sFollowMePc.phase_frames = 1u;
    }
}

static void follow_me_pc_begin_finale(void)
{
    /* Event 157, cell (vertical=1,horizontal=5):
     * man = leave-top (499,-1) + (163,453) => (662,452)
     * Active 4 = leave-top + (329,463) => (828,462). */
    sFollowMePc.phase = FOLLOW_PHASE_GHOST_CHASE;
    sFollowMePc.phase_frames = 0u;
    sFollowMePc.purple_x = follow_me_pc_grid_x(662);
    sFollowMePc.purple_y = follow_me_pc_grid_y(452);
    sFollowMePc.purple_target = 0; /* `man` alterable value A */
    sFollowMePc.chase_contacts = 0; /* `count trips` */
    sFollowMePc.contact_cooldown = 0u;
    sFollowMfaFinaleLastScareState = -1;
    sFollowMfaFinaleSpiritsVisible = true;
    sFollowMfaSpringState = 0;
    sFollowMfaSpringTimer = 0u;
    sFollowMfaSpringLaughPlayed = false;
    sFollowMfaFinaleRng = 0x9E3779B9u ^ sFollowMePc.frames;
}

static void follow_me_pc_begin_run_to_suit(void)
{
    /* Event 163 destroys `man` and creates `man run` from leave-top with
     * (-335,+359): exact source position (164,358). */
    sFollowMePc.phase = FOLLOW_PHASE_SUIT;
    sFollowMePc.phase_frames = 0u;
    sFollowMePc.purple_x = follow_me_pc_grid_x(164);
    sFollowMePc.purple_y = follow_me_pc_grid_y(358);
    sFollowMePc.contact_cooldown = 0u;
}

static void follow_me_pc_finale_scare_if_needed(void)
{
    const int state = sFollowMePc.purple_target;
    if (state != 1 && state != 3) {
        sFollowMfaFinaleLastScareState = -1;
        return;
    }
    if (sFollowMfaFinaleLastScareState == state) return;

    sFollowMfaFinaleLastScareState = state;
    pc_minigame_sfx_play(PC_MINIGAME_SFX_SCARE, 1.0f, false);
    ++sFollowMePc.chase_contacts; /* events 161/162: count trips += 1 */
    if (sFollowMePc.chase_contacts >= 4)
        follow_me_pc_begin_run_to_suit();
}

static void follow_me_pc_update_ghost_chase(void)
{
    /* Events 142..162. Purple Guy does not follow a fixed waypoint list: the
     * child chases him, he keeps a 200-PC-pixel gap and alternates between the
     * 200 and 900 X limits. Every state-1/state-3 entry is one `scare`; after
     * four scares event 163 starts the run to the suit. */
    ++sFollowMePc.phase_frames;

    const bool tick_100ms = (sFollowMePc.phase_frames % 6u) == 0u;
    const int step = follow_me_pc_grid_scale(50);
    const int gap = follow_me_pc_grid_scale(200);
    const int left_limit = follow_me_pc_grid_x(200);
    const int right_limit = follow_me_pc_grid_x(900);
    int state = sFollowMePc.purple_target;
    bool moved_with_sample = false;

    /* Event 142: state 0, child closes to within 200 from the left -> +50. */
    if (tick_100ms && state == 0 &&
        sFollowMePc.purple_x < sFollowMePc.player_x + gap &&
        sFollowMePc.purple_x < right_limit) {
        sFollowMePc.purple_x += step;
        moved_with_sample = true;
    }

    /* Event 143: state 2 mirrors that chase toward the left. */
    if (tick_100ms && state == 2 &&
        sFollowMePc.purple_x > sFollowMePc.player_x - gap &&
        sFollowMePc.purple_x > left_limit) {
        sFollowMePc.purple_x -= step;
        moved_with_sample = true;
    }

    /* Event 144. */
    if (state == 0 && sFollowMePc.purple_x > right_limit) {
        state = 1;
        sFollowMePc.purple_target = state;
    }

    /* Event 145. */
    if (tick_100ms && state == 1 &&
        sFollowMePc.purple_x > sFollowMePc.player_x - gap &&
        sFollowMePc.purple_x > left_limit) {
        sFollowMePc.purple_x -= step;
    }

    /* Events 146/151: state 1 settles into state 2 once the 200 gap is made. */
    if (state == 1 && sFollowMePc.purple_x <= sFollowMePc.player_x - gap) {
        state = 2;
        sFollowMePc.purple_target = state;
    }

    /* Event 147. */
    if (tick_100ms && state == 3 &&
        sFollowMePc.purple_x < sFollowMePc.player_x + gap &&
        sFollowMePc.purple_x < right_limit) {
        sFollowMePc.purple_x += step;
    }

    /* Events 148/150. */
    if (state == 1 && sFollowMePc.purple_x <= left_limit) {
        state = 2;
        sFollowMePc.purple_target = state;
    }
    if (state == 2 && sFollowMePc.purple_x <= left_limit) {
        state = 3;
        sFollowMePc.purple_target = state;
    }

    /* Event 152. */
    if (state == 3 && sFollowMePc.purple_x >= sFollowMePc.player_x + gap) {
        state = 0;
        sFollowMePc.purple_target = state;
    }

    /* Events 153..156: two independent Random(2) checks every 100 ms. */
    state = sFollowMePc.purple_target;
    if (tick_100ms && (state == 1 || state == 3)) {
        if (sFollowMePc.purple_y > follow_me_pc_grid_y(200) &&
            follow_me_pc_finale_random2())
            sFollowMePc.purple_y -= step;
        if (sFollowMePc.purple_y < follow_me_pc_grid_y(600) &&
            follow_me_pc_finale_random2())
            sFollowMePc.purple_y += step;
    }

    /* Events 142/143 include `run`; events 159/160 replay it each 100 ms in
     * states 1 and 3. */
    if (tick_100ms &&
        (moved_with_sample || state == 1 || state == 3))
        pc_minigame_sfx_play(PC_MINIGAME_SFX_RUN, 1.0f, false);

    /* Events 161/162 are Only-one-action conditions and run after the state
     * transitions above. */
    follow_me_pc_finale_scare_if_needed();
}

static void follow_me_pc_begin_springlock(void)
{
    /* Event 165: `man run` overlaps Active 4 -> state=1, insuit, destroy run. */
    sFollowMePc.phase = FOLLOW_PHASE_SPRINGLOCK;
    sFollowMePc.phase_frames = 0u;
    sFollowMfaSpringState = 1;
    sFollowMfaSpringTimer = 0u;
    sFollowMfaSpringLaughPlayed = false;
    sFollowMePc.crush_played = false;
    pc_minigame_sfx_play(PC_MINIGAME_SFX_INSUIT, 1.0f, false);
}

static void follow_me_pc_update_suit(void)
{
    /* Event 164: every 100 ms play run and move man-run +30 PC pixels. */
    ++sFollowMePc.phase_frames;
    if ((sFollowMePc.phase_frames % 6u) == 0u) {
        pc_minigame_sfx_play(PC_MINIGAME_SFX_RUN, 1.0f, false);
        sFollowMePc.purple_x += follow_me_pc_grid_scale(30);
    }

    /* Active 4 idle image 471 has hotspot (73,150), and man-run's widest
     * right extension is 97 px. Its collision therefore begins around source
     * x=755-97=658 on the exact horizontal run line. */
    if (sFollowMePc.purple_x >= follow_me_pc_grid_x(658))
        follow_me_pc_begin_springlock();
}

static bool follow_me_pc_spring_second_frame(int speed)
{
    if (sFollowMfaSpringTimer == 0u || speed <= 0) return false;
    /* Fusion's animation counter adds the animation speed and advances while
     * counter > 100. This gives the exact two-frame phase for speeds 5/25/50. */
    const uint32_t advances =
        (sFollowMfaSpringTimer * (uint32_t)speed - 1u) / 100u;
    return (advances & 1u) != 0u;
}

static bool follow_me_pc_update_springlock(Game *game, const InputState *input)
{
    ++sFollowMePc.phase_frames;

    /* The child is not destroyed until event 181 (state 5), so preserve player
     * movement through states 1..4 just like the Clickteam event sheet. */
    if (sFollowMfaSpringState < 5)
        follow_me_pc_move_player(input);
    else
        sFollowMePc.moving = false;

    /* Event 166: while state > 0, alterable value B += 1 every event loop. */
    ++sFollowMfaSpringTimer;

    /* Event 179: animation 14, frame 1, only once -> laugh. Animation 14 is
     * speed 5 and uses PC images 486/489. */
    if (sFollowMfaSpringState == 3 &&
        follow_me_pc_spring_second_frame(5) &&
        !sFollowMfaSpringLaughPlayed) {
        sFollowMfaSpringLaughPlayed = true;
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LAUGH, 1.0f, false);
    }

    uint32_t threshold = 0u;
    switch (sFollowMfaSpringState) {
        case 1: threshold = 100u; break;
        case 2: threshold = 100u; break;
        case 3: threshold = 200u; break;
        case 4: threshold = 300u; break;
        case 5: threshold = 200u; break;
        case 6: threshold = 300u; break;
        default: break;
    }

    if (threshold == 0u || sFollowMfaSpringTimer <= threshold)
        return false;

    if (sFollowMfaSpringState == 6) {
        /* Event 178: state 6 and timer > 300 -> end frame. */
        memset(&sFollowMePc, 0, sizeof(sFollowMePc));
        finishing_complete_minigame(game);
        return true;
    }

    ++sFollowMfaSpringState;
    sFollowMfaSpringTimer = 0u;

    if (sFollowMfaSpringState == 4) {
        /* Event 180: stop samples, then play the original crush sample. */
        pc_minigame_sfx_stop_all();
        sFollowMePc.crush_played = true;
        audio_play(AUDIO_CUE_MINIGAME_CRUSH, 1.0f, false);
    } else if (sFollowMfaSpringState == 5) {
        /* Event 181: insuit, then destroy child and child2 spirits. */
        pc_minigame_sfx_play(PC_MINIGAME_SFX_INSUIT, 1.0f, false);
        sFollowMfaFinaleSpiritsVisible = false;
    }
    return false;
}

static void follow_me_pc_update(Game *game, const InputState *input)
{
    if (!sFollowMePc.initialized ||
        sFollowMePc.chapter != sFinishing.minigame_chapter) {
        follow_me_pc_reset(sFinishing.minigame_chapter);
        sFollowMfaFinaleSpiritsVisible = false;
        sFollowMfaSpringState = 0;
        sFollowMfaSpringTimer = 0u;
        sFollowMfaFinaleLastScareState = -1;
    }

    ++sFollowMePc.frames;
    ++game->screen_frames;

    if (sFinishing.minigame_replay && input_was_pressed(input, GAME_BUTTON_BACK)) {
        memset(&sFollowMePc, 0, sizeof(sFollowMePc));
        finishing_open_extras(game);
        sFinishing.extras_page = EXTRAS_PAGE_MINIGAMES;
        sFinishing.extras_index = sFinishing.minigame_chapter - 1;
        return;
    }

    if (sFollowMePc.phase == FOLLOW_PHASE_DISMANTLED) {
        ++sFollowMePc.phase_frames;
        /* Event 118 increments the take-apart counter once per tick; event 119
         * ends the frame as soon as that counter reaches 100. */
        if (sFollowMePc.phase_frames >= 100u) {
            memset(&sFollowMePc, 0, sizeof(sFollowMePc));
            finishing_complete_minigame(game);
            return;
        }
    } else if (sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK) {
        if (follow_me_pc_update_springlock(game, input)) return;
    } else {
        /* Event157 does not destroy/disable the player, and man-run is an
         * independent object, so Wii U input remains active through the chase
         * and run-to-suit phases. */
        follow_me_pc_move_player(input);

        /* Event 157 fires Only Once when Night 5 reaches source cell (1,5).
         * The old seven-room router used to start this confrontation; now the
         * MFA grid owns it directly. */
        if (sFollowMePc.chapter == 5 &&
            sFollowMePc.phase == FOLLOW_PHASE_GUIDE &&
            sFollowMfaVertical == 1 && sFollowMfaHorizontal == 5) {
            follow_me_pc_begin_finale();
        }

        if (sFollowMePc.phase == FOLLOW_PHASE_GUIDE) {
            follow_me_pc_update_shadow();
            follow_me_pc_update_room_transitions();
        } else if (sFollowMePc.phase == FOLLOW_PHASE_ERR) {
            ++sFollowMePc.phase_frames;

            if (sFollowMePc.phase_frames >= 200u &&
                follow_me_pc_near(sFollowMePc.player_x,
                                  sFollowMePc.player_y,
                                  follow_me_pc_grid_x(501),
                                  follow_me_pc_grid_y(387), 28)) {
                sFollowMePc.phase = FOLLOW_PHASE_HUNT;
                sFollowMePc.phase_frames = 0u;
                sFollowMePc.purple_x = follow_me_pc_grid_x(502);
                sFollowMePc.purple_y = follow_me_pc_grid_y(14);
                pc_minigame_sfx_play(PC_MINIGAME_SFX_CRAZY_GARBLE,
                                     1.0f, false);
            }
        } else if (sFollowMePc.phase == FOLLOW_PHASE_HUNT) {
            ++sFollowMePc.phase_frames;
            follow_me_pc_update_hunt();
        } else if (sFollowMePc.phase == FOLLOW_PHASE_GHOST_CHASE) {
            follow_me_pc_update_ghost_chase();
        } else if (sFollowMePc.phase == FOLLOW_PHASE_SUIT) {
            follow_me_pc_update_suit();
        }
    }

    follow_me_pc_sync();
    game->dirty = true;
}

static FollowMePcSprite follow_me_pc_player_sprite(void)
{
    int base = FOLLOW_ME_PC_FREDDY_0;
    if (sFollowMePc.chapter == 2) base = FOLLOW_ME_PC_BONNIE_0;
    else if (sFollowMePc.chapter == 3) base = FOLLOW_ME_PC_CHICA_0;
    else if (sFollowMePc.chapter == 4) base = FOLLOW_ME_PC_FOXY_0;
    else if (sFollowMePc.chapter == 5) base = FOLLOW_ME_PC_GHOST_0;

    int pair = 0;
    switch (sFollowMePc.direction) {
        case FOLLOW_DIR_LEFT: pair = 0; break;
        case FOLLOW_DIR_UP: pair = 2; break;
        case FOLLOW_DIR_RIGHT: pair = 4; break;
        case FOLLOW_DIR_DOWN: pair = 6; break;
        default: break;
    }
    const int frame = sFollowMePc.moving
        ? (int) ((sFollowMePc.move_frames / 7u) & 1u) : 0;
    return (FollowMePcSprite) (base + pair + frame);
}

static void follow_me_pc_draw_sprite(FollowMePcSprite sprite, int x, int y,
                                     int scale)
{
    const TextureRle *texture = follow_me_pc_sprite(sprite);
    if (texture == NULL || scale <= 0) return;

    const int origin_x = x - ((int) texture->width * scale) / 2;
    const int origin_y = y - (int) texture->height * scale;
    for (uint32_t source_y = 0u; source_y < texture->height; ++source_y) {
        const uint16_t row_start = texture->row_offsets[source_y];
        const uint16_t row_end = texture->row_offsets[source_y + 1u];
        uint32_t source_x = 0u;
        for (uint16_t offset = row_start;
             offset + 1u < row_end;
             offset += 2u) {
            const uint8_t run_length = texture->runs[offset];
            const uint8_t palette_index = texture->runs[offset + 1u];
            if (palette_index != texture->transparent_index && run_length > 0u) {
                graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                                   origin_x + (int) source_x * scale,
                                   origin_y + (int) source_y * scale,
                                   (int) run_length * scale,
                                   scale,
                                   texture->palette[palette_index]);
            }
            source_x += run_length;
        }
    }
}

static void follow_me_pc_draw_env(FollowMePcEnvSprite sprite, int x, int y,
                                  int scale)
{
    const TextureRle *texture = follow_me_pc_env_sprite(sprite);
    if (texture == NULL) return;
    texture_draw_rle(GRAPHICS_TARGET_BOTH, x, y,
                     (int) texture->width * scale,
                     (int) texture->height * scale, texture);
}

static void follow_me_pc_draw_checker_floor(int top)
{
    const int cell = 24;
    for (int y = top; y < 480; y += cell) {
        for (int x = 0; x < 854; x += cell) {
            const bool alternate = (((x / cell) + (y / cell)) & 1) != 0;
            graphics_draw_rect(GRAPHICS_TARGET_BOTH, x, y, cell, cell,
                alternate ? GRAPHICS_RGB(6, 6, 9) : GRAPHICS_RGB(35, 34, 43));
        }
    }
}
