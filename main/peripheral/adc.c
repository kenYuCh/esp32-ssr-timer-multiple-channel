#include <stdio.h>
#include "esp_system.h"
#include "driver/adc.h"
#include "esp_log.h"


#define VRxPin 34  // VRx pin connected to GPIO34 (ADC1_CHANNEL_6)
#define VRyPin 35  // VRy pin connected to GPIO35 (ADC1_CHANNEL_7)
#define SwButtonPin 0  // SW pin connected to GPIO0

int pressed = -1;  // This variable will determine whether joystick has been pressed down (selected)
int x = -1;        // This variable will hold the X-coordinate value
int y = -1;        // This variable will hold the Y-coordinate value

void readJoystick(void) {
    // Read whether joystick has been pressed down (selected) or not
    pressed = gpio_get_level(SwButtonPin);
    // Read the X-coordinate value (from VRxPin)
    x = adc1_get_raw(ADC1_CHANNEL_6);  // Read from ADC1 Channel 6 (GPIO34)
    // Read the Y-coordinate value (from VRyPin)
    y = adc1_get_raw(ADC1_CHANNEL_7);  // Read from ADC1 Channel 7 (GPIO35)
}


void adc_run() {
    // Configure the SW button pin as an input
    gpio_set_direction(SwButtonPin, GPIO_MODE_INPUT);
    // Configure ADC1 for VRxPin and VRyPin
    adc1_config_width(ADC_WIDTH_BIT_12);  // Set ADC width to 12 bits (0-4095)
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);  // Set attenuation for VRxPin (GPIO34)
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_0);  // Set attenuation for VRyPin (GPIO35)
    ESP_LOGI("Joystick", "Start");
    while (1) {
        readJoystick();  // Call the function to read the joystick values

        // Print the joystick status (coordinates and button press status)
        printf("Joystick X: %d, Y: %d, Pressed: %d\n", x, y, pressed);

        // Add a delay
        // vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}