#include "i2c_bus.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"



#define I2C_BUS_SDA_GPIO  4
#define I2C_BUS_SCL_GPIO  5
#define I2C_BUS_FREQ_HZ   400000
#define CONTROLLER_I2C_PORT 0
#define I2C_BUS_TIMEOUT_MS 100

static const char *TAG = "i2c_bus";
static SemaphoreHandle_t bus_lock = NULL;
static i2c_master_bus_handle_t bus_handle = NULL;
static bool initialized = false;

static bus_err_t esp_to_bus_err(esp_err_t e)
{
    switch (e) {
        case ESP_OK:                return BUS_OK;
        case ESP_ERR_TIMEOUT:       return BUS_ERR_TIMEOUT;
        case ESP_ERR_INVALID_ARG:   return BUS_ERR_INVALID;
        case ESP_ERR_NOT_FOUND:      return BUS_ERR_NACK;
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

/*
Probe i2c bus

Probe address - addr - on the i2c bus

If not initialized, return BUS_ERR_INVALID

If takes too long to obtain lock, return BUS_ERR_TIMEOUT


*/
bus_err_t i2c_bus_probe(uint8_t addr){
    if(!initialized || !bus_lock){
        return BUS_ERR_INVALID;
    }
    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }
    i2c_master_dev_handle_t dev_handle;
    esp_err_t err = i2c_master_probe(bus_handle, addr, I2C_BUS_TIMEOUT_MS);
    xSemaphoreGive(bus_lock);
    ESP_LOGD(TAG, "probe 0x%02X -> %s", addr, esp_err_to_name(err));
    return esp_to_bus_err(err);
}
/*
Write to i2c bus

IN CONSTRUCTION
args:
- curr_device: device address and address width info.
- data: data to write
- loc: location on device to write to
- data_size: size in bytes to write
*/

bus_err_t i2c_bus_write(const i2c_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size){
    if(!initialized || !bus_lock){
        return BUS_ERR_INVALID;
    }
    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }
    i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = curr_device->address,
    .scl_speed_hz = I2C_BUS_FREQ_HZ,
    };
    i2c_master_dev_handle_t dev_handle;
    esp_err_t err_adddevice = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    esp_err_t err_transmit = i2c_master_transmit(dev_handle, data, data_size*8, I2C_BUS_TIMEOUT_MS);
    xSemaphoreGive(bus_lock);
    return esp_to_bus_err(err_transmit);
}