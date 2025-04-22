// --------------------- SYSTEM FUNCTION--------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// --------------------- ESP-API ---------------------
#include "access_point.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_random.h"
#include "esp_task_wdt.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio.h"
#include "mirf.h"
#include "mqtt_callback.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
// --------------------- HELPERS ---------------------
#include "cJSON.h"
#include "app_wifi.h"
#include "locks.h"
#include "setting.h"
#include "soc/gpio_num.h"
#include "switch_model.h"
#include "system_monitor.h"
#include "mqtt.h"
#include "tcp_server.h"
#include "ticks.h"
#include "datetime.h"
#include "ota.h"
#include "ota_rollback_v2.h"
#include "led.h"

#include "nrf24_init.h"

void app_main(void) {
    ESP_LOGI("SYS", "REBOOT");
    printf("Build at %s %s\n", __DATE__, __TIME__);
    vTaskDelay((esp_random() % 1000) / portTICK_PERIOD_MS);
    // ------------------------------- NVS
    ESP_LOGI("NVS", "Initialze NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    };  
    ESP_ERROR_CHECK(err);
    // ------------------------------- System Settings
    Settings settings;
    // settings_factory_reset(&settings);
    settings_load(&settings);
    settings_print(&settings);
    // ------------------------------- Spiffs filesystem
    init_spiffs();
    // ------------------------------- Mutex
    lock_init_mutex();
    // ------------------------------- LED
    ESP_LOGI("SYS", "Initialize LED");
    led_init();
    // ------------------------------- I2C
    
    // ------------------------------- BLE
    // ------------------------------- WIFI
    // handling_wifi_connections(); // add web
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // ------------------------------- MQTT
    // mqtt_app_start();
    // mqtt_set_data_callback(mqtt_data_cb_handler);
    // ------------------------------- NTP-TIME
    // sntp_time_init("pool.ntp.org", "TZ", settings.time.utc, 1);
    // ------------------------------- MODEL init and Task
    // Sensor
    swv_init(&settings);
    xTaskCreate(model_task, "data_task", 16384, NULL, 2, NULL);
    // ------------------------------- Task
    // xTaskCreate(system_monitor_task, "SystemMonitorTask", 4096, NULL, 5, NULL);
    // ------------------------------- OTA Task
    // ota_init(&settings);
    // ------------------------------- Head Size init.
    ESP_LOGI("Heap", "Minimum free heap ever: %zu bytes", xPortGetMinimumEverFreeHeapSize());

    // ------------------------------- Task WatchDog
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    ESP_ERROR_CHECK(esp_task_wdt_status(NULL));
    // ------------------------------- NRF24 remote comunication
    nrf24_task();
    while (1) {
        esp_task_wdt_reset();
        if(lock_take_mutex()) {
            // check_connected();
            lock_give_mutex();
        };
        vTaskDelay(pdMS_TO_TICKS(1000));
    };
}

