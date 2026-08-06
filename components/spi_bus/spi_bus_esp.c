#include "spi_bus.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "string.h"

#define SPI_CS1 10
#define SPI_BUS_MOSI_GPIO 11
#define SPI_BUS_SCLK 12
#define SPI_BUS_MISO_GPIO 13

#define SPI_BUS_MAX_DEVICES 4


#define SPI_BUS_CLK_FRQ_HZ 4000000
#define SPI_BUS_TIMEOUT_MS 100

static SemaphoreHandle_t bus_lock = NULL;
static bool initialized = false;

static struct {
    spi_device_handle_t handle;
    uint8_t cs;
    bool used;
} dev_table [SPI_BUS_MAX_DEVICES];

/* Error Transalation */

static spi_err_t esp_to_bus_err(esp_err_t e)
{
    switch (e) {
        case ESP_OK:                return BUS_OK;
        case ESP_ERR_TIMEOUT:       return BUS_ERR_TIMEOUT;
        case ESP_ERR_INVALID_ARG:   return BUS_ERR_INVALID;
        case ESP_ERR_NOT_FOUND:     return BUS_ERR_NACK;
        default:                    return BUS_ERR_IO;
    }
}

/*
Initialize the 
*/
spi_err_t spi_bus_init(void){
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

    /* SPI Config */

    if(err != ESP_OK){
        vSemaphoreDelete(bus_lock);
        bus_lock = NULL;
        return esp_to_bus_err(err);
    }
    return BUS_ERR_INVALID;
}

spi_err_t spi_device_probe(uint8_t cs){
    if(!initialized || !bus_lock){
        return BUS_ERR_INVALID;
    }
    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(SPI_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }

    xSemaphoreGive(bus_lock);
    return BUS_ERR_INVALID;
}

spi_err_t spi_device_read(const spi_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size){
    /*Set up Buffer*/

    /* Take Lock */
    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(SPI_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }
    /* Read Data */

    /*Give Lock*/
    xSemaphoreGive(bus_lock);

    /*Return Translated Error*/
    return BUS_ERR_INVALID;
}

spi_err_t spi_device_write(const spi_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size){
    /*Set up Buffer*/

    /* Take Lock */
    if (xSemaphoreTake(bus_lock, pdMS_TO_TICKS(SPI_BUS_TIMEOUT_MS)) != pdTRUE) {
        return BUS_ERR_TIMEOUT;
    }
    /* Read Data */

    /*Give Lock*/
    xSemaphoreGive(bus_lock);

    /*Return Translated Error*/
    return BUS_ERR_INVALID;
}

