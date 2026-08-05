typedef enum FinishingMode {
    FINISHING_NONE = 0,
    FINISHING_EXTRAS,
    FINISHING_FOLLOW_ME,
    FINISHING_NIGHT5_ENDING,
    FINISHING_NIGHT6_ENDING
} FinishingMode;

typedef enum ExtrasPage {
    EXTRAS_PAGE_MAIN = 0,
    EXTRAS_PAGE_GALLERY,
    EXTRAS_PAGE_JUMPSCARES,
    EXTRAS_PAGE_MINIGAMES,
    EXTRAS_PAGE_CREDITS
} ExtrasPage;

typedef struct FinishingState {
    FinishingMode mode;
    ExtrasPage extras_page;
    int extras_selection;
    int extras_index;
    bool phone_active;
    bool phone_muted;
    int phone_night;
    uint32_t phone_frames;
    uint32_t phone_duration;
    uint32_t title_notice_frames;
    int minigame_chapter;
    int minigame_x;
    int minigame_y;
    int minigame_waypoint;
    uint32_t minigame_event_frames;
    bool minigame_replay;
} FinishingState;

static FinishingState sFinishing;
static void finishing_draw_phone_overlay(void);
