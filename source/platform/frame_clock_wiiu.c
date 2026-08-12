#include "platform/frame_clock.h"

#include <coreinit/thread.h>
#include <coreinit/time.h>

static uint64_t sFrameNumber = 0u;
static OSTime sFrameTicks = 0;
static OSTime sNextFrameTick = 0;

void frame_clock_reset(void)
{
    sFrameNumber = 0u;

    /* Pace to an absolute 60 Hz deadline.  The old implementation always
     * slept 16 ms *after* rendering, so render/update cost was added on top of
     * the frame period and all Clickteam-derived frame counters ran slow on
     * real hardware.  One second worth of OS ticks divided by 60 keeps the
     * simulation cadence tied to wall-clock time while still using OSSleepTicks
     * instead of a busy loop. */
    sFrameTicks = OSMillisecondsToTicks(1000u) / 60;
    if (sFrameTicks <= 0)
        sFrameTicks = 1;
    sNextFrameTick = OSGetTime() + sFrameTicks;
}

void frame_clock_wait_next(void)
{
    const OSTime now = OSGetTime();
    if (now < sNextFrameTick) {
        OSSleepTicks(sNextFrameTick - now);
    } else if (now - sNextFrameTick > sFrameTicks * 4) {
        /* Do not build an unbounded backlog after a long OS stall.  Re-anchor
         * the deadline but never add an extra fixed sleep to an already-late
         * frame. */
        sNextFrameTick = now;
    }

    sNextFrameTick += sFrameTicks;
    ++sFrameNumber;
}

uint64_t frame_clock_frame_number(void)
{
    return sFrameNumber;
}
