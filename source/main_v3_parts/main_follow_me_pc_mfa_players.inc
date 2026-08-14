/* Exact Follow Me actor rendering from fivenights3-94.mfa.
 *
 * The old compatibility payload cropped the Clickteam canvases then enlarged
 * them with an integer scale. These paths use the original PC image IDs,
 * per-frame hotspots and the same exact 5/8 frame transform as the scenery.
 */

static int follow_me_pc_mfa_anim_frame(uint32_t ticks, int speed)
{
    if (ticks == 0u || speed <= 0) return 0;
    const uint32_t advances = (ticks * (uint32_t)speed - 1u) / 100u;
    return (int)(advances & 1u);
}

static int follow_me_pc_mfa_player_anim_frame(void)
{
    if (!sFollowMePc.moving || sFollowMePc.move_frames == 0u) return 0;

    /* Active297, bonnie walk, chica walk, foxy walk and child all use a
     * two-frame direction at Clickteam speed 5. */
    return follow_me_pc_mfa_anim_frame(sFollowMePc.move_frames, 5);
}

static FollowMePcSprite follow_me_pc_player_sprite(void)
{
    int base = FOLLOW_ME_PC_FREDDY_0;
    if (sFollowMePc.chapter == 2) base = FOLLOW_ME_PC_BONNIE_0;
    else if (sFollowMePc.chapter == 3) base = FOLLOW_ME_PC_CHICA_0;
    else if (sFollowMePc.chapter == 4) base = FOLLOW_ME_PC_FOXY_0;
    else if (sFollowMePc.chapter == 5) base = FOLLOW_ME_PC_GHOST_0;

    /* MFA direction indices: 0=East, 8=North, 16=West, 24=South. */
    int pair = 0;
    switch (sFollowMePc.direction) {
        case FOLLOW_DIR_RIGHT: pair = 0; break;
        case FOLLOW_DIR_UP:    pair = 2; break;
        case FOLLOW_DIR_LEFT:  pair = 4; break;
        case FOLLOW_DIR_DOWN:  pair = 6; break;
        default: break;
    }

    return (FollowMePcSprite)(base + pair + follow_me_pc_mfa_player_anim_frame());
}

static const FollowMeMfaTexture *follow_me_pc_mfa_base_actor_texture(
    FollowMePcSprite sprite)
{
    static const FollowMeMfaTexture *const frames[46] = {
        /* 0..7 Freddy */
        &gFollowMfaPlayerFreddyRight0, &gFollowMfaPlayerFreddyRight1,
        &gFollowMfaPlayerFreddyUp0,    &gFollowMfaPlayerFreddyUp1,
        &gFollowMfaPlayerFreddyLeft0,  &gFollowMfaPlayerFreddyLeft1,
        &gFollowMfaPlayerFreddyDown0,  &gFollowMfaPlayerFreddyDown1,
        /* 8..15 Bonnie */
        &gFollowMfaPlayerBonnieRight0, &gFollowMfaPlayerBonnieRight1,
        &gFollowMfaPlayerBonnieUp0,    &gFollowMfaPlayerBonnieUp1,
        &gFollowMfaPlayerBonnieLeft0,  &gFollowMfaPlayerBonnieLeft1,
        &gFollowMfaPlayerBonnieDown0,  &gFollowMfaPlayerBonnieDown1,
        /* 16..23 Chica */
        &gFollowMfaPlayerChicaRight0,  &gFollowMfaPlayerChicaRight1,
        &gFollowMfaPlayerChicaUp0,     &gFollowMfaPlayerChicaUp1,
        &gFollowMfaPlayerChicaLeft0,   &gFollowMfaPlayerChicaLeft1,
        &gFollowMfaPlayerChicaDown0,   &gFollowMfaPlayerChicaDown1,
        /* 24..31 Foxy */
        &gFollowMfaPlayerFoxyRight0,   &gFollowMfaPlayerFoxyRight1,
        &gFollowMfaPlayerFoxyUp0,      &gFollowMfaPlayerFoxyUp1,
        &gFollowMfaPlayerFoxyLeft0,    &gFollowMfaPlayerFoxyLeft1,
        &gFollowMfaPlayerFoxyDown0,    &gFollowMfaPlayerFoxyDown1,
        /* 32..37 Shadow Freddy: right, down, up */
        &gFollowMfaShadowRight0, &gFollowMfaShadowRight1,
        &gFollowMfaShadowDown0,  &gFollowMfaShadowDown1,
        &gFollowMfaShadowUp0,    &gFollowMfaShadowUp1,
        /* 38..45 child / fifth spirit */
        &gFollowMfaPlayerGhostRight0, &gFollowMfaPlayerGhostRight1,
        &gFollowMfaPlayerGhostUp0,    &gFollowMfaPlayerGhostUp1,
        &gFollowMfaPlayerGhostLeft0,  &gFollowMfaPlayerGhostLeft1,
        &gFollowMfaPlayerGhostDown0,  &gFollowMfaPlayerGhostDown1,
    };

    if ((int)sprite < 0 || (int)sprite >= 46) return NULL;
    return frames[(int)sprite];
}

static FollowMePcSprite follow_me_pc_mfa_shadow_exact_sprite(
    FollowMePcSprite requested)
{
    static int last_vertical = -999;
    static int last_horizontal = -999;
    static int last_kind = -999;
    static uint32_t born_frame = 0u;

    const int offset = (int)requested - (int)FOLLOW_ME_PC_SHADOW_0;
    const int direction_pair = (offset / 2) * 2;

    /* Shadow objects are newly created by events95..104 on room entry. Reset
     * the animation phase when the guide cell/kind changes, then run the exact
     * two-frame speed-5 animation. */
    if (last_vertical != sFollowMfaVertical ||
        last_horizontal != sFollowMfaHorizontal ||
        last_kind != sFollowMfaShadowKind) {
        last_vertical = sFollowMfaVertical;
        last_horizontal = sFollowMfaHorizontal;
        last_kind = sFollowMfaShadowKind;
        born_frame = sFollowMePc.frames;
    }

    const uint32_t age = sFollowMePc.frames - born_frame;
    const int frame = follow_me_pc_mfa_anim_frame(age, 5);
    return (FollowMePcSprite)((int)FOLLOW_ME_PC_SHADOW_0 +
                              direction_pair + frame);
}

static const FollowMeMfaTexture *follow_me_pc_mfa_actor_texture(
    FollowMePcSprite sprite)
{
    if (sprite >= FOLLOW_ME_PC_SHADOW_0 &&
        sprite <= FOLLOW_ME_PC_SHADOW_5) {
        sprite = follow_me_pc_mfa_shadow_exact_sprite(sprite);
    }

    if ((int)sprite >= (int)FOLLOW_ME_PC_FREDDY_0 &&
        (int)sprite <= (int)FOLLOW_ME_PC_GHOST_7) {
        return follow_me_pc_mfa_base_actor_texture(sprite);
    }

    /* Handle352 `man 1`: images382/383, speed10. The event renderer only
     * requests PURPLE_WALK_0/1; choose the exact frame from phase time rather
     * than the old /7 toggle. */
    if (sprite == FOLLOW_ME_PC_PURPLE_WALK_0 ||
        sprite == FOLLOW_ME_PC_PURPLE_WALK_1) {
        return follow_me_pc_mfa_anim_frame(sFollowMePc.phase_frames, 10)
            ? &gFollowMfaHuntMan1 : &gFollowMfaHuntMan0;
    }

    /* Exact one-frame dismantled Actives 355/357/359/362. */
    switch (sprite) {
        case FOLLOW_ME_PC_WRECK_0: return &gFollowMfaWreckFreddy;
        case FOLLOW_ME_PC_WRECK_1: return &gFollowMfaWreckBonnie;
        case FOLLOW_ME_PC_WRECK_2: return &gFollowMfaWreckChica;
        case FOLLOW_ME_PC_WRECK_3: return &gFollowMfaWreckFoxy;
        default: break;
    }

    return NULL;
}

static void follow_me_pc_draw_sprite(FollowMePcSprite sprite, int x, int y,
                                     int scale)
{
    const FollowMeMfaTexture *mfa = follow_me_pc_mfa_actor_texture(sprite);
    if (mfa != NULL) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         x - mfa->hotspot_x,
                         y - mfa->hotspot_y,
                         (int)mfa->texture.width,
                         (int)mfa->texture.height,
                         &mfa->texture);
        return;
    }

    /* Purple panic / old spring enums remain as a safety fallback, although
     * the current chapter-5 renderer uses direct MFA images for both paths. */
    follow_me_pc_draw_sprite_legacy(sprite, x, y, scale);
}
