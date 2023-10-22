#include "Arduino.h"
// #include "soc/rtc.h"
#include "HX711.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "HX711"

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

void measure_weight(void) {
    scale.set_scale(-1.94); // calibration factor = (reading)/(known weight)
    scale.tare(20);

    while(true) {
        ESP_LOGI(TAG, "Leitura: %f", scale.get_units());
        ESP_LOGI(TAG, "Média: %f", scale.get_units(20));
        scale.power_down();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        scale.power_up();
    }
}

void scale_main(void) {
    ESP_LOGI(TAG, "Initializing scale.");
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    // calibration_factor_reading();
    measure_weight();
}


extern "C" void app_main(void) {
    scale_main();
}