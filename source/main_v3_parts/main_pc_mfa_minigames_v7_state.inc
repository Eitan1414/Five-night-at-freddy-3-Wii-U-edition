#include "platform/storage.h"

/* Seventh MFA fidelity pass: reproduce the original secret/cake progression.
 *
 * fivenights3-94.mfa uses a small set of persistent INI keys rather than a
 * generic "minigame completed" flag:
 *   cake  - obtained from Mangle's hidden pickup
 *   k1    - cake delivered in BB's Air Adventure
 *   k2    - cake delivered in Chica's Party
 *   k3    - cake delivered in Stage 01
 *   k4    - cake delivered in RWQFSFASXC
 *   goodend - set by Happiest Day once k1..k4 are present
 *
 * The portable progress mask already has one bit per secret minigame, so the
 * five prerequisite bits are used as the equivalent of cake/k1..k4 and the
 * Happiest Day bit is goodend.  Normal exits deliberately do not set those
 * bits.  The separate `bb` INI flag is persisted in a tiny Wii U save-side
 * file; old saves with secret progress infer it so existing users are not
 * stranded.
 *
 * The cake objects are hidden triggers in the original Event Editor.  Once
 * activated their alterable value is incremented every frame.  BB/Chica/
 * Stage/RWQ set win at 200; Happiest Day runs its longer celebration and sets
 * win at 1300.  Those exact thresholds are reproduced here. */

typedef struct PcMfaV7State {
    bool active;
    SecretMinigameKind kind;
    bool cake_sequence;
    bool final_sequence;
    uint32_t sequence_frames;
} PcMfaV7State;

static PcMfaV7State sPcMfaV7;

#define PC_MFA_V7_BB_SAVE_PATH "secret_bb.dat"
static bool sPcMfaV7BbLoaded = false;
static bool sPcMfaV7BbUnlocked = false;

static void pc_mfa_v7_load_bb_flag(void)
{
    if (sPcMfaV7BbLoaded) return;
    sPcMfaV7BbLoaded = true;
    uint8_t value = 0u;
    size_t bytes_read = 0u;
    if (storage_is_ready() &&
        storage_read(PC_MFA_V7_BB_SAVE_PATH, &value, sizeof(value),
                     &bytes_read) &&
        bytes_read == sizeof(value) && value == 1u)
        sPcMfaV7BbUnlocked = true;
}

static bool pc_mfa_v7_bb_unlocked(void)
{
    pc_mfa_v7_load_bb_flag();
    return sPcMfaV7BbUnlocked;
}

static void pc_mfa_v7_unlock_bb(void)
{
    pc_mfa_v7_load_bb_flag();
    if (sPcMfaV7BbUnlocked) return;
    sPcMfaV7BbUnlocked = true;
    const uint8_t value = 1u;
    if (storage_is_ready())
        (void)storage_write_atomic(PC_MFA_V7_BB_SAVE_PATH, &value, sizeof(value));
}

static const PcMfaPoint kPcMfaV7MangleGetThis[2] = {
    {3367, 1459}, {413, 1301},
};
static const PcMfaPoint kPcMfaV7ChicaGetThis = {2540, 1754};
static const PcMfaPoint kPcMfaV7ChicaCakeTrigger = {402, 984};
static const PcMfaPoint kPcMfaV7StageGetThis = {3258, 1768};
static const PcMfaPoint kPcMfaV7ShadowGetThis = {3258, 1768};
static const PcMfaPoint kPcMfaV7HappiestGetThis = {2540, 1754};
static const PcMfaPoint kPcMfaV7ChicaCakeChild = {271, 1014};
static const PcMfaPoint kPcMfaV7StageCakeChild = {2854, 598};
static const PcMfaPoint kPcMfaV7ShadowCakeChild = {1084, 1474};
static const PcMfaPoint kPcMfaV7HappiestFinalChild = {2854, 616};
static const PcMfaPoint kPcMfaV7HappiestExit = {198, 601};

static bool pc_mfa_v7_progress(int minigame)
{
    return progress_save_is_secret_minigame_completed(minigame);
}

static bool pc_mfa_v7_cake_owned(void)
{
    /* Mangle's Quest is minigame #2: its hidden pickup writes INI "cake"=1. */
    return pc_mfa_v7_progress(2);
}

static bool pc_mfa_v7_k1(void) { return pc_mfa_v7_progress(1); }
static bool pc_mfa_v7_k2(void) { return pc_mfa_v7_progress(3); }
static bool pc_mfa_v7_k3(void) { return pc_mfa_v7_progress(4); }
static bool pc_mfa_v7_k4(void) { return pc_mfa_v7_progress(5); }

static bool pc_mfa_v7_all_happiest_children(void)
{
    return pc_mfa_v7_k1() && pc_mfa_v7_k2() &&
           pc_mfa_v7_k3() && pc_mfa_v7_k4();
}

static void pc_mfa_v7_infer_bb_flag(void)
{
    if (pc_mfa_v7_bb_unlocked()) return;
    /* Compatibility with saves made by the earlier Wii U reconstruction:
     * any already-recorded k flag implies that the hidden route had been
     * reached before the dedicated persistent bb flag existed. */
    if (pc_mfa_v7_k1() || pc_mfa_v7_k2() || pc_mfa_v7_k3() ||
        pc_mfa_v7_k4() || pc_mfa_v7_progress(6))
        pc_mfa_v7_unlock_bb();
}

static int pc_mfa_v7_sequence_progress_id(bool final_sequence)
{
    if (final_sequence) return 6; /* goodend */
    switch (sSecret.kind) {
        case SECRET_MINIGAME_BB: return 1;      /* k1 */
        case SECRET_MINIGAME_CHICA: return 3;   /* k2 */
        case SECRET_MINIGAME_STAGE01: return 4; /* k3 */
        case SECRET_MINIGAME_SHADOW_BONNIE: return 5; /* k4 */
        default: return 0;
    }
}

static void pc_mfa_v7_normal_finish(Game *game)
{
    if (sSecret.completed) return;
    sSecret.completed = true;
    sSecret.completed_frames = 0u;
    audio_play(AUDIO_CUE_END, 0.76f, false);
    game->dirty = true;
}

static void pc_mfa_v7_begin_cake_sequence(Game *game, bool final_sequence)
{
    if (sPcMfaV7.cake_sequence || sPcMfaV7.final_sequence) return;
    sPcMfaV7.cake_sequence = !final_sequence;
    sPcMfaV7.final_sequence = final_sequence;
    sPcMfaV7.sequence_frames = 0u;
    /* The PC writes cake/k1..k4/goodend at the trigger event itself, before
     * the visible 100/200/300/1300 animation chain finishes. */
    const int progress_id = pc_mfa_v7_sequence_progress_id(final_sequence);
    if (progress_id > 0) progress_save_complete_secret_minigame(progress_id);
    pc_minigame_sfx_play(PC_MINIGAME_SFX_GET2, 0.58f, false);
    game->dirty = true;
}

static void pc_mfa_v7_tick_secret_sequence(Game *game)
{
    if (!sPcMfaV7.cake_sequence && !sPcMfaV7.final_sequence) return;
    ++sPcMfaV7.sequence_frames;

    if (sPcMfaV7.final_sequence) {
        /* Marion Event groups 27-29 animate at 100/200/300.  Group 49 sets
         * win when the final big-cake alterable value reaches 1300. */
        if (sPcMfaV7.sequence_frames == 100u ||
            sPcMfaV7.sequence_frames == 200u ||
            sPcMfaV7.sequence_frames == 300u)
            pc_minigame_sfx_play(PC_MINIGAME_SFX_GET, 0.42f, false);
        if (sPcMfaV7.sequence_frames >= 1300u)
            secret_complete(game);
    } else {
        /* BB/Chica/Stage/RWQ switch the child animation at 100 and set win at
         * 200 from the visible cake object's alterable value. */
        if (sPcMfaV7.sequence_frames == 100u)
            pc_minigame_sfx_play(PC_MINIGAME_SFX_GET, 0.42f, false);
        if (sPcMfaV7.sequence_frames >= 200u)
            pc_mfa_v7_normal_finish(game);
    }
}

static bool pc_mfa_v7_point_on_balloon(int x, int y)
{
    /* The original balloon/glitch routes are created from the persistent bb
     * flag.  They must not be collidable before that hidden route is unlocked. */
    if (!pc_mfa_v7_bb_unlocked()) return false;
    return pc_mfa_v5_point_on_balloon(x, y);
}

static bool pc_mfa_v7_sensor_floor(int ox, int oy)
{
    const int x = sSecret.x + ox;
    const int y = sSecret.y + oy;
    return pc_mfa_v5_point_in_solid(x, y) || pc_mfa_v7_point_on_balloon(x, y);
}

static void pc_mfa_v7_reset_physics(void)
{
    memset(&sPcMfaV5, 0, sizeof(sPcMfaV5));
    sPcMfaV5.active = true;
    sPcMfaV5.kind = sSecret.kind;
    sPcMfaV5.grounded = pc_mfa_v7_sensor_floor(3, 60);
}

static bool pc_mfa_v7_apply_physics(const InputState *input)
{
    const bool was_grounded = sPcMfaV5.grounded;
    const int old_x = sSecret.x;
    const int horizontal =
        sSecret.kind == SECRET_MINIGAME_HAPPIEST_DAY ? 20 : 15;

    if (input_is_held(input, GAME_BUTTON_LEFT) &&
        !pc_mfa_v5_sensor_solid(-33, -3))
        sSecret.x -= horizontal;
    if (input_is_held(input, GAME_BUTTON_RIGHT) &&
        !pc_mfa_v5_sensor_solid(39, -3))
        sSecret.x += horizontal;
    sPcMfaV5.moved_horizontal = sSecret.x != old_x;

    if (!input_is_held(input, GAME_BUTTON_UP))
        sPcMfaV5.jump_latch = false;

    bool jump_started = false;
    if (input_is_held(input, GAME_BUTTON_UP) &&
        !sPcMfaV5.jump_latch && sPcMfaV5.jump_ticks == 0 &&
        pc_mfa_v7_sensor_floor(3, 60)) {
        sPcMfaV5.jump_ticks = 7;
        sPcMfaV5.jump_latch = true;
        sPcMfaV5.jump_clock = 0u;
        jump_started = true;
    }

    if (pc_mfa_v5_sensor_solid(5, -31)) {
        sPcMfaV5.jump_ticks = 0;
        sPcMfaV5.jump_clock = 0u;
    }

    if (sPcMfaV5.jump_ticks > 0) {
        ++sPcMfaV5.jump_clock;
        if (sPcMfaV5.jump_clock >= 4u) {
            sPcMfaV5.jump_clock = 0u;
            --sPcMfaV5.jump_ticks;
            sSecret.y -= 20;
            if (pc_mfa_v5_sensor_solid(5, -31))
                sPcMfaV5.jump_ticks = 0;
        }
    } else {
        sPcMfaV5.jump_clock = 0u;
        const bool floor1 = pc_mfa_v7_sensor_floor(3, 41);
        const bool floor2 = pc_mfa_v7_sensor_floor(3, 60);
        if (!floor1 && !floor2 && !pc_mfa_v5_on_zip_up()) {
            ++sPcMfaV5.gravity_clock;
            if (sPcMfaV5.gravity_clock >= 6u) {
                sPcMfaV5.gravity_clock = 0u;
                sSecret.y += 10;
            }
        } else {
            sPcMfaV5.gravity_clock = 0u;
        }
    }

    if (pc_mfa_v7_sensor_floor(3, 30))
        sSecret.y -= 5;

    sSecret.x = pc_mfa_clamp(sSecret.x, -256, 3400);
    sSecret.y = pc_mfa_clamp(sSecret.y, -800, 2340);
    sPcMfaV5.grounded =
        pc_mfa_v7_sensor_floor(3, 60) || pc_mfa_v7_sensor_floor(3, 41);

    if (jump_started)
        pc_minigame_sfx_play(pc_mfa_v2_jump_sound(), 0.50f, false);
    if (!was_grounded && sPcMfaV5.grounded && sPcMfaV5.jump_ticks == 0)
        pc_minigame_sfx_play(PC_MINIGAME_SFX_LAND, 0.42f, false);
    return jump_started;
}

static void pc_mfa_v7_collect_bb(Game *game)
{
    /* Only the seven instances named "collect" decrement Counter.  The
     * separate "collect 2" instance at 641,42 is not referenced by that event. */
    for (int i = 0; i < 7; ++i) {
        const uint8_t bit = (uint8_t)(1u << i);
        if ((sSecret.collected & bit) != 0u) continue;
        if (!pc_mfa_v3_near(kPcMfaBbCollect[i], 34)) continue;
        sSecret.collected |= bit;
        game->dirty = true;
    }
}
