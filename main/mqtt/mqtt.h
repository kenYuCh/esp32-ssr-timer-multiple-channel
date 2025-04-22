#ifndef MQTT_H
#define MQTT_H
#include <stdint.h>
#include "mqtt_client.h"
#include "setting.h"
#include "status.h"

typedef struct {
    char *topic;
    uint32_t topicLength;
    char *data;
    uint32_t dataLength;
} MqttData;
void mqtt_app_start();
void mqtt_start();
void mqtt_stop();
void mqtt_restart();
uint8_t mqtt_subscribe(char *topic);
uint8_t mqtt_publish(char *topic, char *data);
uint8_t mqtt_publish2(char *topic, char *data, uint8_t qos, uint8_t remain);
esp_mqtt_client_handle_t mqtt_get_client_handle_t() ;
ConnectStatus mqtt_get_status();
#endif // !