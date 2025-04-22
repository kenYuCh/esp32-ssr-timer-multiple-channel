#include "access_point.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_wifi_types.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "setting.h"
#include "system.h"
#include "app_wifi.h"
#include <ctype.h>
#include <stdio.h>
#include "esp_spiffs.h"
// spiffs,   data, spiffs,  0x110000, 512K,
static const char *TAG = "WIFI-AP";

void init_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs", // 設定 SPIFFS 基本路徑
        .partition_label = "spiffs", // 使用的分區名稱
        .max_files = 5, // 最大打開文件數
        .format_if_mount_failed = true // 如果掛載失敗，格式化 SPIFFS
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE("SPIFFS", "SPIFFS 初始化失敗");
    } else {
        ESP_LOGI("SPIFFS", "SPIFFS 已掛載");
    }
}
// -----------------------  GET  -------------------------------------
// 通用替換函數
char *replace_placeholder(const char *html, const char *placeholder, const char *value) {
    size_t len = strlen(html) + strlen(value) - strlen(placeholder);
    char *result = (char *)malloc(len + 1);  // 分配空間給新字串
    if (result == NULL) {
        ESP_LOGE("HTTP", "Memory allocation failed");
        return NULL;
    }

    const char *pos = html;
    char *out = result;
    while ((pos = strstr(pos, placeholder)) != NULL) {
        // 複製佔位符之前的部分
        size_t prefix_len = pos - html;
        memcpy(out, html, prefix_len);
        out += prefix_len;

        // 複製替換值
        memcpy(out, value, strlen(value));
        out += strlen(value);

        // 更新位置，繼續查找
        pos += strlen(placeholder);
        html = pos;
    }

    // 複製剩餘的部分
    strcpy(out, html);

    return result;
}

// 服務 HTML 函數
esp_err_t serve_html(httpd_req_t *req) {
    // **打開 index.html**
    FILE *fd = fopen("/spiffs/index.html", "r");
    if (fd == NULL) {
        ESP_LOGE("HTTP", "Failed to open index.html");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // **讀取 HTML**
    char html[2048];  // 確保足夠存放 HTML
    size_t read_size = fread(html, 1, sizeof(html) - 1, fd);
    fclose(fd);
    html[read_size] = '\0';  // 確保字串結尾

    // **讀取 Wi-Fi 設定**
    Settings settings;
    settings_load(&settings);

    // **替換 `{SSID}` 和 `{PASSWORD}`**
    char *response = replace_placeholder(html, "{SSID}", settings.wifi.ssid);
    if (response == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    char *final_response = replace_placeholder(response, "{PASSWORD}", settings.wifi.password);
    free(response);  // 釋放替換 `{SSID}` 之後的記憶體

    if (final_response == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // **傳送替換後的 HTML**
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, final_response, strlen(final_response));

    // **釋放記憶體**
    free(final_response);

    return ESP_OK;
}

// -----------------------  POST  -------------------------------------
void parse_wifi_config(const char *content, char *ssid, char *password) {
    char *ssid_start = strstr(content, "ssid=");
    char *pass_start = strstr(content, "&password=");

    if (ssid_start && pass_start) {
        ssid_start += 5;  // 跳過 "ssid="
        pass_start += 10;  // 跳過 "&password="

        // 找到 "&"（表示 password 起點），計算 SSID 長度
        int ssid_len = pass_start - ssid_start - 10;  
        if (ssid_len > 0 && ssid_len < 32) {
            strncpy(ssid, ssid_start, ssid_len);
            ssid[ssid_len] = '\0';
        }

        // 確保 password 只有內容
        strncpy(password, pass_start, 64);
        password[63] = '\0';
    }
}

void url_decode(char *str) {
    char *p = str;
    while (*str) {
        if (*str == '%') {
            int hex;
            if (sscanf(str + 1, "%2x", &hex) == 1) {
                *p++ = (char)hex;
                str += 3;
            }
        } else if (*str == '+') {
            *p++ = ' ';
            str++;
        } else {
            *p++ = *str++;
        }
    }
    *p = '\0';
}

// Web 伺服器處理 POST 請求，更新 Wi-Fi 設置
static esp_err_t wifi_config_post_handler(httpd_req_t *req) {
    printf("wifi_config_post_handler! \n");
    httpd_resp_set_hdr(req, "Content-Type", "text/html");

    char content[100] = {0};
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);

    if (ret <= 0) {
        ESP_LOGE("wifi_config", "Failed to read HTTP request");
        return ESP_FAIL;
    }
    content[ret] = '\0'; // 確保結束符號

    // 解析 Wi-Fi SSID 和密碼
    char ssid[32] = {0}, password[64] = {0};
    parse_wifi_config(content, ssid, password);
    url_decode(ssid);
    url_decode(password);

    Settings settings;
    memset(&settings, 0, sizeof(Settings));

    strncpy(settings.wifi.ssid, ssid, sizeof(settings.wifi.ssid) - 1);
    settings.wifi.ssid[sizeof(settings.wifi.ssid) - 1] = '\0';

    strncpy(settings.wifi.password, password, sizeof(settings.wifi.password) - 1);
    settings.wifi.password[sizeof(settings.wifi.password) - 1] = '\0';

    settings_save(&settings);
    settings_print(&settings);
    ESP_LOGI("wifi_config", "Wi-Fi Config: SSID=%s, Password=%s", settings.wifi.ssid, settings.wifi.password);

    const char *resp_str = "<html><body><h1>Wi-Fi Configured Successfully!</h1></body></html>";
    httpd_resp_send(req, resp_str, strlen(resp_str));

    vTaskDelay(pdMS_TO_TICKS(3000));

    esp_restart();
    return ESP_OK;
}

// 啟動 Web 伺服器
httpd_handle_t start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;  // 增加堆疊大小 (默認是 4096)
    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    // 設置處理 GET 請求的 URI 路徑
    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = serve_html,
        .user_ctx = NULL
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_get));

    // 設置處理 POST 請求的 URI 路徑
    httpd_uri_t uri_post = {
        .uri = "/set",
        .method = HTTP_POST,
        .handler = wifi_config_post_handler,
        .user_ctx = NULL
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_post));

    return server;
}
