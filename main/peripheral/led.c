
#include "app_wifi.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"
#include "gpio.h"
#include "freertos/FreeRTOS.h"
#include "mqtt.h"
#include "status.h"
#include "ticks.h"

#define LED_PIN GPIO_NUM_2

void led_init() {
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}
void led_run_times(uint8_t gpio_pin, uint16_t ms, uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        gpio_set_on(gpio_pin);
        vTaskDelay(pdMS_TO_TICKS(ms));
        gpio_set_off(gpio_pin);
        vTaskDelay(pdMS_TO_TICKS(200));
    };
};

void led_process() {
    static uint32_t led_ticks = 0;
    uint32_t now = get_ticks();
    if(calculate_diff(now, led_ticks) > 5000UL) {
        if (wifi_get_status() == DISCONNECTED) {
            led_run_times(LED_PIN, 50, 3);
        } else if (mqtt_get_status() == DISCONNECTED) {
            led_run_times(LED_PIN, 50, 2);
        } else {
            led_run_times(LED_PIN, 50, 1);
        };
        led_ticks = now;
    };
}
