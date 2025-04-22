#include "switch_timer_func.h"
#include "esp_log.h"
#include "gpio.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"
#include "switch_model.h"
#include <stdint.h>
#include <time.h>
#include "stdbool.h"
#include "ticks.h"

static char *TAG_DATA = "DATA";
#define IDLE 0
#define PERIOD_1 1
#define PERIOD_2 2
#define PERIOD_3 3

static uint8_t current_running_period[5] = {IDLE,IDLE,IDLE,IDLE,IDLE};

uint8_t get_channel_running_period(uint8_t channel_id) {
    return current_running_period[channel_id];
};


uint32_t get_calculate_total_seconds(uint8_t hours, uint8_t mininutes) {
    uint32_t result = (hours * 3600UL) + (mininutes * 60UL);
    return result;
}

uint8_t swv_schedule(uint32_t start, uint32_t stop, uint16_t on, uint16_t interval, uint16_t count,
    uint32_t now, uint16_t *debug) {
    if (start == 0) start = 1;
    if (stop == 0) stop = 86400;
    int32_t elapsed_seconds = now - start;
    if(elapsed_seconds < 0 || (stop > start && now >= stop)){
        *debug = 0x0000;
        return 0;
    };
    if (on == 0 && interval == 0) {
        if (elapsed_seconds >= 0 && now < stop && count >= 1) {
            *debug = 0xFFFF;
            return 1;
        } else {
            *debug = 0x0000;
            return 0;
        }
    } else {
        if(count == 0) count = UINT16_MAX;
    };
    int32_t cycle_seconds = on + interval;
    int32_t cycles_passed = elapsed_seconds / cycle_seconds;
    if(cycles_passed >= count) {
        *debug = 0xFFFF;
        return 0;
    }
    int32_t time_in_cycle = elapsed_seconds % cycle_seconds;
    *debug = time_in_cycle;
    if (time_in_cycle < on) {
        return 1;
    } else {
        return 0;
    };
}

void warning_timer_printf(char *content) {
    static uint32_t prev_ticks = 0;
    uint32_t now = get_ticks();
    if(calculate_diff(now, prev_ticks) > 5000) {
        ESP_LOGW(TAG_DATA, "%s", content);  
        prev_ticks = now;
    };
};

uint8_t swv_check_period(SwitchValve *swv, uint8_t channel_id, uint32_t now) {
    uint8_t active_period = IDLE;  // default IDLE
    for (uint8_t i = 0; i < PERIOD_NUM; i++) {
        if (swv->valve[channel_id].period_params[i].period_enable == DISABLE) continue; // if disabled, then continue.
        uint32_t start_sec = get_calculate_total_seconds(swv->valve[channel_id].period_params[i].start_hour, swv->valve[channel_id].period_params[i].start_minute);
        uint32_t stop_sec  = get_calculate_total_seconds(swv->valve[channel_id].period_params[i].stop_hour,swv->valve[channel_id].period_params[i].stop_minute);
        // if (start_sec == 0 && stop_sec == 0) continue;
        if (now >= start_sec && now < stop_sec) {
            if (active_period == IDLE) {
                active_period = i + 1;
            } else {
                char buffer[128] = {0};
                snprintf(buffer, sizeof(buffer), "Warning: The running time period overlaps with other time periods, please confirm: channel-%d period-%d!\n", channel_id+1, i+1);
                warning_timer_printf(buffer);
                return IDLE;  
            };
        };
    };
    return active_period; 
}

void swv_schedule_handle(uint32_t now_secs, uint8_t channel_id) {
    SwitchValve *swv = swv_get_instance();
    uint8_t check_period  = 0;
    check_period = swv_check_period(swv, channel_id, now_secs);
    current_running_period[channel_id] = check_period;
    uint16_t debug = 0;
    switch (check_period) {
        case IDLE: {
            printf("Channel-%d, Schedule Running IDLE.\n", channel_id+1);
            swv->valve[channel_id].output_state = 0;
        };
        break;
        case PERIOD_1: {
            uint8_t period_id = check_period - 1;
            printf("Channel-%d, Schedule Running PERIOD_1: %d.\n", channel_id+1, period_id);
            uint32_t start    = get_calculate_total_seconds(swv->valve[channel_id].period_params[period_id].start_hour, swv->valve[channel_id].period_params[period_id].start_minute);
            uint32_t stop     = get_calculate_total_seconds(swv->valve[channel_id].period_params[period_id].stop_hour, swv->valve[channel_id].period_params[period_id].stop_minute);
            uint32_t on       = swv->valve[channel_id].period_params[period_id].on_timer;
            uint32_t interval = swv->valve[channel_id].period_params[period_id].interval_timer;
            uint32_t count    = swv->valve[channel_id].period_params[period_id].count;
            swv->valve[channel_id].output_state = swv_schedule(start, stop, on, interval, count, now_secs,&debug);
        };
        break;
        case PERIOD_2: {
            uint8_t period_id = check_period - 1;
            printf("Channel-%d, Schedule Running PERIOD_2: %d.\n", channel_id+1, period_id);
            uint32_t start    = get_calculate_total_seconds(swv->valve[channel_id].period_params[period_id].start_hour, swv->valve[channel_id].period_params[period_id].start_minute);
            uint32_t stop     = get_calculate_total_seconds(swv->valve[channel_id].period_params[period_id].stop_hour, swv->valve[channel_id].period_params[period_id].stop_minute);
            uint32_t on       = swv->valve[channel_id].period_params[period_id].on_timer;
            uint32_t interval = swv->valve[channel_id].period_params[period_id].interval_timer;
            uint32_t count    = swv->valve[channel_id].period_params[period_id].count;
            swv->valve[channel_id].output_state = swv_schedule(start, stop, on, interval, count, now_secs,&debug);
        };
        break;
        case PERIOD_3: {
            uint8_t period_id = check_period - 1;
            printf("Channel-%d, Schedule Running PERIOD_3: %d.\n", channel_id+1, period_id);
            uint32_t start    = get_calculate_total_seconds(swv->valve[channel_id].period_params[period_id].start_hour, swv->valve[channel_id].period_params[period_id].start_minute);
            uint32_t stop     = get_calculate_total_seconds(swv->valve[channel_id].period_params[period_id].stop_hour, swv->valve[channel_id].period_params[period_id].stop_minute);
            uint32_t on       = swv->valve[channel_id].period_params[period_id].on_timer;
            uint32_t interval = swv->valve[channel_id].period_params[period_id].interval_timer;
            uint32_t count    = swv->valve[channel_id].period_params[period_id].count;
            swv->valve[channel_id].output_state = swv_schedule(start, stop, on, interval, count, now_secs,&debug);
        };
        break;
        default:
            break;
    };
}


void swv_timer_handle(uint32_t now) {
    SwitchValve *swv = swv_get_instance();
    static uint32_t prev_ticks = 0;
    static uint32_t prev_secs = 0;
    uint32_t sync_secs = get_calculate_total_seconds(swv->current_hour, swv->current_minute); // now time seconds
    if(sync_secs != prev_secs) {
        prev_secs = sync_secs;
        prev_ticks = now;
    };
    uint32_t unsync_secs = (now - prev_ticks) / 1000;
    uint32_t now_secs = sync_secs + unsync_secs; 
    //
    for (uint8_t channel_id = 0; channel_id < SWITCH_VALVE_NUM; channel_id++) {
        if(swv->valve[channel_id].mode == 0) {          // OFF
            swv->valve[channel_id].output_state = 0;
        } else if(swv->valve[channel_id].mode == 1) {   // ON
            swv->valve[channel_id].output_state = 1;
        } else if(swv->valve[channel_id].mode == 2) {   // SCHEDULE
            swv_schedule_handle(now_secs, channel_id);
        } else if(swv->valve[channel_id].mode == 3) {   // LINKAGE
            if(swv->valve[channel_id].trigger) { // trigger_ON
                swv_schedule_handle(now_secs, channel_id);
            } else { // trigger_OFF
                swv->valve[channel_id].output_state = 0;
            }
        } else if(swv->valve[channel_id].mode == 4) {   // Forward and reverse motor. 

        } else if(swv->valve[channel_id].mode == 5) {   // Forward and reverse motor. ON

        } else if(swv->valve[channel_id].mode == 6) {   // Forward and reverse motor. OFF

        } else if(swv->valve[channel_id].mode == 7) {   // Forward and reverse motor. 

        }
    }
};



















