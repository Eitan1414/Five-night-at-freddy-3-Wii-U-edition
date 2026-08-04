#include "platform/frame_clock.h"

#include <coreinit/thread.h>
#include <coreinit/time.h>

static uint64_t sFrameNumber = 0u;

void frame_clock_reset(void)
{
    sFrameNumber = 0u;
}

void frame_clock_wait_next(void)
{
    OSSleepTicks(OSMillisecondsToTicks(16));
    ++sFrameNumber;
}

uint64_t frame_clock_frame_number(void)
{
    return sFrameNumber;
}
