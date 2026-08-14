/* Exact playable Follow Me actor rendering.
 *
 * The recovered compatibility payload crops these actors and redraws them at
 * an integer x3 scale.  The original Clickteam Actives use 200x200 image-bank
 * canvases with per-frame hotspots.  The generated gFollowMfaPlayer* textures
 * preserve those canvases/hotspots through the same 1024x768 -> 640x480 5/8
 * transform used by the rest of the Follow Me frame.
 */

static int follow_me_pc_mfa_player_anim_frame(void)
{
    if (!sFollowMePc.moving || sFollowMePc.move_frames == 0u) return 0;

    /* Active 297 / bonnie walk / chica walk / foxy walk all use animation 0
     * at min=max speed 5. Fusion advances the two-frame animation as its
     * animation counter crosses 100, so preserve that cadence instead of the
     * old arbitrary move_frames/7 toggle. */
    const uint32_t advances =
        (sFollowMePc.move_frames * 5u - 1u) / 100u;
    return (int)(advances & 1u);
}

static FollowMePcSprite follow_me_pc_player_sprite(void)
{
    int base = FOLLOW_ME_PC_FREDDY_0;
    if (sFollowMePc.chapter == 2) base = FOLLOW_ME_PC_BONNIE_0;
    else if (sFollowMePc.chapter == 3) base = FOLLOW_ME_PC_CHICA_0;
    else if (sFollowMePc.chapter == 4) base = FOLLOW_ME_PC_FOXY_0;
    else if (sFollowMePc.chapter == 5) base = FOLLOW_ME_PC_GHOST_0;

    /* MFA direction indices are 0=E, 8=N, 16=W, 24=S. The legacy cropped
     * payload used a different ordering, so map the actual Clickteam order here. */
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

static const FollowMeMfaTexture *follow_me_pc_mfa_player_texture(
    FollowMePcSprite sprite)
{
    static const FollowMeMfaTexture *const frames[32] = {
        &gFollowMfaPlayerFreddyRight0, &gFollowMfaPlayerFreddyRight1,
        &gFollowMfaPlayerFreddyUp0,    &gFollowMfaPlayerFreddyUp1,
        &gFollowMfaPlayerFreddyLeft0,  &gFollowMfaPlayerFreddyLeft1,
        &gFollowMfaPlayerFreddyDown0,  &gFollowMfaPlayerFreddyDown1,

        &gFollowMfaPlayerBonnieRight0, &gFollowMfaPlayerBonnieRight1,
        &gFollowMfaPlayerBonnieUp0,    &gFollowMfaPlayerBonnieUp1,
        &gFollowMfaPlayerBonnieLeft0,  &gFollowMfaPlayerBonnieLeft1,
        &gFollowMfaPlayerBonnieDown0,  &gFollowMfaPlayerBonnieDown1,

        &gFollowMfaPlayerChicaRight0,  &gFollowMfaPlayerChicaRight1,
        &gFollowMfaPlayerChicaUp0,     &gFollowMfaPlayerChicaUp1,
        &gFollowMfaPlayerChicaLeft0,   &gFollowMfaPlayerChicaLeft1,
        &gFollowMfaPlayerChicaDown0,   &gFollowMfaPlayerChicaDown1,

        &gFollowMfaPlayerFoxyRight0,   &gFollowMfaPlayerFoxyRight1,
        &gFollowMfaPlayerFoxyUp0,      &gFollowMfaPlayerFoxyUp1,
        &gFollowMfaPlayerFoxyLeft0,    &gFollowMfaPlayerFoxyLeft1,
        &gFollowMfaPlayerFoxyDown0,    &gFollowMfaPlayerFoxyDown1,
    };

    if ((int)sprite < (int)FOLLOW_ME_PC_FREDDY_0 ||
        (int)sprite > (int)FOLLOW_ME_PC_FOXY_7)
        return NULL;
    return frames[(int)sprite - (int)FOLLOW_ME_PC_FREDDY_0];
}

static void follow_me_pc_draw_sprite(FollowMePcSprite sprite, int x, int y,
                                     int scale)
{
    const FollowMeMfaTexture *mfa = follow_me_pc_mfa_player_texture(sprite);
    if (mfa != NULL) {
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         x - mfa->hotspot_x,
                         y - mfa->hotspot_y,
                         (int)mfa->texture.width,
                         (int)mfa->texture.height,
                         &mfa->texture);
        return;
    }

    /* Shadow Freddy, ghosts and dismantled parts still use the recovered PC
     * payload until each of those objects receives the same MFA image-bank
     * audit. */
    follow_me_pc_draw_sprite_legacy(sprite, x, y, scale);
}
