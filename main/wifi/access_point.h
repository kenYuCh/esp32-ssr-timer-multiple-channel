#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include "esp_http_server.h"
// void wifi_init_sta_mode();
// void wifi_init_ap_mode();
httpd_handle_t start_webserver();
void init_spiffs();
#endif // !ACCESS_POINT_H
