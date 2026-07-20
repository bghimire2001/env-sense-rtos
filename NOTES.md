## I2C_BUS.H
##### Needs to be kept universal. No ESP specifics in here.

## I2C_BUS_ESP.C
##### Specific to the ESP-32-S3-WROOM-1.

#### Lessons Learned:
- Must check what CreateSemaphore returns
- Init is required to be ran before anything else. The init will create the Mutex but not take it.
    - We assume that no tasks are running at this point.


