#ifndef TICKS_H
#define TICKS_H

#include <stdint.h>
#include <sys/time.h>

uint32_t ticks_ms();
uint32_t get_ticks();
uint32_t calculate_diff(uint32_t now, uint32_t prev_ticks);

#endif