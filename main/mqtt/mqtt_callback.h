#ifndef MQTT_CALLBACK_H
#define MQTT_CALLBACK_H

// 定義函數指標類型
#include "mqtt.h"
#include <stdint.h>


typedef void (*mqtt_data_callback_t)(MqttData *data);
typedef void (*operations_verify_function)(const char *text);

// --------------------------------------------------
void mqtt_set_data_callback(mqtt_data_callback_t callback);
mqtt_data_callback_t mqtt_get_data_callback();
void mqtt_data_cb_handler(MqttData *mqtt);


// --------------------------------------------------
uint8_t mqtt_operations_mqtt_data(char *resBuffer);
void mqtt_operations_verify_function_set(operations_verify_function function);
#endif // !