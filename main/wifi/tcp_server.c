#include "tcp_server.h"
#include <string.h>
#include <sys/param.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"

static const char *TAG = "TCP_SERVER";

#define PORT 8202

void tcp_server_task(void *pvParameters) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char rx_buffer[128];

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Socket creation failed.");
        vTaskDelete(NULL);
        return;
    }

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Socket bind failed.");
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    listen(listen_sock, 5);
    ESP_LOGI(TAG, "Server listening on port %d", PORT);

    while (1) {
        int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            ESP_LOGE(TAG, "Client connection failed.");
            continue;
        }

        ESP_LOGI(TAG, "Client connected.");
        while (1) {
            int len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len <= 0) break;

            rx_buffer[len] = '\0';
            ESP_LOGI(TAG, "Received: %s", rx_buffer);
            send(client_sock, "ACK", strlen("ACK"), 0);
        }
        close(client_sock);
    }
}






