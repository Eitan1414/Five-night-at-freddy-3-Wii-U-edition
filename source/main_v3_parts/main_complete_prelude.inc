typedef struct CompleteCheatState {
    bool fast_nights;
    bool radar;
    bool aggressive;
    bool no_errors;
    int selection;
    uint32_t locked_notice_frames;
} CompleteCheatState;

typedef struct AchievementRuntime {
    bool active;
    AchievementId active_id;
    uint32_t popup_frames;
    AchievementId queue[4];
    uint8_t queue_count;
    int menu_selection;
} AchievementRuntime;

struct Game;

static CompleteCheatState sCompleteCheats;
static AchievementRuntime sAchievements;

static bool achievement_unlock(struct Game *game, AchievementId id);
static void achievement_update(struct Game *game);
static void achievement_draw_overlay(void);
static void achievement_draw_menu(void);
static bool achievement_blocks_input(void);
static void complete_graphics_present(uint32_t target);