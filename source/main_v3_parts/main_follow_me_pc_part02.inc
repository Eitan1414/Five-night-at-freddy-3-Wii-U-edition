static void follow_me_pc_update_hunt(void)
{
    /* MFA events 110..113: Purple Guy moves 30 PC pixels every 100 ms,
     * independently on X and Y, toward the hit box.  Six 60-Hz updates are
     * exactly 100 ms; scale the source step for the centered 640x480 Wii U
     * viewport instead of moving five Wii U pixels every frame. */
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
        sFollowMePc.phase = FOLLOW_PHASE_DISMANTLED;
        sFollowMePc.phase_frames = 1u;
        audio_play(AUDIO_CUE_MINIGAME_CRUSH, 0.75f, false);
    }
}

static void follow_me_pc_update_ghost_chase(void)
{
    static const int targets[6][2] = {
        {365, 245}, {535, 220}, {330, 320},
        {585, 330}, {625, 255}, {690, 220}
    };

    ++sFollowMePc.phase_frames;

    if (sFollowMePc.chase_contacts == 0) {
        const int dx = sFollowMePc.player_x - sFollowMePc.purple_x;
        const int dy = sFollowMePc.player_y - sFollowMePc.purple_y;
        const int distance = follow_me_pc_abs(dx) + follow_me_pc_abs(dy);
        if (distance < 135 || sFollowMePc.phase_frames >= 210u) {
            sFollowMePc.chase_contacts = 1;
            sFollowMePc.purple_target = 0;
            sFollowMePc.phase_frames = 0u;
            audio_play(AUDIO_CUE_SELECT, 0.35f, false);
        } else {
            return;
        }
    }

    if (sFollowMePc.purple_target >= 6) {
        sFollowMePc.phase = FOLLOW_PHASE_SUIT;
        sFollowMePc.phase_frames = 0u;
        sFollowMePc.contact_cooldown = 0u;
        return;
    }

    const int tx = targets[sFollowMePc.purple_target][0];
    const int ty = targets[sFollowMePc.purple_target][1];
    if (sFollowMePc.purple_x < tx) sFollowMePc.purple_x += 4;
    else if (sFollowMePc.purple_x > tx) sFollowMePc.purple_x -= 4;
    if (sFollowMePc.purple_y < ty) sFollowMePc.purple_y += 3;
    else if (sFollowMePc.purple_y > ty) sFollowMePc.purple_y -= 3;

    if (follow_me_pc_near(sFollowMePc.purple_x, sFollowMePc.purple_y,
                          tx, ty, 10)) {
        ++sFollowMePc.purple_target;
        audio_play(AUDIO_CUE_SELECT, 0.22f, false);
    }
}

static void follow_me_pc_update_suit(void)
{
    ++sFollowMePc.phase_frames;
    const int suit_x = 742;
    const int suit_y = 235;

    if (!follow_me_pc_near(sFollowMePc.purple_x, sFollowMePc.purple_y,
                           suit_x, suit_y, 12)) {
        if (sFollowMePc.purple_x < suit_x) sFollowMePc.purple_x += 4;
        else if (sFollowMePc.purple_x > suit_x) sFollowMePc.purple_x -= 4;
        if (sFollowMePc.purple_y > suit_y) sFollowMePc.purple_y -= 3;
        else if (sFollowMePc.purple_y < suit_y) sFollowMePc.purple_y += 3;
        sFollowMePc.contact_cooldown = 0u;
        return;
    }

    sFollowMePc.purple_x = suit_x;
    sFollowMePc.purple_y = suit_y;
    ++sFollowMePc.contact_cooldown;
    if (sFollowMePc.contact_cooldown >= 96u) {
        sFollowMePc.phase = FOLLOW_PHASE_SPRINGLOCK;
        sFollowMePc.phase_frames = 1u;
        sFollowMePc.contact_cooldown = 0u;
    }
}

static void follow_me_pc_update(Game *game, const InputState *input)
{
    if (!sFollowMePc.initialized ||
        sFollowMePc.chapter != sFinishing.minigame_chapter) {
        follow_me_pc_reset(sFinishing.minigame_chapter);
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
        if (sFollowMePc.phase_frames >= 150u) {
            memset(&sFollowMePc, 0, sizeof(sFollowMePc));
            finishing_complete_minigame(game);
            return;
        }
    } else if (sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK) {
        ++sFollowMePc.phase_frames;
        if (!sFollowMePc.crush_played && sFollowMePc.phase_frames >= 112u) {
            sFollowMePc.crush_played = true;
            audio_play(AUDIO_CUE_MINIGAME_CRUSH, 0.92f, false);
        }
        if (sFollowMePc.phase_frames >= 330u) {
            memset(&sFollowMePc, 0, sizeof(sFollowMePc));
            finishing_complete_minigame(game);
            return;
        }
    } else {
        if (sFollowMePc.phase != FOLLOW_PHASE_SUIT)
            follow_me_pc_move_player(input);
        else
            sFollowMePc.moving = false;

        if (sFollowMePc.phase == FOLLOW_PHASE_GUIDE) {
            follow_me_pc_update_shadow();
            follow_me_pc_update_room_transitions();
        } else if (sFollowMePc.phase == FOLLOW_PHASE_ERR) {
            ++sFollowMePc.phase_frames;

            /* Event 75 sets ERR value to 200. Events 76/77 keep it visible
             * while the value is positive and subtract one per tick. Event
             * 107 arms the trigger afterwards; event 108 creates `man 1` only
             * when the hit box overlaps trigger 353 at source (501,387). */
            if (sFollowMePc.phase_frames >= 200u &&
                follow_me_pc_near(sFollowMePc.player_x,
                                  sFollowMePc.player_y,
                                  follow_me_pc_grid_x(501),
                                  follow_me_pc_grid_y(387), 28)) {
                sFollowMePc.phase = FOLLOW_PHASE_HUNT;
                sFollowMePc.phase_frames = 0u;

                /* Event 108 creates man1 relative to leave-top (499,-1) with
                 * offset (+3,+15): exact source start (502,14). */
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
