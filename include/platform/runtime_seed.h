#pragma once

#include <stdint.h>

/* Return a non-constant seed for gameplay RNG on Wii U hardware. */
uint32_t runtime_seed_now(void);
