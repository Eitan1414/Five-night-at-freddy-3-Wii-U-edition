#include <coreinit/thread.h>
#include <coreinit/time.h>

#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/proc.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int last_sec = -1;
    OSCalendarTime tm;

    WHBProcInit();
    WHBLogConsoleInit();
    WHBLogPrintf("FNAF 3 Wii U - WUP DIAGNOSTIC");
    WHBLogPrintf("Hello World! RPX main() reached successfully.");
    WHBLogPrintf("If you can read this, the installed WUP/channel launches correctly.");

    while (WHBProcIsRunning()) {
        OSTicksToCalendarTime(OSGetTime(), &tm);
        if (tm.tm_sec != last_sec) {
            WHBLogPrintf("Alive: %02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
            last_sec = tm.tm_sec;
        }
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(100));
    }

    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}
