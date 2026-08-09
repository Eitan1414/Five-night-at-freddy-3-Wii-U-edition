/* Authentic PC Phantom Puppet attack behaviour.
 * The camera appearance still comes from the restored CAM 08 frame pack. */

extern const JumpscareSequence gPhantomPuppetPcAnimation;

static uint32_t sPuppetAttackTotalFrames = 0u;

static uint32_t puppet_pc_attack_duration_frames(const Game *game)
{
    if (game == NULL) return 10u * 60u;
    if (game->night >= 6)
        return (sCompleteCheats.aggressive ? 17u : 15u) * 60u;
    if (game->night == 5) return 13u * 60u;
    return 10u * 60u;
}

static const TextureRle *puppet_pc_sequence_frame(
    const JumpscareSequence *sequence,
    uint32_t remaining_frames)
{
    if (sequence == NULL || sequence->frames == NULL ||
        sequence->frame_count == 0u || sequence->ticks_per_frame == 0u) {
        return NULL;
    }

    uint32_t elapsed = 0u;
    if (sPuppetAttackTotalFrames > remaining_frames)
        elapsed = sPuppetAttackTotalFrames - remaining_frames;
    const uint32_t frame =
        (elapsed / sequence->ticks_per_frame) % sequence->frame_count;
    return sequence->frames[frame];
}

static void process_phantom_event(Game *game, PhantomEvent event)
{
    fnaf3_legacy_process_phantom_event(game, event);

    if ((event.flags & PHANTOM_EVENT_MASK_STARTED) != 0u) {
        const uint32_t duration = puppet_pc_attack_duration_frames(game);
        game->phantoms.puppet_office_frames = duration;
        sPuppetAttackTotalFrames = duration;

        /* Puppet causes the ventilation failure the instant she reaches the
         * office. The existing mask cue continues looping alongside the alarm
         * until MASK_ENDED stops it. */
        set_ventilation_failure(game);
        game->dirty = true;
    }

    if ((event.flags & PHANTOM_EVENT_MASK_ENDED) != 0u)
        sPuppetAttackTotalFrames = 0u;
}
