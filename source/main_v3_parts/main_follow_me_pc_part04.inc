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

typedef struct FollowMeMfaRainDrop {
    bool active;
    int source_x;
    int source_y;
    int alterable_a;
} FollowMeMfaRainDrop;

static FollowMeMfaRainDrop sFollowMfaRainDrops[2];
static uint32_t sFollowMfaRainLastFrame;
static uint32_t sFollowMfaRainRng = 0xA341316Cu;
static int sFollowMfaRainChapter = -1;

static uint32_t follow_me_pc_rain_random(void)
{
    /* The event sheet uses Fusion Random(3) and `Pick one of raindrop`.
     * Preserve their probabilities and selection behavior; as elsewhere in the
     * Wii U port, the exact private Fusion RNG stream is not claimed. */
    sFollowMfaRainRng = sFollowMfaRainRng * 1664525u + 1013904223u;
    return sFollowMfaRainRng;
}

static void follow_me_pc_rain_reset(void)
{
    memset(sFollowMfaRainDrops, 0, sizeof(sFollowMfaRainDrops));
    sFollowMfaRainLastFrame = 0u;
    sFollowMfaRainChapter = sFollowMePc.chapter;
    sFollowMfaRainRng = 0xA341316Cu ^
        ((uint32_t)(sFollowMePc.chapter + 1) * 0x9E3779B9u);
}

static void follow_me_pc_rain_tick(uint32_t frame)
{
    static const int anchor_x[12] = {
        44, 126, 208, 290, 372, 454,
        536, 618, 700, 782, 870, 946
    };
    static const int anchor_y[12] = {
        -25, -25, -25, -25, -25, -25,
        -25, -25, -25, -25, -27, -23
    };

    /* Event 70: if Number of `drop fall` <= 1, every 1000 ms,
     * Random(3)==1, Pick one of the 12 `raindrop` occurrences, then create
     * `drop fall` at the selected occurrence with source offset (0,+1). */
    if ((frame % 60u) == 0u) {
        int count = 0;
        for (int index = 0; index < 2; ++index)
            if (sFollowMfaRainDrops[index].active) ++count;

        if (count <= 1 && (follow_me_pc_rain_random() % 3u) == 1u) {
            const int anchor = (int)(follow_me_pc_rain_random() % 12u);
            for (int index = 0; index < 2; ++index) {
                FollowMeMfaRainDrop *drop = &sFollowMfaRainDrops[index];
                if (drop->active) continue;
                drop->active = true;
                drop->source_x = anchor_x[anchor];
                drop->source_y = anchor_y[anchor] + 1;
                drop->alterable_a = 0;
                break;
            }
        }
    }

    /* Events 71/72 are later in the same Clickteam event sheet, so a drop
     * created on a 1000-ms boundary also receives this 200-ms movement step
     * immediately. A starts at 0; each step subtracts 1 and moves (+1,+30),
     * then the object is destroyed when A <= -20. */
    if ((frame % 12u) == 0u) {
        for (int index = 0; index < 2; ++index) {
            FollowMeMfaRainDrop *drop = &sFollowMfaRainDrops[index];
            if (!drop->active || drop->alterable_a <= -20) continue;
            --drop->alterable_a;
            drop->source_x += 1;
            drop->source_y += 30;
            if (drop->alterable_a <= -20)
                drop->active = false;
        }
    }
}

static void follow_me_pc_draw_rain(uint32_t frame)
{
    /* The 12 image-273 `raindrop` instances live just above the frame and are
     * selection anchors, not twelve continuously falling sprites. Render only
     * the image-368 `drop fall` instances created by events 70..72. */
    if (sFollowMfaRainChapter != sFollowMePc.chapter ||
        frame < sFollowMfaRainLastFrame)
        follow_me_pc_rain_reset();

    while (sFollowMfaRainLastFrame < frame) {
        ++sFollowMfaRainLastFrame;
        follow_me_pc_rain_tick(sFollowMfaRainLastFrame);
    }

    for (int index = 0; index < 2; ++index) {
        const FollowMeMfaRainDrop *drop = &sFollowMfaRainDrops[index];
        if (!drop->active) continue;
        follow_me_pc_draw_mfa(&gFollowMfaDropFall,
                              drop->source_x, drop->source_y);
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
        const int actor_frame = (int) ((sFollowMePc.frames / 7u) & 1u);
        follow_me_pc_draw_sprite((FollowMePcSprite)
            (FOLLOW_ME_PC_PURPLE_WALK_0 + actor_frame),
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
