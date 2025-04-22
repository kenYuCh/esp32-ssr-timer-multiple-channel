idf_component_register(SRCS 
                    "main.c"
        

                    "wifi/app_wifi.c"
                    "wifi/access_point.c"
                    "wifi/tcp_server.c"

                    "helpers/cJSON.c"
        

                    "system/locks.c"
                    "system/system_monitor.c"
                    "system/my_nvs.c"
                    "system/setting.c"
                    "system/ticks.c"
                    "system/datetime.c"

                    "peripheral/gpio.c"
                    "peripheral/i2c.c"
                    "peripheral/led.c"
                    "peripheral/pulse.c"
                    "peripheral/pwm.c"
                    "peripheral/uart.c"
                    "peripheral/adc.c"

                    "mqtt/mqtt.c"
                    "mqtt/mqtt_callback.c"


           
        
                    "models/model_helper.c"
                    "models/switch_model.c"
                    "models/sensor_model.c"
                    "models/setpoint.c"
                    "models/switch_timer_func.c"
     
                    "remote_ota/ota.c"
                    "remote_ota/ota_rollback_v2.c"

                    "configuration/device_config.c"

                    "nrf24/mirf.c"
                    "nrf24/nrf24_init.c"
                    "nrf24/slave_data.c"
                    INCLUDE_DIRS 
                    "."
                    "wifi"
                    "helpers"
                    "system"
                    "configuration"
                    "models"
                    "peripheral"
                    "mqtt"
                    "remote_ota"
                    "nrf24"
                    EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem
           
                    )
