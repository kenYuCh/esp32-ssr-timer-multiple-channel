#include "device_config.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt5_client.h"
#include "app_wifi.h"
#include "locks.h"
#include "setting.h"
#include "status.h"
#include "mqtt_callback.h"
#include "mqtt.h"
#include "switch_model.h"
#include "system.h"
#include <string.h>
static const char *TAG = "MQTT";
// MQTT 客戶端初始化
esp_mqtt_client_handle_t mqtt_client = NULL;
static ConnectStatus mqtt_status = DISCONNECTED;

char *subscribe_topics[MQTT_SUBSCRIBE_LEN] = {""};
void set_topics(uint8_t index, const char *sub_topic) {
    if (index >= MQTT_SUBSCRIBE_LEN) return;
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s", GATEWAY_ID,sub_topic);
    subscribe_topics[index] = strdup(topic); 
};

const int qos[3] = {
    0,  
    1,  
    2   
};

ConnectStatus mqtt_get_status() {
    return mqtt_status;
};

static void mqtt5_event_handler(void *handler_args, 
    esp_event_base_t base, 
    int32_t event_id, 
    void *event_data) {

    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;

    printf("MQTT event received, %d bytes remaining available\n", uxTaskGetStackHighWaterMark(NULL));

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED: {
                mqtt_status = CONNECTED;
                ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
                for (uint8_t i = 0; i < MQTT_SUBSCRIBE_LEN; i++) {
                    if (subscribe_topics[i] != NULL) {
                        mqtt_subscribe(subscribe_topics[i]);
                    } else {
                        printf("Topic %d is NULL, skipping subscription.\n", i);
                    }
                };
            break;
            }
        case MQTT_EVENT_DISCONNECTED:
            mqtt_status = DISCONNECTED;
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_restart();
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA: {
                ESP_LOGI(TAG, "MQTT_EVENT_DATA");
                MqttData mqttData;
                mqttData.topicLength = event->topic_len;
                mqttData.topic = (char *)malloc(mqttData.topicLength + 1);
                if (mqttData.topic != NULL) {
                    memcpy(mqttData.topic, event->topic, mqttData.topicLength);
                    mqttData.topic[mqttData.topicLength] = '\0';  // null-terminate
                };
                mqttData.dataLength = event->data_len;
                mqttData.data = (char *)malloc(mqttData.dataLength + 1);
                if (mqttData.data != NULL) {
                    memcpy(mqttData.data, event->data, mqttData.dataLength);
                    mqttData.data[mqttData.dataLength] = '\0';  // null-terminate
                };

                mqtt_data_callback_t cb = mqtt_get_data_callback();
                if (cb) cb(&mqttData);
                free(mqttData.topic);
                free(mqttData.data);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
        }
}

esp_mqtt_client_handle_t mqtt_get_client_handle_t() {
    return mqtt_client;
}
void  mqtt_start() {
    esp_mqtt_client_start(mqtt_client);
}
void mqtt_stop() {
    mqtt_status = DISCONNECTED;
    esp_mqtt_client_stop(mqtt_client);
    esp_mqtt_client_destroy(mqtt_client);
    // mqtt_client = NULL;
};
void mqtt_restart() {
    if(mqtt_status == CONNECTED) {
        mqtt_stop();
    };
    vTaskDelay(pdMS_TO_TICKS(1000)); // 等待 1 秒后再重启
    mqtt_start();
}

uint8_t mqtt_publish(char *topic, char *data) {
    int msgId = esp_mqtt_client_publish(mqtt_client, topic, data, 0, 0, 0);
    return msgId == -1;
};
uint8_t mqtt_publish2(char *topic, char *data, uint8_t qos, uint8_t remain) {
    int msgId = esp_mqtt_client_publish(mqtt_client, topic, data, 0, qos, remain);
    return msgId == -1;
};
uint8_t mqtt_subscribe(char *topic) {
    int msgId = esp_mqtt_client_subscribe(mqtt_client, topic, 0);
    return msgId == -1;
};

void mqtt_app_start() {
    Settings settings;
    settings_load(&settings);
    set_topics(0,MQTT_SUBSCRIBE_1);
    set_topics(1,MQTT_SUBSCRIBE_2);
    set_topics(2,MQTT_SUBSCRIBE_3);
    set_topics(3,MQTT_SUBSCRIBE_4);
    esp_mqtt_client_config_t mqtt5_cfg = {
        .broker.address.uri = settings.server.url,
        .credentials.username = settings.server.username,
        .credentials.authentication.password = settings.server.password,
        .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
        .network.disable_auto_reconnect = false, // is or reconnect
        .session.disable_clean_session = false,
        .network.reconnect_timeout_ms = 20000,
        .network.timeout_ms = 10000,
        .session.keepalive = 60, // Send a PING message every 120 seconds to keep the connection active
        .task.stack_size = 32768,
        .buffer.size = 8192
    };
    // init mqtt 
    mqtt_client = esp_mqtt_client_init(&mqtt5_cfg);
    if (mqtt_client == NULL) {
        printf("Failed to initialize MQTT client\n");
        return; // 处理初始化失败
    }

    // registor MQTT events.
    esp_mqtt_client_register_event(mqtt_client,
        ESP_EVENT_ANY_ID,
        mqtt5_event_handler,
        mqtt_client);
    esp_mqtt_client_start(mqtt_client);
};
