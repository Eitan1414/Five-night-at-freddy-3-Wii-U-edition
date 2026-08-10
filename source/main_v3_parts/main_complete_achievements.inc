#define ACHIEVEMENT_POPUP_FRAMES 180u
#define ACHIEVEMENT_POPUP_SLIDE_FRAMES 12u
#define UTINE_FINAL_HOLD_FRAMES 60u

static void achievement_begin_popup(AchievementId id)
{
    sAchievements.active = true;
    sAchievements.active_id = id;
    sAchievements.popup_frames = 0u;
    if (id == ACH_UTINE) {
        audio_stop(AUDIO_CUE_ACHIEVEMENT);
        audio_restart(AUDIO_CUE_UTINE, 1.0f, false);
    } else {
        audio_restart(AUDIO_CUE_ACHIEVEMENT, 0.96f, false);
    }
}

static void achievement_enqueue_popup(AchievementId id)
{
    if (!sAchievements.active) {
        achievement_begin_popup(id);
        return;
    }
    if (sAchievements.queue_count >= 4u) return;
    sAchievements.queue[sAchievements.queue_count++] = id;
}

static bool achievement_unlock(Game *game, AchievementId id)
{
    if (!progress_save_unlock_achievement(id)) return false;

    achievement_enqueue_popup(id);
    if (id != ACH_UTINE &&
        progress_save_normal_achievement_count() >= ACH_NORMAL_COUNT &&
        !progress_save_utine_unlocked()) {
        if (progress_save_unlock_achievement(ACH_UTINE))
            achievement_enqueue_popup(ACH_UTINE);
    }
    if (game != NULL) game->dirty = true;
    return true;
}

static uint32_t achievement_popup_duration(void)
{
    if (!sAchievements.active) return 0u;
    if (sAchievements.active_id == ACH_UTINE) {
        return gUtineAnimationFrameCount * gUtineAnimationTicksPerFrame +
               UTINE_FINAL_HOLD_FRAMES;
    }
    return ACHIEVEMENT_POPUP_FRAMES;
}

static void achievement_update(Game *game)
{
    if (!sAchievements.active) return;
    ++sAchievements.popup_frames;
    if (game != NULL) game->dirty = true;

    if (sAchievements.popup_frames < achievement_popup_duration()) return;

    sAchievements.active = false;
    sAchievements.popup_frames = 0u;
    if (sAchievements.queue_count > 0u) {
        const AchievementId next = sAchievements.queue[0];
        for (uint8_t index = 1u; index < sAchievements.queue_count; ++index)
            sAchievements.queue[index - 1u] = sAchievements.queue[index];
        --sAchievements.queue_count;
        achievement_begin_popup(next);
        if (game != NULL) game->dirty = true;
    }
}

static bool achievement_blocks_input(void)
{
    return sAchievements.active && sAchievements.active_id == ACH_UTINE;
}

static uint32_t achievement_normal_popup_slide(void)
{
    const uint32_t frame = sAchievements.popup_frames;
    if (frame < ACHIEVEMENT_POPUP_SLIDE_FRAMES)
        return ACHIEVEMENT_POPUP_SLIDE_FRAMES - frame;

    const uint32_t outro_start =
        ACHIEVEMENT_POPUP_FRAMES - ACHIEVEMENT_POPUP_SLIDE_FRAMES;
    if (frame >= outro_start)
        return frame - outro_start;

    return 0u;
}

static void achievement_draw_normal_popup(AchievementId id)
{
    if (id < ACH_NIGHT_1 || id > ACH_SPRINGTRAP_JUMPSCARE) return;

    const uint32_t slide = achievement_normal_popup_slide();
    /* 470 + 12 * 32 = 854: the first frame starts completely off-screen. */
    const int x = 470 + (int) slide * 32;
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     x, 22, 360, 90,
                     gAchievementUnlockedTextures[(int) id]);
}

static void achievement_draw_utine_popup(void)
{
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     43, 144, 768, 192,
                     &gUtineFrameTexture);

    uint32_t frame = sAchievements.popup_frames /
        gUtineAnimationTicksPerFrame;
    if (frame >= gUtineAnimationFrameCount)
        frame = gUtineAnimationFrameCount - 1u;
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     49, 148, 185, 185,
                     gUtineAnimationFrames[frame]);
}

static void achievement_draw_overlay(void)
{
    if (!sAchievements.active) return;
    if (sAchievements.active_id == ACH_UTINE)
        achievement_draw_utine_popup();
    else
        achievement_draw_normal_popup(sAchievements.active_id);
}

static void achievement_draw_menu(void)
{
    /* Keep the normal Extras background/header block; only the lettering uses
       the title-menu font style supplied by the user. */
    finishing_draw_header("");
    graphics_draw_fnaf3_menu_text(GRAPHICS_TARGET_BOTH,
                                  35, 14, 2,
                                  "EXTRAS / ACHIEVEMENTS",
                                  COLOUR_GREEN);

    /* Utine stays pinned at the top of this category at all times. */
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     43, 55, 768, 192,
                     &gUtineFrameTexture);
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     49, 58, 185, 185,
                     &gUtineTrophyStaticTexture);

    char progress[16];
    snprintf(progress, sizeof(progress), "%d/9",
             progress_save_normal_achievement_count());
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       685, 205, 91, 30,
                       GRAPHICS_RGB(0, 0, 0));
    graphics_draw_fnaf3_menu_text(GRAPHICS_TARGET_BOTH,
                                  704, 212, 1, progress,
                                  progress_save_utine_unlocked()
                                      ? COLOUR_GREEN : COLOUR_WHITE);

    int selection = sAchievements.menu_selection;
    if (selection < 0) selection = 0;
    if (selection >= ACH_NORMAL_COUNT) selection = ACH_NORMAL_COUNT - 1;

    /* Show achievements in stable pairs.  The final page becomes 8+9 rather
       than leaving achievement 9 alone at the top of an otherwise empty area. */
    int first = (selection / 2) * 2;
    if (first >= ACH_NORMAL_COUNT - 1)
        first = ACH_NORMAL_COUNT - 2;

    for (int slot = 0; slot < 2; ++slot) {
        const int index = first + slot;
        if (index >= ACH_NORMAL_COUNT) break;
        const int y = 267 + slot * 96;
        const bool selected = index == selection;
        const TextureRle *texture = progress_save_achievement_unlocked(
            (AchievementId) index)
            ? gAchievementUnlockedTextures[index]
            : gAchievementLockedTextures[index];
        if (selected) {
            graphics_draw_frame(GRAPHICS_TARGET_BOTH,
                                241, y - 5, 372, 100, 3,
                                COLOUR_GREEN);
        }
        texture_draw_rle(GRAPHICS_TARGET_BOTH,
                         247, y, 360, 90, texture);
    }

    char position[16];
    snprintf(position, sizeof(position), "%d/9", selection + 1);
    graphics_draw_fnaf3_menu_text(GRAPHICS_TARGET_BOTH,
                                  64, 438, 1, position,
                                  GRAPHICS_RGB(121, 161, 117));
    graphics_draw_fnaf3_menu_text(GRAPHICS_TARGET_BOTH,
                                  256, 438, 1,
                                  "UP/DOWN: SELECT   B: BACK",
                                  GRAPHICS_RGB(121, 161, 117));
    graphics_present(GRAPHICS_TARGET_BOTH);
}
