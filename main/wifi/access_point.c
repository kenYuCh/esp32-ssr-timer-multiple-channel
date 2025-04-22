#include "access_point.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_wifi_types.h"
#include "mqtt.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "setting.h"
#include "system.h"
#include "app_wifi.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_spiffs.h"
// spiffs,   data, spiffs,  0x110000, 512K,
static const char *TAG = "WIFI-AP";
#define WIFI_SSID_LEN 32
#define WIFI_PASSWORD_LEN 64
#define MQTT_URL_LEN 64
#define MQTT_USERNAME_LEN 64
#define MQTT_PASSWORD_LEN 64
#define MQTT_DEVICE_SERIAL_LEN 16

#define WEB_SERVER_STACK_SIZE 16384
#define HTML_LEN 4096
#define POST_CONTENT_LEN 2048

#define CSS_CHAR_LEN 2048
#define IMAGE_CHAR_LEN 1024
#define JS_CHAR_LEN 1024

void init_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs", // 設定 SPIFFS 基本路徑
        .partition_label = "spiffs", // 使用的分區名稱
        .max_files = 5, // 最大打開文件數
        .format_if_mount_failed = true // 如果掛載失敗，格式化 SPIFFS
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE("SPIFFS", "SPIFFS Initialization failed.");
    } else {
        ESP_LOGI("SPIFFS", "SPIFFS Mounted.");
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

// -----------------------  GET  -------------------------------------
char *replace_placeholder(const char *html, const char *placeholder, const char *value) {
    size_t len = strlen(html) + strlen(value) - strlen(placeholder);
    char *result = (char *)malloc(len + 1);  // allocate space for new string.
    if (result == NULL) {
        ESP_LOGE("HTTP", "Memory allocation failed");
        return NULL;
    };
    const char *pos = html;
    char *out = result;
    while ((pos = strstr(pos, placeholder)) != NULL) {
        // copy the part before the placeholder.
        size_t prefix_len = pos - html;
        memcpy(out, html, prefix_len);
        out += prefix_len;
        // copy replacement value.
        memcpy(out, value, strlen(value));
        out += strlen(value);
        // update location and continue searching.
        pos += strlen(placeholder);
        html = pos;
    };
    // copy the remaining part
    strcpy(out, html);
    return result;
}

// Serving HTML functions.
esp_err_t serve_html(httpd_req_t *req) {
    FILE *fd = fopen("/spiffs/index.html", "r");
    if (fd == NULL) {
        ESP_LOGE("HTTP", "Failed to open index.html");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    };
    char html[HTML_LEN];  
    size_t read_size = fread(html, 1, sizeof(html) - 1, fd);
    fclose(fd);
    html[read_size] = '\0';

    // 讀取設置
    Settings settings;
    settings_load(&settings);

    // replace wifi_ssid
    char *wifi_ssid_res = replace_placeholder(html, "{wifi_ssid}", settings.wifi.ssid);
    if (wifi_ssid_res == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };
    char *wifi_pass_res = replace_placeholder(wifi_ssid_res, "{wifi_password}", settings.wifi.password);
    free(wifi_ssid_res);
    if (wifi_pass_res == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };
    char *mqtt_url_res = replace_placeholder(wifi_pass_res, "{mqtt_url}", settings.server.url);
    free(wifi_pass_res);
    if (mqtt_url_res == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };
    char *mqtt_user_res = replace_placeholder(mqtt_url_res, "{mqtt_username}", settings.server.username);
    free(mqtt_url_res);
    if (mqtt_user_res == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };
    char *mqtt_pass_res = replace_placeholder(mqtt_user_res, "{mqtt_password}", settings.server.password);
    free(mqtt_user_res);
    if (mqtt_pass_res == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };
    char *device_serial_res = replace_placeholder(mqtt_pass_res, "{device_serial}", settings.device_serial);
    free(mqtt_pass_res);
    if (device_serial_res == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };
    char *utc = replace_placeholder(device_serial_res, "{utc}", settings.time.utc);
    free(device_serial_res);
    if (utc == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };
    //settings.ota.url
    char *ota_url = replace_placeholder(utc, "{ota_url}", settings.ota.url);
    free(utc);
    if (ota_url == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    };

    // 返回修改後的 HTML
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, ota_url, strlen(ota_url));
    free(ota_url);
    return ESP_OK;
}

void parse_html_input(const char *content, char *buffer, char *little, uint8_t data_len) {
    char *start =  strstr(content, little); // start position
    uint8_t len = strlen(little);
    if(start) {
        start += len;
        char *end = strstr(start, "&");
        if (end == NULL) {
            end = strstr(start, ""); // 最後一個參數沒有 "&" 時，用 "" 結束
        };
        int buffer_len = end - start;
        if (buffer_len > 0 && buffer_len < data_len) {
            strncpy(buffer, start, buffer_len);
            buffer[buffer_len] = '\0';
        };
    };
}

// Web 伺服器處理 POST 請求，更新 Wi-Fi 設置
static esp_err_t setting_post_handler(httpd_req_t *req) {
    ESP_LOGI("WIFI_CONFIG", "Post Handler");
    httpd_resp_set_hdr(req, "Content-Type", "text/html");
    char content[POST_CONTENT_LEN] = {0};
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);
    if (ret <= 0) {
        ESP_LOGE("wifi_config", "Failed to read HTTP request");
        return ESP_FAIL;
    }
    content[ret] = '\0'; // 確保結束符號

    Settings settings;
    memset(&settings, 0, sizeof(Settings));

    // parse_wifi_config(content, ssid, password, mqtt_url, mqtt_username, mqtt_password, device_serial, utc, ota_url);
    char ssid[WIFI_SSID_LEN];
    parse_html_input(content, ssid, "wifi_ssid=", WIFI_SSID_LEN);
    snprintf(settings.wifi.ssid, sizeof(settings.wifi.ssid), "%s", ssid);

    char password[WIFI_PASSWORD_LEN];
    parse_html_input(content, password, "&wifi_password=", WIFI_PASSWORD_LEN);
    snprintf(settings.wifi.password, sizeof(settings.wifi.password), "%s", password);

    char mqtt_url[MQTT_URL_LEN];
    parse_html_input(content, mqtt_url, "&mqtt_url=", MQTT_URL_LEN);
    url_decode(mqtt_url);
    snprintf(settings.server.url, sizeof(settings.server.url), "%s", mqtt_url);

    char mqtt_username[MQTT_USERNAME_LEN];
    parse_html_input(content, mqtt_username, "&mqtt_username=", MQTT_USERNAME_LEN);
    url_decode(mqtt_username);
    snprintf(settings.server.username, sizeof(settings.server.username), "%s", mqtt_username);

    char mqtt_password[MQTT_PASSWORD_LEN];
    parse_html_input(content, mqtt_password, "&mqtt_password=", MQTT_PASSWORD_LEN);
    url_decode(mqtt_password);
    snprintf(settings.server.password, sizeof(settings.server.password), "%s", mqtt_password);

    char device_serial[MQTT_DEVICE_SERIAL_LEN];
    parse_html_input(content, device_serial, "&device_serial=", MQTT_DEVICE_SERIAL_LEN);
    url_decode(device_serial);
    snprintf(settings.device_serial, sizeof(settings.device_serial), "%s", device_serial);

    char utc[10];
    parse_html_input(content, utc, "&utc=", 10);
    url_decode(utc);
    snprintf(settings.time.utc, sizeof(settings.time.utc), "%s", utc);

    char ota_url[128] = {0};
    parse_html_input(content, ota_url, "&ota_url=", 128);
    url_decode(ota_url);
    snprintf(settings.ota.url, sizeof(settings.ota.url), "%s", ota_url);

    settings_save(&settings);
    settings_print(&settings);
    // --------------------------------------------------------------
    ESP_LOGI("wifi_config", 
        "Wi-Fi Config: SSID=%s, Password=%s, MQTT Config: URL=%s, User=%s, Password=%s, DeviceSerial=%s, UTC=%s, OTA_URL=%s",
        settings.wifi.ssid, 
        settings.wifi.password,
        settings.server.url,
        settings.server.username,
        settings.server.password,
        settings.device_serial,
        settings.time.utc,
        settings.ota.url
    );
    const char *resp_str = "<html><body><h1>Device Configured Successfully!</h1></body></html>";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
    return ESP_OK;
}

esp_err_t get_status_handler(httpd_req_t *req) {
    wifi_ap_record_t wifi_info;
    esp_err_t status = esp_wifi_sta_get_ap_info(&wifi_info);
    char response[64];
    int wifi_status = (status == ESP_OK) ? 1 : 0;  // 假设 Wi-Fi 状态
    int mqtt_status = mqtt_get_status();  // 获取 MQTT 状态
    snprintf(response, sizeof(response), "{\"wifi_status\":%d, \"mqtt_status\":%d}", wifi_status, mqtt_status);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
};

esp_err_t file_handler(httpd_req_t *req, const char *file_path, const char *mime_type, size_t buffer_size) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        ESP_LOGE("HTTP", "Failed to open %s", file_path);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    };
    httpd_resp_set_type(req, mime_type);
    char buffer[buffer_size];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        httpd_resp_send_chunk(req, buffer, bytes_read);
    };
    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t image_handler(httpd_req_t *req) {
    return file_handler(req, "/spiffs/bg.jpg", "image/jpeg", IMAGE_CHAR_LEN);
};
esp_err_t css_handler(httpd_req_t *req) {
    return file_handler(req, "/spiffs/styles.css", "text/css", CSS_CHAR_LEN);
};
esp_err_t js_handler(httpd_req_t *req) {
    return file_handler(req, "/spiffs/script.js", "application/javascript", JS_CHAR_LEN);
};

void set_httpd_url(httpd_handle_t server, const char *url, esp_err_t (*handler)(httpd_req_t *r), void *user_ctx, httpd_method_t method) {
    httpd_uri_t uri_get = {
        .uri = url,
        .method = method,
        .handler = handler,
        .user_ctx = user_ctx
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_get));
};
// 啟動 Web 伺服器
httpd_handle_t start_webserver() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = WEB_SERVER_STACK_SIZE;  // 增加堆疊大小 (默認是 4096, 8192)
    ESP_ERROR_CHECK(httpd_start(&server, &config));
    // Root
    set_httpd_url(server, "/", serve_html, NULL, HTTP_GET);
    set_httpd_url(server, "/setting", setting_post_handler, NULL, HTTP_POST);
    set_httpd_url(server, "/status", get_status_handler, NULL, HTTP_GET);
    set_httpd_url(server, "/bg.jpg", image_handler, NULL, HTTP_GET);
    set_httpd_url(server, "/styles.css", css_handler, NULL, HTTP_GET);
    set_httpd_url(server, "/script.js", js_handler, NULL, HTTP_GET);
    return server;
}

/*
index.html, Reserve a place for the end (default)

Stack Size minumm is 16384


*/