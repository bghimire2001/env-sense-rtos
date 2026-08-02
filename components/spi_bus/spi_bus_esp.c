#include "spi_bus.h"

spi_err_t spi_bus_init(void){
    return BUS_ERR_INVALID;
}

spi_err_t spi_device_probe(uint8_t cs){
    return BUS_ERR_INVALID;
}

spi_err_t spi_device_read(const spi_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size){
    return BUS_ERR_INVALID;
}

spi_err_t spi_device_write(const spi_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size){
    return BUS_ERR_INVALID;
}

