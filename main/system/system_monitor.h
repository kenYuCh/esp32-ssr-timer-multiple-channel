#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>

void heap_size_monitor();
void system_monitor_task(void *arg);
void wifi_rssi_monitor();
uint8_t system_mqtt_setting(cJSON *buffer);
void system_mqtt_get_systeminfo(const char *sub_topic);
void generate_random_six_digit(char *buffer);
void check_connected();
#endif