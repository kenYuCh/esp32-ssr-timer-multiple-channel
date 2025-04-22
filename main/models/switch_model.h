#ifndef SWITCH_MODEL_H
#define SWITCH_MODEL_H

#include "cJSON.h"
#include "setting.h"
#include <stdint.h>
#include <stdio.h>

// MQTT PARAMS_NAMES
#define PARAMS_START_TIME "st_time"
#define PARAMS_STOP_TIME "sp_time"
#define PARAMS_ON_DURATION "on_dur"
#define PARAMS_INTERVAL_DURATION "intv_dur"
#define PARAMS_COUNT "cnt"
#define PARAMS_MODE "mode"
#define PARAMS_TIRGGER "trig"
#define PARAMS_PERIOD_ENABLE "period_en"
// SWITCH INFO
#define SWITCH_VALVE_NUM 5
#define PERIOD_NUM 3
// SWITCH definition
#define SWITCH_OFF 0
#define SWITCH_ON 1
#define DISABLE 0
#define ENABLE 1

typedef struct relay_params_period2 {
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t stop_hour;
    uint8_t stop_minute;
    uint32_t on_timer;            
    uint32_t interval_timer;      
    uint16_t count; 
    uint8_t period_enable;
} RelayParamsPeriod2;

typedef struct relay_channel {
    RelayParamsPeriod2 period_params[3]; // period 1~5
    uint8_t mode;   
    uint8_t trigger;                  
    uint8_t output_state;                    
    uint8_t channel;              
} RelayChannel;

typedef struct valve {
    RelayChannel valve[SWITCH_VALVE_NUM];
    char sn[16];
    uint8_t current_hour;
    uint8_t current_minute;
} SwitchValve;


void swv_init(Settings *settings);
SwitchValve *swv_get_instance();

void model_task(void *arg);

uint8_t swv_operate(SwitchValve *swv, cJSON *buffer);

void swv_update(SwitchValve *swv);

void swv_publish_msg_event(SwitchValve *swv, const char *sub_topic);


void swv_set_nvs(SwitchValve *swv, uint8_t channel, uint8_t period_id);
void swv_get_nvs(SwitchValve *swv, uint8_t channel, uint8_t period_id);

void swv_publish_msg_output_state(SwitchValve *swv, const char *sub_topic);

#endif // !