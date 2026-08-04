#pragma once

#include <stdint.h>

void frame_clock_reset(void);
void frame_clock_wait_next(void);
uint64_t frame_clock_frame_number(void);
