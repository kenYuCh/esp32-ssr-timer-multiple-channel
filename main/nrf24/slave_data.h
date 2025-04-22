#ifndef SLAVE_DATA_H
#define SLAVE_DATA_H

#include <stdint.h>

#define SLAVE_DATA_SIZE 256

uint8_t *get_slave_data();

void set_slave_data_by_uint8_t(uint8_t regAddr, uint8_t data);
uint8_t get_slave_data_by_uint8_t(uint8_t regAddr);
void set_slave_data_by_uint16_t(uint8_t regAddr, uint16_t data);
uint16_t get_slave_data_by_uint16_t(uint8_t regAddr);

uint8_t *get_slave_data_segment(uint8_t start, uint8_t length);

#endif // !