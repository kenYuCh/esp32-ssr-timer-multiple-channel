#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

SemaphoreHandle_t mutex2;

void lock_init_mutex() {
    mutex2 = xSemaphoreCreateMutex();
    if( mutex2 != NULL ) {
    }
}
bool lock_take_mutex(){
    return (xSemaphoreTake(mutex2, (TickType_t)portMAX_DELAY) == pdTRUE);
}

void lock_give_mutex(){
    xSemaphoreGive(mutex2);
}