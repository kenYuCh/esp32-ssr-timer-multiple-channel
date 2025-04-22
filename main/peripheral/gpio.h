#ifndef GPIO_H
#define GPIO_H


#include <stdint.h>
void gpio_init(uint8_t gpio_pin, uint8_t gpio_mode);
void gpio_set_off(uint8_t gpio_pin) ;
void gpio_set_on(uint8_t gpio_pin);

#endif // !