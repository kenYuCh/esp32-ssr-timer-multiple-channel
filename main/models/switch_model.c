#include "switch_model.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio.h"
#include "hal/gpio_types.h"
#include "locks.h"
#include "mqtt.h"
#include "mqtt_callback.h"
#include "my_nvs.h"
#include "setting.h"
#include "soc/gpio_num.h"
#include "switch_timer_func.h"
#include "ticks.h"
#include "system_monitor.h"

static SwitchValve switch_valve = {0};
static uint8_t switch_pin_num[SWITCH_VALVE_NUM] = {GPIO_NUM_4,GPIO_NUM_5,GPIO_NUM_6,GPIO_NUM_7,GPIO_NUM_15};


void swv_init(Settings *settings) {
    switch_output_init();
    memset(&switch_valve, 0, sizeof(SwitchValve));
    for (uint8_t channel = 0; channel < SWITCH_VALVE_NUM; channel++) {
        for (uint8_t period_id = 0; period_id < PERIOD_NUM; period_id++){ 
            swv_get_nvs(&switch_valve, channel,period_id);
        }
    };
    strcpy(switch_valve.sn, settings->device_serial);
};

SwitchValve *swv_get_instance() {
    return &switch_valve;
};

void switch_output_init() {
    for (uint8_t i = 0; i < SWITCH_VALVE_NUM; i++) {
        gpio_init(switch_pin_num[i], GPIO_MODE_OUTPUT);
    };
}

// ------------------------------------------------------------------------
void swv_set_nvs(SwitchValve *swv, uint8_t channel, uint8_t period_id) {
    uint8_t save_mode = 0;
    memcpy(&save_mode, &swv->valve[channel].mode, sizeof(uint8_t));
    char key_mode[16] = {0};
    sprintf(key_mode, "swv-ch-mode-%u", channel);
    mynvs_write("gateway", key_mode, &save_mode, sizeof(uint8_t));

    RelayParamsPeriod2 settings = {0};
    memcpy(&settings, &swv->valve[channel].period_params[period_id], sizeof(RelayParamsPeriod2));
    char key[32] = {0};
    sprintf(key, "swv-ch%u-pd%u", channel, period_id);
    mynvs_write("gateway", key, &settings, sizeof(settings));
    printf("swv-channel%d-period%d save. \r\n", channel, period_id);
}
void swv_get_nvs(SwitchValve *swv, uint8_t channel, uint8_t period_id) {
    uint8_t get_mode = 0;
    char key_mode[16] = {0};
    sprintf(key_mode, "swv-ch-mode-%u", channel);
    mynvs_read("gateway", key_mode, &get_mode, sizeof(uint8_t));
    if (channel < SWITCH_VALVE_NUM) {
        swv->valve[channel].mode = get_mode;
    };

    RelayParamsPeriod2 settings = {0};
    char key[32] = {0};
    sprintf(key, "swv-ch%u-pd%u", channel, period_id);
    mynvs_read("gateway", key, &settings, sizeof(settings));
    if (channel < SWITCH_VALVE_NUM) {
        swv->valve[channel].period_params[period_id] = settings;
        printf("swv-channel load. \r\n");
    }
    
};
// ------------------------------------------------------------------------
void swv_set_start_time(RelayChannel *relay_channel, uint8_t hour, uint8_t minute, uint8_t period) {
    relay_channel->period_params[period].start_hour = hour;
    relay_channel->period_params[period].start_minute = minute;
};
void swv_set_stop_time(RelayChannel *relay_channel,  uint8_t hour, uint8_t minute, uint8_t period) {
    relay_channel->period_params[period].stop_hour = hour;
    relay_channel->period_params[period].stop_minute = minute;
};
void swv_set_on_time(RelayChannel *relay_channel, uint32_t on_time, uint8_t period) {
    relay_channel->period_params[period].on_timer = on_time;
};
void swv_set_interval_time(RelayChannel *relay_channel, uint32_t interval_time, uint8_t period) {
    relay_channel->period_params[period].interval_timer = interval_time;
};
void swv_set_count(RelayChannel *relay_channel, uint16_t count, uint8_t period) {
    relay_channel->period_params[period].count = count;
};
void swv_set_mode(RelayChannel *relay_channel, uint8_t mode, uint8_t period) {
    relay_channel->mode = mode;
};
void swv_set_trigger(RelayChannel *relay_channel, uint8_t trigger, uint8_t period) {
    relay_channel->trigger = trigger;
};
void swv_set_period_enable(RelayChannel *relay_channel, uint8_t enable, uint8_t period) {
    relay_channel->period_params[period].period_enable = enable;
};
// ------------------------------------------------------------------------

void swv_update(SwitchValve *swv) {
    CurrentTime_t *currentTime = get_current_time_instance();
    get_current_time(currentTime);
    swv->current_hour = currentTime->hour;
    swv->current_minute = currentTime->minute;
    // ---------
    static uint32_t prev_ticks = 0;
    uint32_t now = get_ticks();
    if(calculate_diff(now, prev_ticks) >= 1000) {
        for (uint8_t i = 0; i < SWITCH_VALVE_NUM ; i++) {
            if(swv->valve[i].output_state == 0) {
                gpio_set_off(switch_pin_num[i]); 
            } else {
                gpio_set_on(switch_pin_num[i]); 
            };
        };
        prev_ticks = now;
    }
};

// ------------------------------------------------------------------------
const char *channel_names[SWITCH_VALVE_NUM] = {
    "ch1_output_state", 
    "ch2_output_state", 
    "ch3_output_state",
    "ch4_output_state", 
    "ch5_output_state"
};

void add_cJSON_Main_Object(SwitchValve *swv, cJSON *root) {
    char random_int[7] = {0};
    generate_random_six_digit(random_int);
    cJSON_AddStringToObject(root, "id", random_int);
    cJSON_AddStringToObject(root, "sn", swv->sn);
    cJSON_AddStringToObject(root, "label", "switch valve");
    cJSON_AddStringToObject(root, "label_en", "multiple channels");
    cJSON_AddStringToObject(root, "ctrl", "N/A");
};

void swv_publish_msg_event(SwitchValve *swv, const char *sub_topic) {
    // build JSON object
    cJSON *root = cJSON_CreateObject();
    add_cJSON_Main_Object(swv, root);
    cJSON *params = cJSON_CreateArray();
    for (uint8_t i = 0; i < SWITCH_VALVE_NUM; i++) { 
        cJSON *param = cJSON_CreateObject();
        cJSON_AddStringToObject(param, "name", channel_names[i]);
        char output_state[SWITCH_VALVE_NUM] = {0};
        sprintf(output_state, "%d", swv->valve[i].output_state);
        cJSON_AddStringToObject(param, "value", output_state);
        cJSON_AddItemToArray(params, param);
    }
    cJSON_AddItemToObject(root, "params", params);
    // convert JSON to string
    char *json_string = cJSON_Print(root);
    // Publish to mqtt
    char topic[64] = {0};
    snprintf(topic, sizeof(topic), "%s/%s", GATEWAY_ID, sub_topic);
    mqtt_publish2(topic, json_string, 2,1);
    // release memory
    cJSON_Delete(root);
    free(json_string);
};

void swv_publish_msg_output_state(SwitchValve *swv, const char *sub_topic) {
    // build JSON object
    cJSON *root = cJSON_CreateObject();
    add_cJSON_Main_Object(swv, root);
    cJSON *params = cJSON_CreateArray();
    for (uint8_t i = 0; i < SWITCH_VALVE_NUM; i++) { 
        cJSON *param = cJSON_CreateObject();
        cJSON_AddStringToObject(param, "name", channel_names[i]);
        char output_state[SWITCH_VALVE_NUM] = {0};
        sprintf(output_state, "%d", swv->valve[i].output_state);
        cJSON_AddStringToObject(param, "value", output_state);
        cJSON_AddItemToArray(params, param);
    }
    cJSON_AddItemToObject(root, "params", params);
    // convert JSON to string
    char *json_string = cJSON_Print(root);
    // Publish to mqtt
    char topic[64] = {0};
    snprintf(topic, sizeof(topic), "%s/%s", GATEWAY_ID, sub_topic);
    mqtt_publish2(topic, json_string, 2,1);
    // release memory
    cJSON_Delete(root);
    free(json_string);
}
// ------------------------------------------------------------------------

uint8_t get_time_period_index(uint8_t period) {
    switch(period) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 3:
            return 2;
        default:
            return 255;
    };
}
uint8_t get_time_channel_index(uint8_t channel) {
    switch(channel) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 3:
            return 2;
        case 4:
            return 3;
        case 5:
            return 4;
        default:
            return 255;
    };
}

#define ENABLE_CHECK_TIME_OVERLAY 1
uint8_t swv_operate(SwitchValve *swv, cJSON *buffer) {
    char  *id      = cJSON_GetObjectItem(buffer, "id")->valuestring;
    char  *serial     = cJSON_GetObjectItem(buffer, "sn")->valuestring;
    char  *control = cJSON_GetObjectItem(buffer, "ctrl")->valuestring;

    
    printf("[OPERATION] id: %s\n", id);
    printf("[OPERATION] serial: %s\n", serial);
    printf("[OPERATION] control: %s\n", control);

    int ch_id = 0;
    int pd_idx = 0;
    char extra;
    if(sscanf(control, "%d-%d%c", &ch_id, &pd_idx, &extra) != 2) return 1;
    printf("Control Format OK: ch_id:%d, period_idx:%d\n", ch_id, pd_idx);
    uint8_t channel_idx = get_time_channel_index(ch_id);
    uint8_t period_idx = get_time_period_index(pd_idx);
    if(channel_idx == 255) return 2; // check overflow
    if(period_idx == 255) return 3; // check overflow

    cJSON *params  = cJSON_GetObjectItem(buffer, "params");
    uint16_t paramsCount = cJSON_GetArraySize(params);

    // check time params
    for (int i = 0; i < paramsCount; i++) {
        cJSON *param = cJSON_GetArrayItem(params, i);
        char *name = cJSON_GetObjectItem(param, "name")->valuestring;
        char *value = cJSON_GetObjectItem(param, "value")->valuestring;
        if (strcmp(name, PARAMS_START_TIME) == 0) {
            uint8_t start_time[2];
            time_text_to_number(value, start_time);
            swv_set_start_time(&swv->valve[channel_idx], start_time[0], start_time[1], period_idx);
        } else if (strcmp(name, PARAMS_STOP_TIME) == 0) {
            uint8_t stop_time[2] = {0};
            time_text_to_number(value, stop_time);
            swv_set_stop_time(&swv->valve[channel_idx], stop_time[0], stop_time[1], period_idx);
        } else if (strcmp(name, PARAMS_ON_DURATION) == 0) {
            swv_set_on_time(&swv->valve[channel_idx], atoi(value), period_idx);
        } else if (strcmp(name, PARAMS_INTERVAL_DURATION) == 0) {
            swv_set_interval_time(&swv->valve[channel_idx], atoi(value), period_idx);
        } else if (strcmp(name, PARAMS_COUNT) == 0) {
            swv_set_count(&swv->valve[channel_idx], atoi(value), period_idx);
        } else if (strcmp(name, PARAMS_MODE) == 0) {
            swv_set_mode(&swv->valve[channel_idx], atoi(value), period_idx);
        } else if (strcmp(name, PARAMS_TIRGGER) == 0) {
            swv_set_trigger(&swv->valve[channel_idx], atoi(value), period_idx);
        } else if (strcmp(name, PARAMS_PERIOD_ENABLE) == 0) {
            swv_set_period_enable(&swv->valve[channel_idx], atoi(value), period_idx);
        }
    };
    
    swv_set_nvs(swv, channel_idx, period_idx);
    cJSON_Delete(buffer);
    return 0;
};

// ------------------------------------------------------------------------ mqtt publish

void swv_event(SwitchValve *swv, Settings *setting) {
    static uint8_t last_output_state[SWITCH_VALVE_NUM] = {0,0,0,0,0};
    uint8_t changed = 0;
    for (uint8_t i = 0; i < SWITCH_VALVE_NUM; i++){
        if(swv->valve[i].output_state != last_output_state[i]) {
            last_output_state[i] = swv->valve[i].output_state;
            changed = 1;
        };
    };
    if(changed) {
        swv_publish_msg_event(swv, "events");
        changed = 0;
    };
}


void swv_printf(SwitchValve *swv) {
    static uint32_t printf_ticks = 0;
    uint32_t now = get_ticks();
    if(calculate_diff(now, printf_ticks) >= 5000) {
        printf("----- SwitchValve ----- Current Time: %02d:%02d -----\n", swv->current_hour, swv->current_minute);
        for (uint8_t i = 0; i < SWITCH_VALVE_NUM; i++){
            printf("**** channel %d, mode: %d, trigger: %d, output_state: %d ****\n", i+1, swv->valve[i].mode,swv->valve[i].trigger, swv->valve[i].output_state);
            uint8_t running_period = get_channel_running_period(i);  // get current running period.
            for (uint8_t period_id = 0; period_id < PERIOD_NUM; period_id++){
                bool is_running = (running_period == period_id + 1);
                printf("--- Period: %d: StartTime: %02d:%02d, StopTime: %02d:%02d, on_time: %5lu, interval_time: %5lu, count: %5u, period_enable: %d %s\n", 
                    period_id+1,
                    swv->valve[i].period_params[period_id].start_hour, swv->valve[i].period_params[period_id].start_minute,
                    swv->valve[i].period_params[period_id].stop_hour, swv->valve[i].period_params[period_id].stop_minute,
                    swv->valve[i].period_params[period_id].on_timer, swv->valve[i].period_params[period_id].interval_timer,
                    swv->valve[i].period_params[period_id].count,
                    swv->valve[i].period_params[period_id].period_enable,
                    is_running ? " *" : ""
                );
            }
        };
        printf_ticks = now;
    };
}
// ------------------------------------------------------------------------
void model_task(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    Settings settings;
    settings_load(&settings);
    printf("Into data_task \n");
    SwitchValve *swv = swv_get_instance();
    while (1) {
        if(lock_take_mutex()){
            uint32_t now = get_ticks();
            swv_timer_handle(now);
            swv_update(swv); // for wifi mesh (now ,not use)
            // swv_event(swv, &settings);
            swv_printf(swv);
            lock_give_mutex();
        };
        vTaskDelay(pdMS_TO_TICKS(1000));
    };
}