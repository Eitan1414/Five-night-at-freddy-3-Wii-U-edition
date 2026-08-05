typedef struct CompleteCheatState {
    bool fast_nights;
    bool radar;
    bool aggressive;
    bool no_errors;
    int selection;
    uint32_t locked_notice_frames;
} CompleteCheatState;

static CompleteCheatState sCompleteCheats;
static void complete_graphics_present(uint32_t target);
