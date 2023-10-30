#include "Arduino.h"
// #include "soc/rtc.h"
#include "HX711.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* ---------------------- Scale ----------------------*/

#define TAG "HX711"
#define CALIBRATION_FACTOR -1.94

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 1;

HX711 scale;

void calibration_factor_reading(void) {
    while(true) {
        if (scale.is_ready()) {
            scale.set_scale();    
            ESP_LOGI(TAG, "Tare... remove any weights from the scale.");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            scale.tare();
            ESP_LOGI(TAG, "Tare done...");
            ESP_LOGI(TAG, "Place a known weight on the scale...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            long reading = scale.get_units(10);
            ESP_LOGI(TAG, "Result: %ld", reading);
        } else {
            ESP_LOGW(TAG, "HX711 not found.");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
}

float average_weight;

void measure_weight(void) {
    scale.set_scale(CALIBRATION_FACTOR); // calibration factor = (reading)/(known weight)
    scale.tare(20);

    while(true) {
        ESP_LOGI(TAG, "Leitura: %f", scale.get_units());
        average_weight = scale.get_units(20);
        ESP_LOGI(TAG, "Média: %f", average_weight);
        scale.power_down();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        scale.power_up();
    }
}

void scale_main(void * params) {
    ESP_LOGI(TAG, "Initializing scale.");
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    // calibration_factor_reading();
    measure_weight();
}

/* ---------------------- WiFi and MQTT ----------------------*/
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "wifi.h"
#include "mqtt.h"

#ifdef __cplusplus
}
#endif

#define TOPIC "DGM/scale"

SemaphoreHandle_t wifiConnectionSemaphore;
SemaphoreHandle_t MQTTConnectionSemaphore;

void wifiConnected(void *params){
    while(true){
        if(xSemaphoreTake(wifiConnectionSemaphore, portMAX_DELAY)){
            mqtt_start();
        }
    }
}

void mqttConnected(void* params){
    char msg[50];
    if(xSemaphoreTake(MQTTConnectionSemaphore, portMAX_DELAY))
        while (true) {
            sprintf(msg, "Média: %f", average_weight);
            mqtt_send_message(TOPIC, msg);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
        
}

void wifi_main(void) {
    // NVS initialize
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifiConnectionSemaphore = xSemaphoreCreateBinary();
    MQTTConnectionSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    xTaskCreate(&wifiConnected, "MQTT Connection", 4096, NULL, 1, NULL);
    xTaskCreate(&mqttConnected, "Broker communication", 4096, NULL, 1, NULL);
}


extern "C" void app_main(void) {
    xTaskCreate(&scale_main, "Read scale data", 4096, NULL, 1, NULL);
    wifi_main();
}