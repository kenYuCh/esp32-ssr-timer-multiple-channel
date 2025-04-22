#ifndef DATETIME_H
#define DATETIME_H

#include <stdint.h>
#include <time.h>

typedef struct currentTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    char utc[16];
} CurrentTime_t;

void sntp_time_init(char *ntp_server, char *name, char *value, int overwrite);
void sync_time_with_ntp();
CurrentTime_t *get_current_time_instance();
void get_current_time(CurrentTime_t *currentTime);
void formatCurrentTime(CurrentTime_t *time, char *buffer, size_t buffer_size);
void countdown_task(void *pvParameter);
void time_text_to_number(char *text, uint8_t *result);
#endif 