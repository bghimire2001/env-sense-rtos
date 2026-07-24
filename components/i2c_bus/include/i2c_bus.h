#pragma once

#include <stdint.h>
#include <stddef.h>
/*
Bus layer serving BOTH register-style devices (1-byte reg addr) and memory-style devices (2-byte mem addr), 
so that device drivers stay hardware-independent and host-testable.
*/

/* Thread Safety: all transaction functions are serialized on an internal mutex
owned by the backend
*/


// Definition of an i2c devices
typedef struct {
    uint8_t addr; /* i2c Address of the device */
    uint8_t addr_bit; /* Width of it's i2c address 7 or 10 */
    uint8_t loc_width; /*Width of the memory/address locations either 1 or 2 bytes*/
} i2c_device_t;

typedef enum {
    BUS_OK = 0,
    BUS_ERR_NACK,       /* device didn't acknowledge          */
    BUS_ERR_TIMEOUT,    /* bus/transaction timed out          */
    BUS_ERR_INVALID,    /* bad args (null ptr, width not 1/2) */
    BUS_ERR_IO,         /* anything else the backend reports  */
} bus_err_t;

/*
Initialize i2c bus

Initialize PINs, Speed, and internal mutex. Configured in the backend
calling again after success is a no op returning BUS_OK

*/
bus_err_t i2c_bus_init(void);

/* Address-only probe: START, addr+W, check ACK, STOP. No data moves,
 * no device side effects. Requires init.
 * Returns BUS_OK if a device ACKed; BUS_ERR_NACK if silent.
 * Errors: BUS_ERR_TIMEOUT, BUS_ERR_IO. */

bus_err_t i2c_bus_probe(uint8_t addr);

/*
i2c device write

Write a certain size of data from the i2c device bus.

args:
i2c_device: has i2c address and what width the memory/register address are
loc: defines location to write to
data_size: how much data to write in bytes
data: pointer to data to write


*/
bus_err_t i2c_bus_write(const i2c_device_t* curr_device, const uint8_t* data, uint32_t loc, size_t data_size);


/*
i2c device read

Read a certain size of data from an the i2c bus.

i2c_device: has i2c address and address width.
loc: defines location to read from
data_size: how much data to read in bytes

*/
bus_err_t i2c_bus_read(const i2c_device_t* curr_device, uint8_t *data, uint32_t loc, size_t data_size);