idf_component_register(
    SRCS "bmp388.c"
    INCLUDE_DIRS "include"
    REQUIRES i2c_bus            # the driver depends on the bus, not on ESP-IDF's I2C
)