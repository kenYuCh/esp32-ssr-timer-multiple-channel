#ifndef SWITCH_TIMER_FUNC_H
#define SWITCH_TIMER_FUNC_H

#include <stdint.h>
void switch_output_init();
void swv_timer_handle(uint32_t now) ;
uint32_t get_calculate_total_seconds(uint8_t hours, uint8_t mininutes);

uint8_t get_channel_running_period(uint8_t channel_id);
#endif 