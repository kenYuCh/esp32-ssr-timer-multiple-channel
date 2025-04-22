#include "app_wifi.h"
#include "access_point.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "nvs_flash.h"


#include "system.h"
#include <stdio.h>
#include <string.h>
#include "setting.h"
#include "mqtt.h"
#include "ticks.h"

#define MAX_RETRY 10
#define WIFI_RECONNECT_SEC 30

#define STATIC_IP      "192.168.1.2"
#define GATEWAY_IP     "192.168.1.1"
#define NETMASK_IP     "255.255.255.0"
#define DNS_IP         "8.8.8.8"   // Google DNS

static const char *TAG = "WIFI";

static int retry_count = 0;
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT      BIT1

static app_wifi_info wifi_info = {0};
static uint32_t wifi_ticks = 0;

static wifi_config_t wifi_sta_config = {
    .sta = {
        .ssid = "GreenCloud",
        .password = "GreenCloud",
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .sae_pwe_h2e = WPA3_SAE_PWE_BOTH
    },
};

static wifi_config_t wifi_ap_config = {
    .ap = {
        .ssid = "ESP32-Config123",      // AP 名稱
        .password = "12345678",      // AP 密碼
        .max_connection = 2,         // 最大連接設備數量
        .authmode = WIFI_AUTH_WPA2_PSK,  // WPA2 認證
    }
};

void set_wifi_tx_power() {
    // 設定最大 Wi-Fi 發射功率 (範圍 40 ~ 78，數值越大訊號越強)
    esp_err_t err = esp_wifi_set_max_tx_power(80);  // 78 對應 +20dBm（最大功率）
    if (err == ESP_OK) {
        printf("Wi-Fi TX Power Setting success\n");
    } else {
        printf("Wi-Fi TX Power setting wrong: %d\n", err);
    };
}


wifi_config_t *wifi_get_sta_config() {
    return &wifi_sta_config;
}

int8_t wifi_get_rssi(void) {
    return wifi_info.rssi;
};

uint8_t wifi_get_status(void) {
    return wifi_info.status;
};

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    wifi_ap_record_t ap_info;
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START: {
                ESP_LOGI(TAG, "Wi-Fi enable， Starting connect...");
                esp_wifi_connect();
                break;
            };
            case WIFI_EVENT_STA_DISCONNECTED: {
                ESP_LOGI(TAG, "Wi-Fi Disconnected，Trying reconnect...");
                wifi_info.status = DISCONNECTED;
                wifi_info.rssi = -127;
                if (retry_count < 5) {
                    esp_wifi_connect();
                    retry_count++;
                    ESP_LOGI("WiFi", "重新連線第 %d 次", retry_count);
                } else {
                    ESP_LOGE("WiFi", "Wi-Fi 連線失敗，設定 FAIL 標誌");
                    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                }
                break;
            };
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        wifi_info.status = CONNECTED;
        wifi_ticks = 0;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            wifi_info.rssi = ap_info.rssi;
        };
        retry_count = 0;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected! IP obtained: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    // 
    // if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    //     esp_wifi_connect();
    // } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    //     wifi_info.status = DISCONNECTED;
    //     wifi_info.rssi = -127;
    //     ESP_LOGW(TAG, "Wi-Fi Disconnected! Reconnecting...");
    //     uint32_t now = get_ticks();
    //     if(calculate_diff(now, wifi_ticks) >= 30000) {
    //         esp_wifi_connect();
    //         wifi_ticks = now;
    //     };
    //     // if (retry_count < MAX_RETRY) {
    //     //     esp_wifi_connect();
    //     //     retry_count++;
    //     // } else {
    //     //     ESP_LOGE(TAG, "Max retry limit reached! Check Wi-Fi settings.");
    //     //     uint32_t now = get_ticks();
    //     //     if(calculate_diff(now, wifi_ticks) >= (WIFI_RECONNECT_SEC * 1000)) {
    //     //         esp_wifi_connect();
    //     //         wifi_ticks = now;
    //     //     };
    //     // };
    // } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    //     wifi_info.status = CONNECTED;
    //     wifi_ticks = 0;
    //     if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
    //         wifi_info.rssi = ap_info.rssi;
    //     };
    //     ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    //     retry_count = 0; // reset retry.
    //     ESP_LOGI(TAG, "Connected! IP obtained: " IPSTR, IP2STR(&event->ip_info.ip));
    //     // mqtt_restart();
    // };
}

esp_err_t wifi_init() {
    s_wifi_event_group = xEventGroupCreate();
    if(wifi_info.status == CONNECTED) {
        ESP_ERROR_CHECK(esp_wifi_stop());
    };
    
    ESP_ERROR_CHECK(esp_netif_init()); // network
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // event_loop
    
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    Settings settings;
    settings_load(&settings);

    // override ap ssid
    memset(wifi_ap_config.ap.ssid, 0x00, sizeof(wifi_ap_config.ap.ssid));
    strncpy((char*)wifi_ap_config.ap.ssid, settings.device_serial, 32);

    // override SSID
    memset(wifi_sta_config.sta.ssid, 0x00, sizeof(wifi_sta_config.sta.ssid));
    strncpy((char*)wifi_sta_config.sta.ssid, settings.wifi.ssid, 32);
    // override Password
    memset(wifi_sta_config.sta.password, 0x00, sizeof(wifi_sta_config.sta.password));
    strncpy((char*)wifi_sta_config.sta.password, settings.wifi.password, 64);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_err_t ret_value = ESP_OK;
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);
	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", wifi_sta_config.sta.ssid, wifi_sta_config.sta.password);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", wifi_sta_config.sta.ssid, wifi_sta_config.sta.password);
		ret_value = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret_value = ESP_FAIL;
	}

    set_wifi_tx_power();
    // ------------------------------------------
    // // ** use static ip **
    // esp_netif_dhcpc_stop(sta_netif); // disable DHCP
    // esp_netif_ip_info_t ip_info;
    // inet_pton(AF_INET, STATIC_IP, &ip_info.ip);
    // inet_pton(AF_INET, NETMASK_IP, &ip_info.netmask);
    // inet_pton(AF_INET, GATEWAY_IP, &ip_info.gw);
    // esp_netif_set_ip_info(sta_netif, &ip_info);

    // // set DNS server
    // esp_netif_dns_info_t dns;
    // inet_pton(AF_INET, DNS_IP, &dns.ip.u_addr.ip4);
    // dns.ip.type = IPADDR_TYPE_V4;
    // esp_netif_set_dns_info(sta_netif, ESP_NETIF_DNS_MAIN, &dns);
    // ------------------------------------------

    printf("WiFi init completed!.\n");
    vEventGroupDelete(s_wifi_event_group);
    return ret_value;
};


void handling_wifi_connections(void) {
    printf("Handling WIFI connections.\n");
    wifi_init();
    start_webserver();
};
