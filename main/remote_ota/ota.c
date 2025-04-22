
#include "app_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "status.h"
#include "string.h"
#include "cJSON.h"

#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
#include "esp_crt_bundle.h"
#endif

static const char *TAG = "OTA_BOOTLOAD";
#define CURRENT_VERSION "0.0.9"
#define SERVER_JSON_URL   "https://drive.google.com/uc?export=download&id=1XOqSGYd3UWE_7WoQUS3XEPbP6gPnRviM"  // 伺服器 IP 地址
#define SERVER_OTA_FILE "https://drive.google.com/uc?export=download&id=1St93T6N_l1D-JiXrSlrNUosmTMy0d0we"

#define OTA_SIZE    (1024 * 1024)    // 預計固件大小

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADERS_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER: %s: %s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            // ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGE(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            break;

    }
    return ESP_OK;
}


void ota_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting OTA update from: %s", SERVER_OTA_FILE);

    // 獲取當前運行的分區
    const esp_partition_t *current_partition = esp_ota_get_boot_partition();
    ESP_LOGI(TAG, "Currently running partition: %s", current_partition->label);
    static bool ota_updated = false;
    // 獲取下一個更新分區
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(current_partition);
    if (!update_partition) {
        ESP_LOGE(TAG, "No valid OTA partition found!");
        vTaskDelete(NULL);
        return;
    };

    ESP_LOGI(TAG, "Next OTA partition: %s", update_partition->label);

    // 配置 HTTP 請求
    esp_http_client_config_t http_config = {
        .url = SERVER_OTA_FILE,
        .event_handler = _http_event_handler,
        .cert_pem = NULL  // 如果需要 SSL 憑證，請在這裡設置
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    // 執行 OTA 更新
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA update successful! Marking app as valid...");
        // 更新成功，將新固件標記為有效
        esp_ota_mark_app_valid_cancel_rollback();
        ESP_LOGI(TAG, "Restarting...");
        ota_updated = true;
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA update failed!");
        // 更新失敗，回滾到最後一個有效的分區
        esp_ota_mark_app_invalid_rollback_and_reboot();
        const esp_partition_t *last_valid = esp_ota_get_last_invalid_partition();
        if (last_valid) {
            ESP_LOGI(TAG, "Reverting to last valid partition: %s", last_valid->label);
            esp_ota_set_boot_partition(last_valid);
        }
        esp_restart();
    }

    vTaskDelete(NULL);
}


esp_err_t ota_rollback() {
    const esp_partition_t *current_partition = esp_ota_get_boot_partition();
    const esp_partition_t *previous_partition = esp_ota_get_next_update_partition(current_partition);
    if (previous_partition != NULL) {
        ESP_LOGI(TAG, "Rolling back to previous partition: %s", previous_partition->label);
        esp_ota_set_boot_partition(previous_partition); // Set the boot partition to the previous one
        esp_restart(); // Reboot the device to apply the change
    } else {
        ESP_LOGE(TAG, "No previous partition found for rollback.");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void ota_task2() {
    ESP_LOGI(TAG, "Starting OTA update from: %s", SERVER_OTA_FILE);
    // 獲取當前運行的分區
    const esp_partition_t *current_partition = esp_ota_get_boot_partition();
    ESP_LOGI(TAG, "Currently running partition: %s", current_partition->label);

    // 獲取下一個更新分區
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(current_partition);
    if (!update_partition) {
        ESP_LOGE(TAG, "No valid OTA partition found!");
        return;
    };

    ESP_LOGI(TAG, "Next OTA partition: %s", update_partition->label);

    // 配置 HTTP 請求
    esp_http_client_config_t http_config = {
        .url = SERVER_OTA_FILE,
        .event_handler = _http_event_handler,
        .cert_pem = NULL  // 如果需要 SSL 憑證，請在這裡設置
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    // 執行 OTA 更新
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA update successful! Marking app as valid...");
        // 更新成功，將新固件標記為有效
        esp_ota_mark_app_valid_cancel_rollback();
        ESP_LOGI(TAG, "Restarting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA update failed!");
        // 更新失敗，回滾到最後一個有效的分區
        esp_ota_mark_app_invalid_rollback_and_reboot();
        const esp_partition_t *last_valid = esp_ota_get_last_invalid_partition();
        if (last_valid) {
            ESP_LOGI(TAG, "Reverting to last valid partition: %s", last_valid->label);
            esp_ota_set_boot_partition(last_valid);
        }
        esp_restart();
    }
}


void run_ota() {
    vTaskDelay(20000/portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "OTA example app_main start");
    
    if(wifi_get_status() == DISCONNECTED) return;
    ota_task2();
}





/*


// // // TCP 客戶端下載固件並更新
// esp_err_t ota_tcp_update() {

//     const esp_partition_t *running_partition = esp_ota_get_running_partition();
//     ESP_LOGI(TAG, "Currently running on partition: %s", running_partition->label);
//     // --------------------------------------------- Socket
//     struct sockaddr_in dest_addr;
//     dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
//     dest_addr.sin_family = AF_INET;
//     dest_addr.sin_port = htons(SERVER_PORT);

//     // 創建套接字
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock < 0) {
//         ESP_LOGE(TAG, "Socket creation failed!");
//         return ESP_FAIL;
//     }

//     // 連接到伺服器
//     if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
//         ESP_LOGE(TAG, "Socket connection failed!");
//         close(sock);
//         return ESP_FAIL;
//     }

//     ESP_LOGI(TAG, "Connected to server");
//     // --------------------------------------
//     // // 獲取下次更新的分區
//     const esp_partition_t *update_partition = esp_ota_get_next_update_partition(running_partition);
//     ESP_LOGI(TAG, "Next update partition: %s", update_partition->label);
//     // 獲取 OTA 分區
//     // const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
//     if (update_partition == NULL) {
//         ESP_LOGE(TAG, "Error getting OTA partition");
//         close(sock);
//         return ESP_FAIL;
//     }


//     // 開始寫入固件到 OTA 分區
//     esp_ota_handle_t ota_handle;
//     esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE, &ota_handle);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Error starting OTA: %s", esp_err_to_name(err));
//         close(sock);
//         return err;
//     }

//     ESP_LOGI(TAG, "Begin OTA update");

//     // 下載固件並寫入到 Flash
//     uint8_t *buffer = malloc(1024);
//     int bytes_received;
//     while ((bytes_received = recv(sock, buffer, 1024, 0)) > 0) {
//         err = esp_ota_write(ota_handle, buffer, bytes_received);
//         if (err != ESP_OK) {
//             ESP_LOGE(TAG, "Error writing OTA data to flash: %s", esp_err_to_name(err));
//             free(buffer);
//             close(sock);
//             return err;
//         }
//         ESP_LOGI(TAG, "Received %d bytes", bytes_received);
//     }

//     if (bytes_received < 0) {
//         ESP_LOGE(TAG, "Error receiving OTA data: %d", bytes_received);
//         free(buffer);
//         close(sock);
//         return ESP_FAIL;
//     }

//     // 完成 OTA 更新
//     err = esp_ota_end(ota_handle);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Error finishing OTA: %s", esp_err_to_name(err));
//         free(buffer);
//         close(sock);
//         return err;
//     }

//     // 提交更新並重啟
//     err = esp_ota_set_boot_partition(update_partition);
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Error setting boot partition: %s", esp_err_to_name(err));
//         free(buffer);
//         close(sock);
//         return err;
//     }

//     ESP_LOGI(TAG, "OTA Update Successful, restarting...");
//     free(buffer);
//     close(sock);

//     // 重啟並執行新的固件
//     esp_restart();
//     return ESP_OK;
// }



*/

/* OK

void ota_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting OTA update from: %s", SERVER_JSON_URL);
    const esp_partition_t *current_partition = esp_ota_get_boot_partition();
    ESP_LOGI(TAG, "Currently running partition: %s", current_partition->label);
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        ESP_LOGE(TAG, "No valid OTA partition found!");
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Next OTA partition: %s", update_partition->label);

    esp_http_client_config_t http_config = {
        .url = SERVER_JSON_URL,
        .event_handler = _http_event_handler,
        .cert_pem = NULL
        // .auth_type = HTTP_AUTH_TYPE_BASIC,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA update successful! Marking app as valid...");
        esp_ota_mark_app_valid_cancel_rollback();
        ESP_LOGI(TAG, "Restarting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA update failed!");
        esp_ota_mark_app_invalid_rollback_and_reboot();
        const esp_partition_t *last_valid = esp_ota_get_last_invalid_partition();
        if (last_valid) {
            ESP_LOGI(TAG, "Reverting to last valid partition: %s", last_valid->label);
            esp_ota_set_boot_partition(last_valid);
        }
        esp_restart();
    }
    vTaskDelete(NULL);
}
*/