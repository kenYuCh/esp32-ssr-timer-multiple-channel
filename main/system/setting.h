#ifndef SETTING_H
#define SETTING_H


#include "datetime.h"
#include <stdint.h>
typedef struct {
    char ssid[32];
    char password[64];
    uint8_t enable_tx_power;
} WifiDetails;

typedef struct {
    char url[64];
    char username[64];
    char password[64];
} ServerDetails; // mqtt

typedef struct {
    char url[128];
} OTA_Url;

typedef struct {
    WifiDetails wifi;
    ServerDetails server;
    OTA_Url ota;
    CurrentTime_t time;
    char device_serial[16];
    uint16_t measurementSpeed;
    uint16_t checksum;
} Settings;

void settings_load(Settings *settings);

void settings_save(Settings *settings);

void settings_print(Settings *settings);

void settings_factory_reset(Settings *settings);



#endif // !