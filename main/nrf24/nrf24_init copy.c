#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "mirf.h"


static NRF24_t dev;  // **全域變數，供 sender & receiver 共用**
const uint8_t payload = 32;
const uint8_t channel = CONFIG_RADIO_CHANNEL;

#define CONFIG_PRIMARY 0
#define CONFIG_SECONDARY 1

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

#if CONFIG_PRIMARY
void primary(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	uint8_t payload = 32;
	uint8_t channel = CONFIG_RADIO_CHANNEL;
	Nrf24_config(&dev, channel, payload);

	// Set my own address using 5 characters
	esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"ABCDE");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(NULL), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

	// Set destination address using 5 characters
	ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
	if (ret != ESP_OK) {
		ESP_LOGE(pcTaskGetName(NULL), "nrf24l01 not installed");
		while(1) { vTaskDelay(1); }
	}

#if CONFIG_ADVANCED
	AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

	// Print settings
	Nrf24_printDetails(&dev);

	uint8_t buf[32];
	while(1) {
		TickType_t nowTick = xTaskGetTickCount();
		sprintf((char *)buf, "Hello World %"PRIu32, nowTick);
		Nrf24_send(&dev, buf);
		memset(buf, 0, sizeof(buf));
		ESP_LOGI(pcTaskGetName(NULL), "Wait for sending.....");
		if (Nrf24_isSend(&dev, 1000)) {
			ESP_LOGI(pcTaskGetName(NULL),"Send success:%s", buf);

			// Wait for response
			//ESP_LOGI(pcTaskGetName(NULL), "Wait for response.....");
			bool received = false;
			for(int i=0;i<100;i++) {
				if (Nrf24_dataReady(&dev)) {
					received = true;
					break;
				}
				vTaskDelay(1);
			}
			if (received) {
				Nrf24_getData(&dev, buf);
				TickType_t diffTick = xTaskGetTickCount() - nowTick;
				ESP_LOGI(pcTaskGetName(NULL), "Got response:[%s] Elapsed:%"PRIu32" ticks", buf, diffTick);
			} else {
				ESP_LOGW(pcTaskGetName(NULL), "No response");
			}

		} else {
			ESP_LOGW(pcTaskGetName(NULL),"Send fail:");
		}
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
#endif // CONFIG_PRIMARY

#if CONFIG_SECONDARY
void secondary(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	NRF24_t dev;
	Nrf24_init(&dev);
	uint8_t payload = 32;
	uint8_t channel = CONFIG_RADIO_CHANNEL;
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

#if CONFIG_ADVANCED
	AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

	// Print settings
	Nrf24_printDetails(&dev);
	ESP_LOGI(pcTaskGetName(NULL), "Listening...");

	uint8_t buf[32];

	// Clear RX FiFo
	while(1) {
		if (Nrf24_dataReady(&dev) == false) break;
		Nrf24_getData(&dev, buf);
	}

	while(1) {
		// Wait for received data
		if (Nrf24_dataReady(&dev)) {
			Nrf24_getData(&dev, buf);
			ESP_LOGI(pcTaskGetName(NULL), "Got data:%s", buf);
			Nrf24_send(&dev, buf);
			ESP_LOGI(pcTaskGetName(NULL), "Wait for sending.....");
			if (Nrf24_isSend(&dev, 1000)) {
				ESP_LOGI(pcTaskGetName(NULL),"Send success:%s", buf);
			} else {
				ESP_LOGW(pcTaskGetName(NULL),"Send fail:");
			}
		}
		vTaskDelay(1);
	}
}
#endif // CONFIG_SECONDARY

	
void nrf24_task()
{
#if CONFIG_PRIMARY
	xTaskCreate(&primary, "PRIMARY", 1024*3, NULL, 2, NULL);
#endif

#if CONFIG_SECONDARY
	xTaskCreate(&secondary, "SECONDARY", 1024*3, NULL, 2, NULL);
#endif
}