#ifndef PINOUT_H
#define PINOUT_H
#include "device_config.h"

// ------ MODEL_TYPE Define
#define MODEL_TYPE_SENSOR 0
#define MODEL_TYPE_SWITCH 1
// ------ METHOD_TYPE Define
#define METHOD_I2C_TYPE 0
#define METHOD_UART_TYPE 1
#define METHOD_PWM_TYPE 2
#define METHOD_PULSE_TYPE 3
#define METHOD_GPIO_TYPE 4
#define METHOD_4_20mA_TYPE 5

// --------------------------------------- DEVICE Pinout (Don't touch)
#if USE_BOARD == SEEED_ESP32_WROOM_D3
#define PIN_LED 2
#define PIN_I2C_SDA_1 35
#define PIN_I2C_CLK_1 34
#define PIN_I2C_SDA_2 1
#define PIN_I2C_CLK_2 3

#define PIN_UART_TX_1 4
#define PIN_UART_RX_1 5
#define PIN_UART_TX_2 4
#define PIN_UART_RX_2 5

#elif USE_BOARD == SEEED_ESP32_S3
#define TEST 1

#else
#define TEST2 1

#endif

// --------------------------------------
#endif // !PINOUT_H