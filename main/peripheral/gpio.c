#include "driver/gpio.h"
#include "app_wifi.h"
#include "esp_log.h"
#include "gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "status.h"


#define OFF 0
#define ON 1

/*
GPIO_MODE_DISABLE                                             
GPIO_MODE_INPUT                                                                             
GPIO_MODE_OUTPUT                                                                  
GPIO_MODE_OUTPUT_OD                   
GPIO_MODE_INPUT_OUTPUT_OD
GPIO_MODE_INPUT_OUTPUT
*/
void gpio_init(uint8_t gpio_pin, uint8_t gpio_mode) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; 
    io_conf.pin_bit_mask = 1ULL << gpio_pin;
    io_conf.mode = GPIO_MODE_OUTPUT;      // 設置為輸出模式
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;  // 禁用上拉電阻
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉電阻
    gpio_config(&io_conf);
    gpio_set_direction(gpio_pin, gpio_mode);
};
// mode: OFF, ON
void gpio_set_off(uint8_t gpio_pin) {
    gpio_set_level(gpio_pin, OFF);
}
void gpio_set_on(uint8_t gpio_pin) {
    gpio_set_level(gpio_pin, ON);
};

