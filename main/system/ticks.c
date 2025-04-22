#include "ticks.h"

#include <sys/time.h>

#include "esp_timer.h"


uint32_t ticks_ms() {
	uint32_t uptimeMs = esp_timer_get_time() / 1000ul;
	return uptimeMs;
}

uint32_t get_ticks(){
	return ticks_ms();
}

uint32_t calculate_diff(uint32_t now, uint32_t prev_ticks) {
    uint32_t diff;
    if (now >= prev_ticks) {
        diff = now - prev_ticks;
    } else {
        diff = (UINT32_MAX - prev_ticks) + now + 1;
    }
    return diff;
};
