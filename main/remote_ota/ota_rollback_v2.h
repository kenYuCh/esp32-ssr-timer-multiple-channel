#ifndef OTA_ROLLBACK_V2_H
#define OTA_ROLLBACK_V2_H


#include "setting.h"
void ota_init(Settings *settings);
void run_ota2(Settings *settings);
void advanced_ota_example_task(void *pvParameter) ;
#endif // !TEST_OTA_2_H