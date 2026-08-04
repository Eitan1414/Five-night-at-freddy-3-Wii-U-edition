#include <stdint.h>

#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <vpad/input.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/proc.h>

static void log_pressed_buttons(uint32_t pressed)
{
    if (pressed & VPAD_BUTTON_A) {
        WHBLogPrintf("[INPUT] A detected - GamePad input works.");
    }

    if (pressed & VPAD_BUTTON_B) {
        WHBLogPrintf("[INPUT] B detected - cancel/back test.");
    }

    if (pressed & VPAD_BUTTON_PLUS) {
        WHBLogPrintf("[PORT] Phase 0: native Wii U bootstrap.");
    }

    if (pressed & VPAD_BUTTON_MINUS) {
        WHBLogPrintf("[PORT] Next: portable input abstraction.");
    }

    if (pressed & VPAD_BUTTON_HOME) {
        WHBLogPrintf("[SYSTEM] HOME detected. Use the normal system flow to leave.");
    }
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    WHBProcInit();
    WHBLogConsoleInit();

    WHBLogPrintf("Five Nights at Freddy's 3 - Wii U Edition");
    WHBLogPrintf("------------------------------------------");
    WHBLogPrintf("Native wut bootstrap is running.");
    WHBLogPrintf("A: input test | B: back test | +: port status");
    WHBLogPrintf("This build contains no game logic or assets yet.");

    uint32_t heartbeat_frames = 0;

    while (WHBProcIsRunning()) {
        VPADStatus status;
        VPADReadError read_error = VPAD_READ_SUCCESS;
        const int32_t samples = VPADRead(VPAD_CHAN_0, &status, 1, &read_error);

        if (samples > 0) {
            log_pressed_buttons(status.trigger);
        } else if (read_error != VPAD_READ_NO_SAMPLES &&
                   read_error != VPAD_READ_SUCCESS) {
            WHBLogPrintf("[INPUT] VPAD read error: %d", (int) read_error);
        }

        heartbeat_frames++;
        if (heartbeat_frames >= 3600) {
            WHBLogPrintf("[SYSTEM] Bootstrap still running.");
            heartbeat_frames = 0;
        }

        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(16));
    }

    WHBLogPrintf("Shutting down cleanly...");
    WHBLogConsoleDraw();
    OSSleepTicks(OSMillisecondsToTicks(250));

    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}
