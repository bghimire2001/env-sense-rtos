#include "i2c_bus.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "string.h"



#define I2C_BUS_SDA_GPIO  4
#define I2C_BUS_SCL_GPIO  5
#define I2C_BUS_FREQ_HZ   400000
#define I2C_BUS_MAX_PAYLOAD 64
#define I2C_BUS_TIMEOUT_MS 100
#define I2C_BUS_MAX_DEVICES 4
#define CONTROLLER_I2C_PORT 0

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
        case ESP_ERR_NOT_FOUND:     return BUS_ERR_NACK;
        default:                    return BUS_ERR_IO;
    }
}
static struct {
    bool used;
    uint8_t addr;
    i2c_master_dev_handle_t handle;
} dev_table [I2C_BUS_MAX_DEVICES];
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

/* Caller MUST hold bus_lock. Returns cached handle for dev, creating it
 * on first use. */
static bus_err_t get_dev_handle(const i2c_device_t *dev, i2c_master_dev_handle_t *out)
{
    int free_slot = -1;

    for (int i = 0; i < I2C_BUS_MAX_DEVICES; i++) {
        if (dev_table[i].used && dev_table[i].addr == dev->addr) {
            *out = dev_table[i].handle;         /* cache hit */
            return BUS_OK;
        }
        if (!dev_table[i].used && free_slot < 0) {
            free_slot = i;                       /* remember first free slot */
        }
    }

    if (free_slot < 0) {
        return BUS_ERR_IO;                       /* table full */
    }

    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = dev->addr,
        .scl_speed_hz    = I2C_BUS_FREQ_HZ,
    };
    esp_err_t err = i2c_master_bus_add_device(bus_handle, &cfg,
                                              &dev_table[free_slot].handle);
    if (err != ESP_OK) {
        return esp_to_bus_err(err);
    }

    dev_table[free_slot].used = true;            /* NOW record it */
    dev_table[free_slot].addr = dev->addr;
    *out = dev_table[free_slot].handle;
    return BUS_OK;
}


/*
Probe i2c bus

Probe address - addr - on the i2c bus

If not initialized, return BUS_ERR_INVALID

If takes too long to obtain lock, return BUS_ERR_TIMEOUT


*/
bus_err_t i2c_bus_probe(uint8_t addr){
    ESP_LOGD(TAG, "Before Lock");
    if(!initialized || !bus_lock){
        return BUS_ERR_INVALID;
    }
    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }
    ESP_LOGD(TAG, "After Lock");
    esp_err_t err = i2c_master_probe(bus_handle, addr, I2C_BUS_TIMEOUT_MS);
    xSemaphoreGive(bus_lock);
    ESP_LOGD(TAG, "probe 0x%02X -> %s", addr, esp_err_to_name(err));
    return esp_to_bus_err(err);
}
/*
Write to i2c bus

args:
- curr_device: device address and address width info.
- data: data to write
- loc: location on device to write to
- data_size: size in bytes to write
*/

bus_err_t i2c_bus_write(const i2c_device_t *dev, const uint8_t *data, uint32_t loc, size_t data_size)
{
    if (!initialized || !bus_lock)                  return BUS_ERR_INVALID;
    if (!dev || (!data && data_size > 0))           return BUS_ERR_INVALID;
    if (data_size > I2C_BUS_MAX_PAYLOAD)            return BUS_ERR_INVALID;
    if (dev->loc_width != 1 && dev->loc_width != 2) return BUS_ERR_INVALID;
    if (dev->loc_width == 1 && loc > 0xFF)          return BUS_ERR_INVALID;
    if (dev->loc_width == 2 && loc > 0xFFFF)        return BUS_ERR_INVALID;

    uint8_t buf[2 + I2C_BUS_MAX_PAYLOAD];
    size_t n = 0;
    if (dev->loc_width == 2) {
        buf[n++] = (uint8_t)(loc >> 8); 
    }
    buf[n++] = (uint8_t)(loc & 0xFF);
    memcpy(buf + n, data, data_size);
    n += data_size;

    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }
    i2c_master_dev_handle_t h;
    bus_err_t be = get_dev_handle(dev, &h);
    if (be != BUS_OK) {
        xSemaphoreGive(bus_lock);
        return be;
    }
    esp_err_t err = i2c_master_transmit(h, buf, n, I2C_BUS_TIMEOUT_MS);
    xSemaphoreGive(bus_lock);
    return esp_to_bus_err(err);
}

/*
Read from i2c bus

args:
- curr_device: device address and address width info.
- data: data to write
- loc: location on device to write to
- data_size: size in bytes to write
*/

bus_err_t i2c_bus_read(const i2c_device_t* dev, uint8_t *data, uint32_t loc, size_t data_size){
    if (!initialized || !bus_lock)                  return BUS_ERR_INVALID;
    if (!dev || !data)                              return BUS_ERR_INVALID;
    if (data_size > I2C_BUS_MAX_PAYLOAD)            return BUS_ERR_INVALID;
    if (dev->loc_width != 1 && dev->loc_width != 2) return BUS_ERR_INVALID;
    if (dev->loc_width == 1 && loc > 0xFF)          return BUS_ERR_INVALID;
    if (dev->loc_width == 2 && loc > 0xFFFF)        return BUS_ERR_INVALID;
    if (data_size == 0)                             return BUS_ERR_INVALID;


    uint8_t buf[2];
    size_t n = 0;
    if (dev->loc_width == 2) {
        buf[n++] = (uint8_t)(loc >> 8); 
    }
    buf[n++] = (uint8_t)(loc & 0xFF);

    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }
    i2c_master_dev_handle_t h;
    bus_err_t be = get_dev_handle(dev, &h);
    if (be != BUS_OK) {
        xSemaphoreGive(bus_lock);
        return be;
    }
    esp_err_t err = i2c_master_transmit_receive(h, buf, n, data, data_size, I2C_BUS_TIMEOUT_MS);
    xSemaphoreGive(bus_lock);
    return esp_to_bus_err(err);
}