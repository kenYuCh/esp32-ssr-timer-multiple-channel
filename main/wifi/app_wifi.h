
#ifndef  APP_WIFI_H
#define APP_WIFI_H

// #include "esp_wifi_types.h"
#include "esp_err.h"
#include "status.h"
#include <stdint.h>

typedef enum {
    AP_MODE,
    STA_MODE
} wifi_mode;

typedef struct {
    char wifi_ssid[16];
    char wifi_pass[16];
    wifi_mode mode;   // sta or ap
    ConnectStatus status; // connected or disconnected
    int8_t rssi;
} app_wifi_info;

esp_err_t wifi_init();
void handling_wifi_connections(void);
int8_t wifi_get_rssi(void);
uint8_t wifi_get_status(void);

void set_wifi_tx_power();

// void wifi_task(void *pvParameters);
// void restart_wifi_task();
// void restart_wifi(const char* new_ssid, const char* new_password);
#endif // ! APP_BLE_H


