static const TextureRle *sequence_ping_pong_frame(
    const JumpscareSequence *sequence,
    uint32_t timer)
{
    if (sequence == NULL || sequence->frames == NULL ||
        sequence->frame_count == 0u || sequence->ticks_per_frame == 0u) {
        return NULL;
    }
    if (sequence->frame_count == 1u) return sequence->frames[0];

    const uint32_t span = sequence->frame_count * 2u - 2u;
    uint32_t frame = (timer / sequence->ticks_per_frame) % span;
    if (frame >= sequence->frame_count) frame = span - frame;
    return sequence->frames[frame];
}

static int phantom_bob(uint32_t timer, uint32_t period, int distance)
{
    if (period < 2u || distance <= 0) return 0;
    const uint32_t phase = timer % period;
    const uint32_t half = period / 2u;
    const uint32_t ramp = phase < half ? phase : period - phase;
    return (int) ((ramp * (uint32_t) distance) / half);
}
