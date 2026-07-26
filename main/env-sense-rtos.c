#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_bus.h"
#include "bmp388.h"


static const char *TAG = "env-sense-rtos";

static void task_one(void *arg)
{
    while (1) {
        ESP_LOGI(TAG, "Task 1 running");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task_two(void *arg)
{
    while (1) {
        ESP_LOGI(TAG, "Task 2 running");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    printf("HERE");
    ESP_LOGI(TAG, "HERE");
    bus_err_t initerr = i2c_bus_init();
    i2c_device_t bmp388 = {
        .addr = 0x77,
        .addr_bit = 7,
        .loc_width = 2
    };
    bmp388_t currsens = {
        .dev = bmp388,
    };
    
    bus_err_t probeerr = i2c_bus_probe(0x77);
    // uint8_t bmpdata[3];
    if(probeerr == BUS_OK){
        ESP_LOGI(TAG, "BMP388 found");
    } else{
        ESP_LOGI(TAG, "BMP388 not found");
    }
    bmp388_init(&currsens, bmp388.addr);
    bmp388_reading_t reading;
    while(true){
        // i2c_bus_read(&bmp388, bmpdata, 7, 3);
        bmp388_read(&currsens, &reading);
        ESP_LOGI(TAG, "%f", reading.temperature_c);
        ESP_LOGI(TAG, "%f", reading.pressure_pa);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    

}
