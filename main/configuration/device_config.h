
#ifndef DEVICE_CONFIG_H

#define SEEED_ESP32_S3 0
#define SEEED_ESP32_WROOM_D3 1

// --------------------------------------- DEVICE Base ------- Check
#define MAX_PORT 30
#define GATEWAY_ID "AA01830FF1"
#define USE_MODEL MODEL_TYPE_SENSOR
#define USE_MODEL_METHOD METHOD_I2C_TYPE
#define USE_BOARD SEEED_ESP32_WROOM_D3


// --------------------------------------- DEVICE sensor -------- Check
#define DEVICE_ID 65535
#if USE_MODEL == MODEL_TYPE_SENSOR
    #define DEVICE_MEASUREMENT_POINT 3                                
    #define DEVICE_MEASUREMENT_NAME_1 ""
    #define DEVICE_MEASUREMENT_NAME_2 ""
    #define DEVICE_MEASUREMENT_NAME_3 ""
#else

#endif //






// ---------
#endif //