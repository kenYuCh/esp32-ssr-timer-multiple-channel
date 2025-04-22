#ifndef NRF24_INIT_H
#define NRF24_INIT_H


#include <stdint.h>
#include "slave_data.h"

typedef enum {
    STATE_IDLE,
    STATE_SEND,
    STATE_WAIT_FOR_RESPONSE,
    STATE_ERROR
} nrf24_state_t;

typedef enum {
    READ,
    WRITE,
} io_state;


typedef struct nrf24_data {
    uint8_t start_byte;
    uint8_t data_len;
} NRF24_data_t;

typedef struct {
    uint8_t start_byte;
    uint8_t data_len;
} SendConfig_t;

void nrf24_task();
void restartPrimaryTask();


#endif // !NRF24_INIT_H
