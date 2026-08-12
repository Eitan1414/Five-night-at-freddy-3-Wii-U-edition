#include "platform/runtime_seed.h"

#include <coreinit/time.h>

uint32_t runtime_seed_now(void)
{
    const uint64_t time = (uint64_t)OSGetTime();
    uint32_t seed = (uint32_t)time ^ (uint32_t)(time >> 32u) ^ 0xF3A30A10u;
    /* Avoid a zero state so simple LCG/xorshift users are never accidentally
       initialized into a degenerate sequence. */
    if (seed == 0u) seed = 0x6D2B79F5u;
    return seed;
}
