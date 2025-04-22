
#ifndef SYSTEMS_H
#define SYSTEMS_H

// --------------------------------------- WIFI ---------------------------------------
#define WIFI_SSID      "SianAP"
#define WIFI_PASSWORD  "sian79917"
#define WIFI_CHECK_CONN_EVERY_X_SEC 5



// --------------------------------------- SYSTEM ---------------------------------------
#define SYSTEM_CHECK_INTERVAL_MS 60  // Check every 60 seconds.
#define SYSTEM_CRITICAL_THRESHOLD 10 // Below 10KB considered dangerous (Head size).



// --------------------------------------- BLE ---------------------------------------



// --------------------------------------- MQTT ---------------------------------------
#define MQTT_SUBSCRIBE_LEN 4
#define MQTT_SUBSCRIBE_1 "operations"
#define MQTT_SUBSCRIBE_2 "settings"
#define MQTT_SUBSCRIBE_3 "system"
#define MQTT_SUBSCRIBE_4 "states"

// --------------------------------------- OTHER ---------------------------------------
#define NTP_WAIT_DELAY 10



#endif // ! APP_BLE_H




