#include "platform/frame_clock.h"

#include <coreinit/thread.h>
#include <coreinit/time.h>

static uint64_t sFrameNumber = 0u;
static OSTime sLastFrameTick = 0;
static OSTime sFrameDurationTicks = 0;

static OSTime frame_duration_ticks(void)
{
    /* Keep the game logic at the intended 60 Hz without adding a fixed sleep
     * on top of update/render time. */
    return (OSTime) (OSMillisecondsToTicks(1000) / 60u);
}

void frame_clock_reset(void)
{
    sFrameNumber = 0u;
    sFrameDurationTicks = frame_duration_ticks();
    sLastFrameTick = OSGetSystemTime();
}

void frame_clock_wait_next(void)
{
    if (sFrameDurationTicks <= 0) {
        sFrameDurationTicks = frame_duration_ticks();
    }

    const OSTime now = OSGetSystemTime();
    const OSTime elapsed = now - sLastFrameTick;

    if (elapsed >= 0 && elapsed < sFrameDurationTicks) {
        OSSleepTicks(sFrameDurationTicks - elapsed);
    }

    /* Rebase every frame so one slow frame cannot create a catch-up burst. */
    sLastFrameTick = OSGetSystemTime();
    ++sFrameNumber;
}

uint64_t frame_clock_frame_number(void)
{
    return sFrameNumber;
}
