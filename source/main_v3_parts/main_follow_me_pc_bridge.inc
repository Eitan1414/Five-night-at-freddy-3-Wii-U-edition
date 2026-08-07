/*
 * Progression bridge for the PC-style Follow Me replacement.
 *
 * Deliberately does not depend on Game/ProgressSave yet.  The active game can
 * translate the returned result into its existing night/ending transitions,
 * which lets this replacement be validated beside the old implementation.
 */

#ifndef FNAF3_FOLLOW_ME_PC_BRIDGE_INC
#define FNAF3_FOLLOW_ME_PC_BRIDGE_INC

typedef enum FollowPcResult {
    FOLLOW_PC_RESULT_RUNNING = 0,
    FOLLOW_PC_RESULT_NEXT_NIGHT,
    FOLLOW_PC_RESULT_BAD_ENDING
} FollowPcResult;

typedef struct FollowPcBridge {
    FollowPcState state;
    int source_night;
    bool completion_reported;
} FollowPcBridge;

static FollowPcChapter follow_pc_chapter_for_night(int completed_night)
{
    switch (completed_night) {
        case 1: return FOLLOW_PC_FREDDY;
        case 2: return FOLLOW_PC_BONNIE;
        case 3: return FOLLOW_PC_CHICA;
        case 4: return FOLLOW_PC_FOXY;
        case 5: return FOLLOW_PC_FINAL_CHILD;
        default: return FOLLOW_PC_FREDDY;
    }
}

static bool follow_pc_has_chapter_after_night(int completed_night)
{
    return completed_night >= 1 && completed_night <= 5;
}

static void follow_pc_bridge_begin(FollowPcBridge *bridge,
                                   int completed_night)
{
    if (!bridge) return;
    memset(bridge, 0, sizeof(*bridge));
    bridge->source_night = completed_night;
    follow_pc_reset(&bridge->state,
                    follow_pc_chapter_for_night(completed_night));
}

static FollowPcResult follow_pc_bridge_tick(FollowPcBridge *bridge,
                                             int axis_x,
                                             int axis_y)
{
    if (!bridge) return FOLLOW_PC_RESULT_RUNNING;

    follow_pc_tick(&bridge->state, axis_x, axis_y);
    if (!follow_pc_is_complete(&bridge->state))
        return FOLLOW_PC_RESULT_RUNNING;

    if (bridge->completion_reported)
        return bridge->source_night >= 5
            ? FOLLOW_PC_RESULT_BAD_ENDING
            : FOLLOW_PC_RESULT_NEXT_NIGHT;

    bridge->completion_reported = true;
    bridge->state.active = false;
    return bridge->source_night >= 5
        ? FOLLOW_PC_RESULT_BAD_ENDING
        : FOLLOW_PC_RESULT_NEXT_NIGHT;
}

#endif /* FNAF3_FOLLOW_ME_PC_BRIDGE_INC */
