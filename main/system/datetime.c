#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "datetime.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "system.h"
#include "wifi_provisioning/manager.h"

static const char *TAG = "NTP_TIME";
static CurrentTime_t currentTime = {0};

CurrentTime_t *get_current_time_instance() {
    return &currentTime;
}

void time_sync_notify_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void setup_time_zone(char *name, char *value, int overwrite) {
    setenv(name, value, overwrite); // zone localtime
    tzset(); // auto update time.
};
/*
const char* NTP_SERVER = "pool.ntp.org";
setenv("TZ", "CST-8", 1);
*/
void sntp_time_init(char *ntp_server, char *name, char *value, int overwrite) {
    // setting NTP serve
    setup_time_zone(name, value, overwrite);
    // start SNTP client（NTP sync）
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, ntp_server);
    sntp_set_time_sync_notification_cb(time_sync_notify_cb);
    esp_sntp_init();
    // wait sync
    // struct tm timeinfo = { 0 };
    // int retry = NTP_WAIT_DELAY;
    // while (timeinfo.tm_year < (2050 - 1900) && retry--) {
    //     ESP_LOGI(TAG, "Waiting for system time to be set... (%d)", retry);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     if(retry == 0) break;
    // };
}


void sync_time_with_ntp() {
    // get current time epoach
    // struct timeval now;
    // gettimeofday(&now, NULL);
    // convert_epoch_to_hms(now.tv_sec);  
    CurrentTime_t *currentTime = get_current_time_instance();
    get_current_time(currentTime);
    ESP_LOGI(TAG, "Date: %04d-%02d-%02d Time: %02d:%02d:%02d",
        currentTime->year + 1900, currentTime->month + 1, currentTime->day,
        currentTime->hour, currentTime->minute, currentTime->second);
};

void get_current_time(CurrentTime_t *currentTime) {
    struct timeval now;
    struct tm *timeinfo;
    gettimeofday(&now, NULL);
    timeinfo = localtime(&now.tv_sec); // will epoch seconds convert to localtime.
    currentTime->year   = timeinfo->tm_year;
    currentTime->month  = timeinfo->tm_mon;
    currentTime->day    = timeinfo->tm_mday;
    currentTime->hour   = timeinfo->tm_hour;
    currentTime->minute = timeinfo->tm_min;
    currentTime->second = timeinfo->tm_sec;
};



void formatCurrentTime(CurrentTime_t *time, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, 
             "%04u-%02u-%02u %02u:%02u:%02u",
             time->year, time->month, time->day,
             time->hour, time->minute, time->second);
}




void time_text_to_number(char *text, uint8_t *result) {
    uint8_t index = -1;
    for (size_t i = 0; i < strlen(text); i++) {
        if (text[i] == ':') {
            text[i] = '\0';
            index = i + 1;
            break;
        };
    };
    result[0] = atoi(text);
    result[1] = 0;
    if (index > 0) {
        result[1] = atoi(text + index);
    };
}