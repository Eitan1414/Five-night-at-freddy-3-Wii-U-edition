static void follow_me_pc_draw_remains(void)
{
    /* Events 120..123 recreate the previously dismantled suits in source cell
     * (2,5). These are the exact creation positions relative to leave-top. */
    if (!follow_me_pc_mfa_cell(2, 5)) return;

    if (sFollowMePc.chapter >= 2)
        follow_me_pc_draw_sprite(FOLLOW_ME_PC_WRECK_0,
                                 follow_me_pc_grid_x(444),
                                 follow_me_pc_grid_y(572), 2);
    if (sFollowMePc.chapter >= 3)
        follow_me_pc_draw_sprite(FOLLOW_ME_PC_WRECK_1,
                                 follow_me_pc_grid_x(648),
                                 follow_me_pc_grid_y(214), 2);
    if (sFollowMePc.chapter >= 4)
        follow_me_pc_draw_sprite(FOLLOW_ME_PC_WRECK_2,
                                 follow_me_pc_grid_x(342),
                                 follow_me_pc_grid_y(302), 2);
    if (sFollowMePc.chapter >= 5)
        follow_me_pc_draw_sprite(FOLLOW_ME_PC_WRECK_3,
                                 follow_me_pc_grid_x(640),
                                 follow_me_pc_grid_y(454), 2);
}

static void follow_me_pc_draw_safe_room_ghosts(void)
{
    /* Event 157 makes the four `child 2` instances visible in cell (1,5).
     * Event 181 destroys them when the springlock state reaches 5. */
    if (!follow_me_pc_mfa_cell(1, 5) ||
        !sFollowMfaFinaleSpiritsVisible) return;

    static const int source_positions[4][2] = {
        {352, 692}, {443, 692}, {562, 692}, {673, 692}
    };
    for (int index = 0; index < 4; ++index)
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_GHOST_2 + (index & 1)),
            follow_me_pc_grid_x(source_positions[index][0]),
            follow_me_pc_grid_y(source_positions[index][1]), 3);
}

static void follow_me_pc_draw_rain(uint32_t frame)
{
    /* Original PC image-bank object. The exact drop-fall creation logic is a
     * separate environment pass; these are the 12 original MFA anchors. */
    static const int pc_x[12] = {
        44, 126, 208, 290, 372, 454,
        536, 618, 700, 782, 870, 946
    };
    static const int pc_y0[12] = {
        -25, -25, -25, -25, -25, -25,
        -25, -25, -25, -25, -27, -23
    };

    for (int index = 0; index < 12; ++index) {
        const uint32_t phase = (frame * 4u + (uint32_t) index * 61u) % 850u;
        const int pc_y = pc_y0[index] + (int) phase;
        follow_me_pc_draw_mfa(&gFollowMfaRaindrop, pc_x[index], pc_y);
    }
}

static void follow_me_pc_draw_mfa_at_wii(const FollowMeMfaTexture *asset,
                                         int x, int y)
{
    if (asset == NULL || asset->texture.width == 0u ||
        asset->texture.height == 0u) return;
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     x - asset->hotspot_x,
                     y - asset->hotspot_y,
                     (int)asset->texture.width,
                     (int)asset->texture.height,
                     &asset->texture);
}

static bool follow_me_pc_finale_anim_second(uint32_t ticks, int speed)
{
    if (ticks == 0u || speed <= 0) return false;
    const uint32_t advances =
        (ticks * (uint32_t)speed - 1u) / 100u;
    return (advances & 1u) != 0u;
}

static const FollowMeMfaTexture *follow_me_pc_finale_man_texture(void)
{
    const bool second = follow_me_pc_finale_anim_second(
        sFollowMePc.phase_frames, 20);
    /* Handle 363 `man`: direction 0 = images 453/454, direction 16 = 455/456. */
    if (sFollowMePc.purple_target == 1 || sFollowMePc.purple_target == 2)
        return second ? &gFollowMfaFinalManLeft1 : &gFollowMfaFinalManLeft0;
    return second ? &gFollowMfaFinalManRight1 : &gFollowMfaFinalManRight0;
}

static const FollowMeMfaTexture *follow_me_pc_finale_run_texture(void)
{
    /* Handle 368 `man run`, animation 0, speed 20: images 458/460. */
    return follow_me_pc_finale_anim_second(sFollowMePc.phase_frames, 20)
        ? &gFollowMfaFinalManRun1 : &gFollowMfaFinalManRun0;
}

static const FollowMeMfaTexture *follow_me_pc_spring_texture(void)
{
    /* Handle 369 `Active 4`, exact animations selected by events 167..172:
     * 12->[472], 13->[473], 14->[486,489] speed5,
     * 15->[490,494] speed50, 16->[495,496] speed50,
     * 17->[497,498] speed25. */
    switch (sFollowMfaSpringState) {
        case 1:
            return &gFollowMfaSpring12;
        case 2:
            return &gFollowMfaSpring13;
        case 3:
            return follow_me_pc_finale_anim_second(sFollowMfaSpringTimer, 5)
                ? &gFollowMfaSpring14B : &gFollowMfaSpring14A;
        case 4:
            return follow_me_pc_finale_anim_second(sFollowMfaSpringTimer, 50)
                ? &gFollowMfaSpring15B : &gFollowMfaSpring15A;
        case 5:
            return follow_me_pc_finale_anim_second(sFollowMfaSpringTimer, 50)
                ? &gFollowMfaSpring16B : &gFollowMfaSpring16A;
        case 6:
            return follow_me_pc_finale_anim_second(sFollowMfaSpringTimer, 25)
                ? &gFollowMfaSpring17B : &gFollowMfaSpring17A;
        default:
            return &gFollowMfaFinalSuitIdle;
    }
}

static FollowMePcSprite follow_me_pc_mfa_shadow_sprite(void)
{
    int base = FOLLOW_ME_PC_SHADOW_0;
    if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_DOWN)
        base = FOLLOW_ME_PC_SHADOW_2;
    else if (sFollowMfaShadowKind == FOLLOW_MFA_SHADOW_UP)
        base = FOLLOW_ME_PC_SHADOW_4;
    return (FollowMePcSprite) (base + (int) ((sFollowMePc.frames / 9u) & 1u));
}

static void follow_me_pc_draw(const Game *game)
{
    follow_me_pc_draw_room(game);
    follow_me_pc_draw_remains();
    follow_me_pc_draw_rain(sFollowMePc.frames);

    if (sFollowMePc.frames < 300u)
        follow_me_pc_draw_mfa(&gFollowMfaControls, 502, 158);

    if (sFollowMePc.chapter < 5 &&
        sFollowMfaShadowKind != FOLLOW_MFA_SHADOW_NONE) {
        follow_me_pc_draw_sprite(follow_me_pc_mfa_shadow_sprite(),
                                 follow_me_pc_grid_x(sFollowMfaShadowSourceX),
                                 follow_me_pc_grid_y(sFollowMfaShadowSourceY),
                                 3);
    }

    if (sFollowMePc.chapter < 5 && sFollowMfaFollowTextTicks > 0u)
        follow_me_pc_draw_mfa(&gFollowMfaFollowText, 10, 660);

    if (sFollowMePc.phase == FOLLOW_PHASE_ERR &&
        sFollowMePc.phase_frames < 200u)
        follow_me_pc_draw_mfa(&gFollowMfaErr, 222, 618);

    follow_me_pc_draw_safe_room_ghosts();

    if (sFollowMePc.phase == FOLLOW_PHASE_HUNT) {
        const int frame = (int) ((sFollowMePc.frames / 7u) & 1u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_PURPLE_WALK_0 + frame),
            sFollowMePc.purple_x, sFollowMePc.purple_y, 3);
    } else if (sFollowMePc.phase == FOLLOW_PHASE_GHOST_CHASE) {
        /* Event157 actors at their actual MFA coordinates. */
        follow_me_pc_draw_mfa_at_wii(follow_me_pc_finale_man_texture(),
                                     sFollowMePc.purple_x,
                                     sFollowMePc.purple_y);
        follow_me_pc_draw_mfa(&gFollowMfaFinalSuitIdle, 828, 462);
    } else if (sFollowMePc.phase == FOLLOW_PHASE_SUIT) {
        follow_me_pc_draw_mfa_at_wii(follow_me_pc_finale_run_texture(),
                                     sFollowMePc.purple_x,
                                     sFollowMePc.purple_y);
        follow_me_pc_draw_mfa(&gFollowMfaFinalSuitIdle, 828, 462);
    } else if (sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK) {
        /* Active4 remains at the event157 source anchor throughout states1..6. */
        follow_me_pc_draw_mfa(follow_me_pc_spring_texture(), 828, 462);
    }

    if (sFollowMePc.phase == FOLLOW_PHASE_DISMANTLED) {
        const FollowMePcSprite wreck = (FollowMePcSprite)
            (FOLLOW_ME_PC_WRECK_0 + sFollowMePc.chapter - 1);
        follow_me_pc_draw_sprite(wreck, sFollowMePc.player_x,
                                 sFollowMePc.player_y + 12, 2);
    } else if (sFollowMePc.chapter == 5) {
        /* Event181 destroys `child` at springlock state5. */
        if (!(sFollowMePc.phase == FOLLOW_PHASE_SPRINGLOCK &&
              sFollowMfaSpringState >= 5)) {
            follow_me_pc_draw_sprite(follow_me_pc_player_sprite(),
                                     sFollowMePc.player_x,
                                     sFollowMePc.player_y, 3);
        }
    } else if (sFollowMePc.phase != FOLLOW_PHASE_SPRINGLOCK &&
               sFollowMePc.phase != FOLLOW_PHASE_SUIT) {
        follow_me_pc_draw_sprite(follow_me_pc_player_sprite(),
                                 sFollowMePc.player_x,
                                 sFollowMePc.player_y, 3);
    }

    /* Replay still accepts B/Back in the update path, but the original PC
     * Follow Me frame contains no Wii-U-only `B: EXTRAS` overlay. */
    graphics_present(GRAPHICS_TARGET_BOTH);
}
