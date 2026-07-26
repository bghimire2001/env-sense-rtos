#include "bmp388.h"
#include "bmp388_regs.h"
static float compensate_temp(uint32_t raw, const bmp388_calib_t *cal, float *t_lin_out){
    float partial_data1;
    float partial_data2;

    partial_data1 = (float)(raw - (cal->t1));
    partial_data2 = (float)(partial_data1 * (cal->t2));

    float t_lin = partial_data2 + (partial_data1 * partial_data1) * (cal->t3);
    *t_lin_out = t_lin;
    return t_lin;

}
static float compensate_press(uint32_t raw, const bmp388_calib_t *cal){
    float comp_press;
    float partial_data1;
    float partial_data2;
    float partial_data3;
    float partial_data4;
    float partial_out1;
    float partial_out2;

    partial_data1 = (cal->p6) * (cal->t_lin);
    partial_data2 = (cal->p7) * ((cal->t_lin)*(cal->t_lin));
    partial_data3 = (cal->p8) * ((cal->t_lin)*(cal->t_lin)*(cal->t_lin));
    partial_out1 = (cal->p5) + partial_data1 + partial_data2 + partial_data3;

    partial_data1 = (cal->p2) * (cal->t_lin);
    partial_data2 = (cal->p3) * ((cal->t_lin)*(cal->t_lin));
    partial_data3 = (cal->p4) * ((cal->t_lin)*(cal->t_lin)*(cal->t_lin));
    partial_out2 = (float)raw * ((cal->p1)+ partial_data1 + partial_data2 + partial_data3);

    partial_out2 = (float)raw * ((cal->p1) + partial_data1 + partial_data2 + partial_data3);

    partial_data1 = (float)raw * (float)raw;
    partial_data2 = (cal->p9) + (cal->p10)* (cal->t_lin);
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + ((float)raw * (float)raw * (float)raw) * (cal->p11);
    comp_press = partial_out1 + partial_out2 + partial_data4;
    return comp_press;
}

bmp388_err_t bmp388_init(bmp388_t *sensor, uint8_t addr){
    bus_err_t probeerr = i2c_bus_probe(addr);
    if(probeerr != BUS_OK){
        return BMP388_ERR_INVALID;
    }
    /* Instantiate Device */
    i2c_device_t bmp388 = {
        .addr = addr,
        .addr_bit = 7,
        .loc_width = 2
    };
    sensor->dev = bmp388;

    /* Configure Temp and Pressure Compensation Values*/
    uint8_t param_data[21];
    bus_err_t err = i2c_bus_read(&bmp388, param_data, BMP388_CALIB_DATA, BMP388_CALIB_LEN);
    if(err != BUS_OK){
        return BMP388_ERR_BUS;
    }
    uint16_t par_t1 = (uint16_t)((uint16_t)param_data[0] | ((uint16_t)param_data[1] << 8));
    uint16_t par_t2 = (uint16_t)((uint16_t)param_data[2] | ((uint16_t)param_data[3] << 8));
    int8_t par_t3 = (int8_t)(param_data[4]);
    int16_t par_p1 = (int16_t)(((uint16_t)param_data[5] | ((uint16_t)param_data[6] << 8)));
    int16_t par_p2 = (int16_t)(((uint16_t)param_data[7] | ((uint16_t)param_data[8] << 8)));
    int8_t par_p3 = (int8_t)(param_data[9]);
    int8_t par_p4 = (int8_t)(param_data[10]);
    uint16_t par_p5 = (uint16_t)((uint16_t)param_data[11] | ((uint16_t)param_data[12] << 8));
    uint16_t par_p6 = (uint16_t)((uint16_t)param_data[13] | ((uint16_t)param_data[14] << 8));
    int8_t par_p7 = (int8_t)(param_data[15]);
    int8_t par_p8 = (int8_t)(param_data[16]);
    int16_t par_p9 = (int16_t)(((uint16_t)param_data[17] | ((uint16_t)param_data[18] << 8)));
    int8_t par_p10 = (int8_t)(param_data[19]);
    int8_t par_p11 = (int8_t)(param_data[20]);

    bmp388_calib_t calib = {
        .t1 = (float) par_t1 / BMP388_SCALE_T1,
        .t2 = (float) par_t2 / BMP388_SCALE_T2,
        .t3 = (float) par_t3 / BMP388_SCALE_T3,
        .p1 = ((float) par_p1 - BMP388_OFFSET_P1) / BMP388_SCALE_P1,
        .p2 = ((float) par_p2 - BMP388_OFFSET_P2) / BMP388_SCALE_P2,
        .p3 = (float) par_p3 / BMP388_SCALE_P3,
        .p4 = (float) par_p4 / BMP388_SCALE_P4,
        .p5 = (float) par_p5 / BMP388_SCALE_P5,
        .p6 = (float) par_p6 / BMP388_SCALE_P6,
        .p7 = (float) par_p7 / BMP388_SCALE_P7,
        .p8 = (float) par_p8 / BMP388_SCALE_P8,
        .p9 = (float) par_p9 / BMP388_SCALE_P9,
        .p10 = (float) par_p10 / BMP388_SCALE_P10,
        .p11 = (float) par_p11 / BMP388_SCALE_P11,
    };
    return BMP388_ERR_INVALID;
}

bmp388_err_t bmp388_read(bmp388_t *sensor, bmp388_reading_t *out){
    return BMP388_ERR_INVALID;
}