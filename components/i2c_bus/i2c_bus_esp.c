#include "i2c_bus.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


#define I2C_BUS_SDA_GPIO  4
#define I2C_BUS_SCL_GPIO  5
#define I2C_BUS_FREQ_HZ   400000
#define CONTROLLER_I2C_PORT 0

static SemaphoreHandle_t bus_lock = NULL;
static i2c_master_bus_handle_t bus_handle = NULL;
static bool initialized = false;

static bus_err_t esp_to_bus_err(esp_err_t e)
{
    switch (e) {
        case ESP_OK:                return BUS_OK;
        case ESP_ERR_TIMEOUT:       return BUS_ERR_TIMEOUT;
        case ESP_ERR_INVALID_ARG:   return BUS_ERR_INVALID;
        /* NACK code TBD empirically — see NOTES */
        default:                    return BUS_ERR_IO;
    }
}
/*
* Initialize PINs, Speed, and internal mutex
*/
bus_err_t i2c_bus_init(void){
    esp_err_t err = ESP_OK;
    if(initialized){
        return BUS_OK;
    }
    if(!bus_lock){
        bus_lock = xSemaphoreCreateMutex();
        if (!bus_lock) {
            return BUS_ERR_IO;
        }
    }

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = CONTROLLER_I2C_PORT,
        .scl_io_num = I2C_BUS_SCL_GPIO,
        .sda_io_num = I2C_BUS_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
        };
    err = i2c_new_master_bus(&i2c_mst_config, &bus_handle);


    if(err != ESP_OK){
        vSemaphoreDelete(bus_lock);
        bus_lock = NULL;
        return esp_to_bus_err(err);

    }
    initialized = true;
    return BUS_OK;
}