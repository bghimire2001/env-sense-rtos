#pragma once

#include <stdint.h>
#include <stddef.h>

/*
SPI Bus Layer so that device drivers stay hardware-independent and host-testable
*/

/*
Thread Safety: all transactions are serialize on an internal mutex owned by the backend
*/

typedef struct{
    uint8_t cs; /* Chip Select */
    uint8_t loc_width; /* Width of the memory/address locations either 1 or 2 bytes */
} spi_device_t;

typedef enum {
    BUS_OK = 0,
    BUS_ERR_NACK,
    BUS_ERR_TIMEOUT,
    BUS_ERR_INVALID,
    BUS_ERR_IO,
} spi_err_t;

/*
Initialize spi bus

Initialize shared pins, speed, and internal mutex. Configured in the backend
calling again after success is a no op returning BUS_OK

*/
spi_err_t spi_bus_init(void);

/*
*
* Address-only probe: Read from a known register.
* No device side effects. Requires init
* Returns BUS_OK if a device ACKed; BUS_ERR_NACK if silent
* Errors: BUS_ERR_TIMEOUT, BUS_ERR_IO
*
**/

spi_err_t spi_device_probe(uint8_t cs);

/*
* Spi device read
*
* Read a certain size of data from an spi device
*
* cs: cs pin of spi device
* loc: defines location to read from
* data_size: how much data to read in bytes
*
*
*/
spi_err_t spi_device_read(const spi_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size);

/*
* Spi device write
*
* write a certain size of data from an spi device
*
* cs: cs pin of spi device
* loc: defines location to read from
* data_size: how much data to read in bytes
*
*
*/
spi_err_t spi_device_write(const spi_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size);