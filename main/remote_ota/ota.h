#ifndef OTA_H
#define OTA_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    uint32_t magic_number;  // 固件頭部的魔數
    uint32_t version;       // 固件版本
    uint32_t crc;           // CRC
} firmware_header_t;


void run_ota();
esp_err_t ota_rollback();
bool get_ota_update_state();

#endif // !OTA_H