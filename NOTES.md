## I2C_BUS.H
##### Needs to be kept universal. No ESP specifics in here.

## I2C_BUS_ESP.C
##### Specific to the ESP-32-S3-WROOM-1.

#### Lessons Learned:
- Must check what CreateSemaphore returns
- Init is required to be ran before anything else. The init will create the Mutex but not take it.
    - We assume that no tasks are running at this point.

- Use %e to show extremely small or large numbers. 

#### Next Tasks

- SPI driver
- Control some form of equipment
- Add more i2c devices
- Write more tests
- Communicate over bluetooth or wifi


