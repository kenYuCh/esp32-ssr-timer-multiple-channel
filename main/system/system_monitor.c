#include "cJSON.h"
#include "datetime.h"
#include "device_config.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app_wifi.h"
#include <stdio.h>
#include "nvs.h"
#include "status.h"
#include "switch_model.h"
#include "system.h"
#include "mqtt.h"
#include "nvs_flash.h"
#include "ticks.h"
static const char *TAG = "SYSTEM Monitor";
static uint8_t reboot = 3;


// Monitor Head size.
void heap_size_monitor() {
    size_t freeHeap = xPortGetFreeHeapSize();
    size_t minHeap = xPortGetMinimumEverFreeHeapSize();
    ESP_LOGI(TAG, "Free heap: %d bytes, Min ever free: %d bytes", freeHeap, minHeap);
    if (minHeap < SYSTEM_CRITICAL_THRESHOLD * 1000) {
        ESP_LOGE(TAG, "⚠️ WARNING: Heap critically low!");
    };
};

void wifi_rssi_monitor() {
    int8_t rssi = wifi_get_rssi();
    ESP_LOGI(TAG, "Wi-Fi Signal Strength: %d dBm [%s]\n", rssi,
        (rssi > -30) ? "Excellent" :
        (rssi > -40) ? "Good" :
        (rssi > -60) ? "Fair" :
        (rssi > -80) ? "Weak" : "Very Weak");
}


uint8_t system_mqtt_setting(cJSON *buffer) {
    char  *id      = cJSON_GetObjectItem(buffer, "id")->valuestring;
    char  *serial     = cJSON_GetObjectItem(buffer, "sn")->valuestring;

    printf("[OPERATION] id: %s\n", id);
    printf("[OPERATION] serial: %s\n", serial);

    cJSON *params  = cJSON_GetObjectItem(buffer, "params");
    uint16_t paramsCount = cJSON_GetArraySize(params);
    Settings settings;
    settings_load(&settings);
    // check time params
    for (int i = 0; i < paramsCount; i++) {
        cJSON *param = cJSON_GetArrayItem(params, i);
        char *name = cJSON_GetObjectItem(param, "name")->valuestring;
        char *value = cJSON_GetObjectItem(param, "value")->valuestring;
        if (strcmp(name, "wifi_ssid") == 0) {
            strcpy(settings.wifi.ssid, value);
        } else if (strcmp(name, "wifi_password") == 0) {
            strcpy(settings.wifi.password, value);
        } else if (strcmp(name, "mqtt_username") == 0) {
            strcpy(settings.server.username, value);
        } else if (strcmp(name, "mqtt_password") == 0) {
            strcpy(settings.server.password, value);
        } else if (strcmp(name, "device_serial") == 0) {
            strcpy(settings.device_serial, value);
        } else if (strcmp(name, "utc") == 0) {
            strcpy(settings.time.utc, value);
        } else if (strcmp(name, "ota_url") == 0) {
            strcpy(settings.ota.url, value);
        } else if (strcmp(name, "measurement_speed") == 0) {
            settings.measurementSpeed = atoi(value);
        } else if (strcmp(name, "current_time") == 0) {
            uint8_t result[2];
            time_text_to_number(value, result);
            settings.time.hour = result[0];
            settings.time.minute = result[1];
        } else if (strcmp(name, "enable_wifi_tx_power") == 0) {
            settings.wifi.enable_tx_power = atoi(value);
        }
    };
    settings_save(&settings);
    cJSON_Delete(buffer);
    return 0;
};

void system_mqtt_get_systeminfo(const char *sub_topic) {
    Settings settings;
    settings_load(&settings);
    // build JSON object
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", "0");
    cJSON_AddStringToObject(root, "sn", settings.device_serial);
    cJSON_AddStringToObject(root, "label", "system info");
    cJSON_AddStringToObject(root, "label_en", "system info");
    cJSON_AddStringToObject(root, "ctrl", "N/A");
    cJSON_AddStringToObject(root, "wifi_ssid",settings.wifi.ssid);
    cJSON_AddStringToObject(root, "wifi_password", settings.wifi.password);
    cJSON_AddStringToObject(root, "mqtt_url", settings.server.url);
    cJSON_AddStringToObject(root, "mqtt_username", settings.server.username);
    cJSON_AddStringToObject(root, "mqtt_password", settings.server.password);
    cJSON_AddStringToObject(root, "ota_url", settings.ota.url);
    cJSON_AddStringToObject(root, "utc", settings.time.utc);
    // convert JSON to string
    char *json_string = cJSON_Print(root);
    // Publish to mqtt
    char topic[64] = {0};
    snprintf(topic, sizeof(topic), "%s/%s/%s", GATEWAY_ID, settings.device_serial, sub_topic);
    mqtt_publish2(topic, json_string, 2,1);
    // release memory
    cJSON_Delete(root);
    free(json_string);
}

#define LAZY_CONNECTION_TIME 30*1000UL // in developing condition.

void check_connected() {
    if (get_ticks() < LAZY_CONNECTION_TIME) {
        printf("Waiting %03ld\r", (LAZY_CONNECTION_TIME - get_ticks()) / 1000);
        vTaskDelay(pdMS_TO_TICKS(100)); // 添加短延迟，避免频繁调用
        return;
    }
    static uint32_t prev_ticks = 0;
    uint32_t now = get_ticks();
    if(calculate_diff(now, prev_ticks) >= 10000) {
        if(wifi_get_status() == DISCONNECTED) {
            // if(mqtt_get_status() == CONNECTED) {
            //     mqtt_stop();
            // };
            ESP_ERROR_CHECK(esp_wifi_connect());
        } else if(wifi_get_status() == CONNECTED) {
            // if(mqtt_get_status() == DISCONNECTED) {
            //     mqtt_restart();
            //     printf("mqtt_restart\r\n");
            // }
        }
        prev_ticks = now;
    }
}



void generate_random_six_digit(char *buffer) {
    int num = (rand() % 900000) + 100000; // 產生 100000~999999
    snprintf(buffer, 7, "%u", num); // 轉成字串 (6位數 + null 結尾)
}


// Monitor Head size.
void system_monitor_task(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    while (1) {
        heap_size_monitor();
        wifi_rssi_monitor();
        vTaskDelay(pdMS_TO_TICKS(SYSTEM_CHECK_INTERVAL_MS * 1000)); // delay
    };
    vTaskDelete(NULL);
}


