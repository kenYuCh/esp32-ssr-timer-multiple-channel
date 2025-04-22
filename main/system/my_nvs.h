#ifndef MY_NVS_H
#define MY_NVS_H

#include <stdint.h>

uint8_t nvs_helpers_save(const char *key, void *data, unsigned int length);
uint8_t nvs_helpers_load(const char *key, void *data, unsigned int length);

uint8_t mynvs_write(const char *namespace, const char *key, void *data, unsigned int length);
uint8_t mynvs_read(const char *namespace, const char *key, void *data, unsigned int length);
void erase_nvs();
#endif