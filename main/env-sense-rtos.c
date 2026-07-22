#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_bus.h"


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
    bus_err_t initerr = i2c_bus_init();
    bus_err_t probeerr = i2c_bus_probe(0x77);
    xTaskCreate(task_one, "task_one", 2048, NULL, 5, NULL);
    xTaskCreate(task_two, "task_two", 2048, NULL, 5, NULL);
}
