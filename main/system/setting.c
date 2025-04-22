#include "setting.h"
#include <stdio.h>
#include <string.h>
#include "my_nvs.h"


static Settings my_settings = {0};
// Function prototypes
// static uint8_t settings_checksum_generate(Settings *settings);


static uint8_t settings_checksum_generate(Settings *settings) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < sizeof(Settings) - 2; i++) {
        checksum ^= ((uint8_t *)settings)[i];
    };
    checksum ^= 0xFF;
    return checksum;
}

void settings_load(Settings *settings) {
    nvs_helpers_load("SETTINGS", (void *)settings, sizeof(Settings));
    memcpy(&my_settings, settings, sizeof(Settings));
    // // Check checksum
    uint8_t checksum = settings_checksum_generate(settings);
    if (checksum != settings->checksum) {
        // printf("-- Factory reset --\n");
        // settings_factory_reset(settings);
    }
}

void settings_save(Settings *settings) {
    // Measurement speed minimum 20 seconds
    if (settings->measurementSpeed < 10) {
        settings->measurementSpeed = 60;
    };
    // Measurement speed maximum 30 seconds
    if (settings->measurementSpeed > 3600) {
        settings->measurementSpeed = 3600;
    };
    uint8_t checksum = settings_checksum_generate(settings);
    settings->checksum = checksum;
    nvs_helpers_save("SETTINGS", (void *)settings, sizeof(Settings));
}

void settings_print(Settings *settings) {
    printf(" - WiFi SSID: %s\n", settings->wifi.ssid);
    printf(" - WiFi Password: %s\n", settings->wifi.password);
    printf(" - MQTT URL: %s\n", settings->server.url);
    printf(" - MQTT Username: %s\n", settings->server.username);
    printf(" - MQTT Password: %s\n", settings->server.password);
    printf(" - Measurement publish period (seconds): %d\n", settings->measurementSpeed);
    printf(" - UTC: %s\n", settings->time.utc);
    printf(" - DeviceSerial: %s\n", settings->device_serial);
    printf(" - OTA_URL: %s\n", settings->ota.url);
}

void settings_factory_reset(Settings *settings) {
    memset(settings, 0, sizeof(Settings));
    strcpy(settings->wifi.ssid, "admin");
    strcpy(settings->wifi.password, "password");
    strcpy(settings->server.url, "mqtts://admin.com");
    strcpy(settings->server.username, "admin");
    strcpy(settings->server.password, "password");
    strcpy(settings->ota.url, "https://driver.google.com");
    strcpy(settings->device_serial, "ModelSerial");
    strcpy(settings->time.utc, "CST-8");
    settings->measurementSpeed = 60;
    settings_save(settings);
}
