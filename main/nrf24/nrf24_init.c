#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "driver/spi_master.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "locks.h"

#include "mirf.h"
#include "nrf24_init.h"
#include "ticks.h"

TaskHandle_t myTaskHandle = NULL;

static nrf24_state_t currentState = STATE_IDLE;
static uint8_t is_send_mode_flag = 0;

const uint8_t payload = 32;
const uint8_t channel = CONFIG_RADIO_CHANNEL;
const int max_attempts = 1000;

QueueHandle_t sendQueue;
static NRF24_t dev;  // **全域變數，供 sender & receiver 共用**

#define CHUNK_SIZE 32
#define TOTAL_CHUNKS (SLAVE_DATA_SIZE / CHUNK_SIZE)

#define CONFIG_PRIMARY 1
#define CONFIG_SECONDARY 0

#if CONFIG_ADVANCED
void AdvancedSettings(NRF24_t * dev)
{
#if CONFIG_RF_RATIO_2M
	ESP_LOGW(pcTaskGetName(NULL), "Set RF Data Ratio to 2MBps");
	Nrf24_SetSpeedDataRates(dev, 1);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_1M
	ESP_LOGW(pcTaskGetName(NULL), "Set RF Data Ratio to 1MBps");
	Nrf24_SetSpeedDataRates(dev, 0);
#endif // CONFIG_RF_RATIO_2M

#if CONFIG_RF_RATIO_250K
	ESP_LOGW(pcTaskGetName(NULL), "Set RF Data Ratio to 250KBps");
	Nrf24_SetSpeedDataRates(dev, 2);
#endif // CONFIG_RF_RATIO_2M

	ESP_LOGW(pcTaskGetName(NULL), "CONFIG_RETRANSMIT_DELAY=%d", CONFIG_RETRANSMIT_DELAY);
	Nrf24_setRetransmitDelay(dev, CONFIG_RETRANSMIT_DELAY);
}
#endif // CONFIG_ADVANCED



void process_send_data(SendConfig_t *config) {
    uint8_t buf[CHUNK_SIZE];
    uint8_t start_index = config->start_byte;
    uint8_t data_length = config->data_len;
    buf[0] = 0xFC; 
    buf[1] = 0xAD; 
    buf[2] = data_length; // 数据长度
	uint8_t *s = get_slave_data();
    memcpy(&buf[3], &s[start_index], data_length);
    buf[3 + data_length] = 0xAD; 
    buf[4 + data_length] = 0xFC; 
};



void print_buffer(uint8_t *buf, size_t size) {
    printf("Buffer contents: ");
    for (size_t i = 0; i < size; i++) {
        printf("0x%02X ", buf[i]); // 以十六进制格式打印每个字节
    }
    printf("\n");
}

bool set_primary_config() {
	Nrf24_init(&dev);
	Nrf24_config(&dev, channel, payload);
	esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"ABCDE"); // esp32 sensor
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(NULL), "nrf24l01 not installed");
		Nrf24_deinit(&dev);
		return false;
		// while(1) { 
		// 	vTaskDelay(1); 
		// }
	}
	// Set destination address using 5 characters （target address）
	ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ"); // other esp32 or gateway
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(NULL), "nrf24l01 not installed");
		Nrf24_deinit(&dev);
		return false;
		// while(1) { vTaskDelay(1); }
	}
	#if CONFIG_ADVANCED
		AdvancedSettings(&dev);
	#endif // CONFIG_ADVANCED
	// Print settings
	Nrf24_printDetails(&dev);
	return true;
}

// esp32 sensor
#if CONFIG_PRIMARY
void primary(void *pvParameters)
{
	ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
	// ------------------------------------------------------------------------------
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	// ------------------------------------------------------------------------------
	static bool reset = false;
	if(set_primary_config() == false) {
		reset = true;
	}

	// ------------------------------------------------------------------------------
	uint8_t index = 0;
    uint8_t buf[CHUNK_SIZE] = {0x00}; // 每次发送的缓冲区
	static int timeout_counter = 0;

	while(1) {
		esp_task_wdt_reset();
		if(reset) {
			if(set_primary_config()) { // set wrong
				reset = false;
				return;
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
		};
		if(lock_take_mutex()) {
			uint8_t start_byte = index * CHUNK_SIZE;
			uint8_t *slaveData = get_slave_data_segment(start_byte, CHUNK_SIZE);
			if (slaveData == NULL) return;
			memcpy(buf, slaveData, CHUNK_SIZE); // 直接使用 slaveData
			Nrf24_send(&dev, buf);
			memset(buf, 0, sizeof(buf));
			// ------------------------------------------------------
			ESP_LOGI(pcTaskGetName(NULL), "Wait for sending.....");
			if (Nrf24_isSend(&dev, 1000)) {
				// ------------------------------------------------------
				index++;
				ESP_LOGI(pcTaskGetName(NULL),"Send success!");
				ESP_LOGI(pcTaskGetName(NULL), "Wait for response.....");
				timeout_counter = 0;
				bool received = false;
				if (index >= TOTAL_CHUNKS) {
                    index = 0; // 重置为 0，准备下一轮发送
                };
				// ------------------------------------------------------
				while (timeout_counter < max_attempts) { 
					if (Nrf24_dataReady(&dev)) {
						received = true;
						timeout_counter = 0;
						break;
					}
					vTaskDelay(pdMS_TO_TICKS(3));
					timeout_counter++;
				}
				if (timeout_counter >= max_attempts) {
					ESP_LOGW("NRF24", "Timeout waiting for data");
				}
				// ------------------------------------------------------
				if (received) {
					Nrf24_getData(&dev, buf); // Reads payload bytes into data array.
					print_buffer(buf, sizeof(buf));
				} else {
					ESP_LOGW(pcTaskGetName(NULL), "No response");
				};
				// ------------------------------------------------------
			} else {
				ESP_LOGW(pcTaskGetName(NULL),"Send fail:");
				static uint32_t ticks = 0;
				uint32_t now = get_ticks();
				if(calculate_diff(now, ticks) >= 30000) {
					ticks = now;
					reset = true;
				}
			}
			lock_give_mutex();
		};
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	// ------------------------------------------------------------------------------
}
#endif // CONFIG_PRIMARY

#if CONFIG_SECONDARY
void secondary(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	Nrf24_config(&dev, channel, payload);
	// Set my own address using 5 characters
	esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(NULL), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}
	

	// Set destination address using 5 characters
	ret = Nrf24_setTADDR(&dev, (uint8_t *)"ABCDE");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(NULL), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}
	// ------------------------------------------------------------------------------
	#if CONFIG_ADVANCED
		AdvancedSettings(&dev);
	#endif // CONFIG_ADVANCED

	// Print settings
	Nrf24_printDetails(&dev);
	ESP_LOGI(pcTaskGetName(NULL), "Listening...");

	uint8_t buf[32];

	while(1) {
		if (Nrf24_dataReady(&dev) == false) {
			break;
		}
		Nrf24_getData(&dev, buf);
		vTaskDelay(1);
	};

	// nrf24_state_t state = STATE_IDLE;	
	bool received = false;
	while(1) {
		while(1) {
			if(Nrf24_dataReady(&dev)) {
				received = true;
				break;
			};
			vTaskDelay(1);
		}
		if (received){
			received = false;
			Nrf24_getData(&dev, buf);
			print_buffer(buf, sizeof(buf));
			Nrf24_send(&dev, buf);
			/*
				start_byte (0xFC, 0xFB), 
				data_len (0x04), 
				data_array (0x03, 0x05, 0x06, 0x02), 
				stop_byte(0xFB, 0xFC), 
				crc_bytes
			*/
			// 0xFC, 0xFB, 0x04, 0x01, 0x02, 0x03, 0x04, 0xFB, 0xFC
			
			ESP_LOGI(pcTaskGetName(NULL), "Wait for sending.....");
			if (Nrf24_isSend(&dev, 1000)) {
				ESP_LOGI(pcTaskGetName(NULL),"Send success");
			} else {
				ESP_LOGW(pcTaskGetName(NULL),"Send fail:");
			}
		};
		vTaskDelay(2);
	}
}
#endif // CONFIG_SECONDARY

void nrf24_task(){
#if CONFIG_PRIMARY // sensor/switch
	xTaskCreate(&primary, "PRIMARY", 1024*3, NULL, 5, &myTaskHandle);
#endif

#if CONFIG_SECONDARY
	// xTaskCreate(&run_sensor_nrf24, "SECONDARY", 1024*3, NULL, 2, NULL);
	xTaskCreate(&secondary, "SECONDARY", 1024*3, NULL, 5, NULL);
#endif
}