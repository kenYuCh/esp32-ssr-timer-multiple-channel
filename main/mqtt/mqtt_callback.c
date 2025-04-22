#include "mqtt_callback.h"
#include "cJSON.h"
#include "device_config.h"
#include "esp_log.h"
#include "mqtt.h"
#include "setting.h"
#include "switch_model.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "system_monitor.h"

// ------------------------------------------------------------
// global variable - data
static mqtt_data_callback_t mqtt_data_callback = NULL;
static operations_verify_function mqtt_operations_verify_func = NULL;

// ------------------------------------------------------------
void operations_mqtt_send_message(char *buffer, char *id, uint8_t statusCode, char *message);
void operations_verify_send_text(char *text);
void operations_verify_write_function_set(operations_verify_function function) {
    mqtt_operations_verify_func = function;
}
void operations_verify_send_text(char *text) {
    if (mqtt_operations_verify_func) {
        mqtt_operations_verify_func(text);
    }
}

void operations_mqtt_send_message(char *buffer, char *id, uint8_t statusCode, char *message) {
    sprintf(buffer, "{\"id\":%s,\"status\":\"%d\",\"message\":\"%s\"}",
                    id, 
                    statusCode, 
                    message);
    operations_verify_send_text(buffer);
}






// -----------------------------------------------------------
// setting MQTT callback function
void mqtt_set_data_callback(mqtt_data_callback_t callback) {
    mqtt_data_callback = callback;
};
mqtt_data_callback_t mqtt_get_data_callback() {
    return mqtt_data_callback;
};

void extract_topic_part(const char *topic) {
    char topic_copy[64]; // store string copy.
    strcpy(topic_copy, topic);  // copy string , because strtok_func can modify original string.
    char *token = strtok(topic_copy, "/"); // so "/" is divide.
    while (token != NULL) {
        // this check.
        printf("Token: %s\n", token);
        token = strtok(NULL, "/");  // get next part.
    };
}

int is_valid_json_object_or_array(const char *json_str) {
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        return 0;  // parser wrong. not json format.
    }

    // check json is object or array.
    if (!cJSON_IsObject(json) && !cJSON_IsArray(json)) {
        cJSON_Delete(json);
        return 0;
    }

    cJSON_Delete(json);
    return 1;  // parser success and json is object or array.
}

void get_topic_last_segment(const char *topic, char *result) {
    const char *last_slash = strrchr(topic, '/'); 
    if (last_slash != NULL) {
        strcpy(result, last_slash + 1); 
    } else {
        strcpy(result, topic); 
    }
}


void operations_verify(char *buffer) {
    char verify_topic[64]; 
    snprintf(verify_topic, sizeof(verify_topic), "%s/%s", GATEWAY_ID, "verify");
    mqtt_publish(verify_topic, buffer);
};

void mqtt_data_cb_handler(MqttData *mqtt) {
    ESP_LOGI("MQTT", "Custom Handler: Received message from topic %s: %s", mqtt->topic, mqtt->data);
    SwitchValve *swv = swv_get_instance();
    char last_topic_segment[50];
    get_topic_last_segment(mqtt->topic, last_topic_segment);
    char random_int[7] = {0};
    generate_random_six_digit(random_int);

    if(strcmp(last_topic_segment, MQTT_SUBSCRIBE_4) == 0) {
        printf("Topic is 'getoutputstates'. Performing action for getoutputstates.\n");
        swv_publish_msg_output_state(swv, "outputstates");
        return;
    };

    cJSON *operations = cJSON_Parse(mqtt->data); // 
    if (operations == NULL) {
        operations_verify("Warning-999: JSON Format Is Invalid"); 
        cJSON_Delete(operations);
        return;
    };
    
    cJSON  *id      = cJSON_GetObjectItem(operations, "id");
    cJSON  *serial     = cJSON_GetObjectItem(operations, "sn");
    cJSON  *label = cJSON_GetObjectItem(operations, "label");
    cJSON  *control = cJSON_GetObjectItem(operations, "ctrl");

    if(!id) {
        operations_verify("Warning-1: id parameter not found!"); 
        return;
    } else {
        strcpy(id->valuestring, random_int);
    };
    if(!serial) {
        operations_verify("Warning-2: sn parameter not found!"); 
        return;
    };
    if(!label) {
        operations_verify("Warning-3: label parameter not found!"); 
        return;
    };
    if(!control) {
        operations_verify("Warning-4: ctrl parameter not found!"); 
        return;
    };

    if(strcmp(serial->valuestring, swv->sn)!=0) {
        operations_verify("Warning-655: This device cannot be found!"); 
        return;
    }
    // --------------------------------------------------------- 
    
    if (strcmp(last_topic_segment, MQTT_SUBSCRIBE_1) == 0) { // operations
        if (!cJSON_IsObject(operations)) {
            operations_verify("Warning-41: JSON Format Is Invalid: Must be an object"); 
            cJSON_Delete(operations);
            return;
        };
        printf("Topic is 'operations'. Performing action for operations.\n");
        uint8_t result = swv_operate(swv, operations);
        if(result > 0) {
            if(result == 1) operations_verify("Warning-31: The Control parameter format is incorrect: ChannelId-PeriodId");
            if(result == 2) operations_verify("Warning-32: The Control channel is out of range."); 
            if(result == 3) operations_verify("Warning-33: Period time is not in range.");
            return;
        };
    } else if (strcmp(last_topic_segment, MQTT_SUBSCRIBE_2) == 0) { // settings
        printf("Topic is 'settings'. Performing action for settings.\n");
        system_mqtt_setting(operations);
    } else if (strcmp(last_topic_segment, MQTT_SUBSCRIBE_3) == 0) { // systeminfo
        printf("Topic is 'systeminfo'. Performing action for systeminfo.\n");
        cJSON  *pkey = cJSON_GetObjectItem(operations, "pkey");
        if(pkey) {
            if(strcmp(pkey->valuestring, "a3@90f0!2z0@sdf!oox10!0@sa") == 0) {
                system_mqtt_get_systeminfo("verify");
            } else {
                operations_verify("Warning-35: The pkey parameter format is incorrect: Wrong password!");
            };
        }
        return;
    }
    // else {
    //     operations_verify("Warning-34: Topic is unknown. No action for this topic."); 
    //     return;
    // };
    char verify_topic[64]; 
    snprintf(verify_topic, sizeof(verify_topic), "%s/%s", GATEWAY_ID, "verify");
    // mqtt_publish(verify_topic, "Success-666: Verification Success");
    mqtt_publish2(verify_topic, mqtt->data, 2, 1);
};



