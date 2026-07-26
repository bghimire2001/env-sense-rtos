#pragma once
#include "i2c_bus.h"

typedef enum {
    BMP388_OK = 0,
    BMP388_ERR_BAD_CHIP_ID,
    BMP388_ERR_BUS,
    BMP388_ERR_INVALID,
} bmp388_err_t;

typedef struct {
    float temperature_c;
    float pressure_pa;
} bmp388_reading_t;

/*Calibration Data from Sensor*/
typedef struct {
    float t1, t2, t3;
    float p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11;
} bmp388_calib_t;

typedef struct { 
    i2c_device_t dev; 
    bmp388_calib_t calib; 
} bmp388_t;

bmp388_err_t bmp388_init(bmp388_t *sensor, uint8_t addr);
bmp388_err_t bmp388_read(bmp388_t *sensor, bmp388_reading_t *out);