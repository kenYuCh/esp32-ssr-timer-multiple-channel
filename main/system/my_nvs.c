#include "my_nvs.h"

#include "nvs_flash.h"
#include "nvs.h"


/**
 * @brief 
 * 
 * @param key Max 15 characters
 * @param data 
 * @param length 
 * @return uint8_t 
 */
uint8_t nvs_helpers_save(const char *key, void *data, unsigned int length) {

    nvs_handle_t handle = 0;
    esp_err_t returnValue = nvs_open("simugro", NVS_READWRITE, &handle);
    if (returnValue != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(returnValue));
        return 1;
    }

    // Write
    printf("Saving values in NVS ... \n");

    returnValue = nvs_set_blob(handle, key, data, length);

    printf((returnValue != ESP_OK) ? "Failed!\n" : "OK\n");

    // Commit written value.
    printf("Committing updates in NVS ... ");
    returnValue = nvs_commit(handle);
    if (returnValue != ESP_OK) {
        printf("Failed\n");
        printf("%d\n", returnValue);

    } else {
        printf("OK\n");
    }

    // Close
    nvs_close(handle);

    printf("\n");

    return 0;
}


/**
 * @brief
 *
 * @param key Max 15 characters
 * @param data
 * @param length
 * @return uint8_t
 */
uint8_t nvs_helpers_load(const char *key, void *data, unsigned int length) {

    nvs_handle_t handle = 0;
    esp_err_t returnValue = nvs_open("simugro", NVS_READWRITE, &handle);
    if (returnValue != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(returnValue));
        return 1;
    }

    // Read
    returnValue = nvs_get_blob(handle, key, data, &length);

    if (returnValue != ESP_OK) {
        //printf("NVS loading key \"%s\" failed\n", key);

        // Close
        nvs_close(handle);
        return 1;
    }

    // Close
    nvs_close(handle);

    return 0;
}


void erase_nvs() {
    esp_err_t err = nvs_flash_erase();
    if (err == ESP_OK) {
        printf("NVS erased successfully\n");
    } else {
        printf("Failed to erase NVS: %s\n", esp_err_to_name(err));
    }
}





uint8_t mynvs_write(const char *namespace, const char *key, void *data, unsigned int length) {
    nvs_handle_t handle = 0;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(ret));
        return 1;
    }
    ret = nvs_set_blob(handle, key, data, length);
    if((ret != ESP_OK))
        printf("NVS Error %s\n", esp_err_to_name(ret));
    ret = nvs_commit(handle);
    if (ret != ESP_OK)
        printf("NVS Error %s\n", esp_err_to_name(ret));
    nvs_close(handle);
    return 0;
}


uint8_t mynvs_read(const char *namespace, const char *key, void *data, unsigned int length) {
    nvs_handle_t handle = 0;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        printf("NVS Error %s\n", esp_err_to_name(ret));
        return 1;
    }
    ret = nvs_get_blob(handle, key, data, &length);
    /*if (ret != ESP_OK)
        printf("NVS Error %s\n", esp_err_to_name(ret));
        */
    nvs_close(handle);
    return 0;
}
