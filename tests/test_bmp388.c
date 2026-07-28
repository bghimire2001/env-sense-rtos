/*
 * test_bmp388.c — regression test for BMP388 compensation math.
 *
 * WHAT THIS PROVES (be precise about this — it's an interview-worthy distinction):
 *   This is a REGRESSION test, not a correctness proof. It captures a known-good
 *   (raw input + calibration + expected output) tuple from verified hardware —
 *   readings you confirmed were physically accurate for your location — and
 *   asserts the compensation math still reproduces them. It catches drift if you
 *   later refactor the math. It does NOT independently prove the formula is
 *   correct; for that, cross-check against Bosch's reference BMP3 API driver.
 *
 * HOW TO POPULATE:
 *   1. On the running sensor, log the RAW 24-bit temp and pressure integers
 *      (before compensation) plus the compensated outputs, for one reading.
 *   2. Also log (or reconstruct from the NVM read) the calibration coefficients.
 *   3. Fill every TODO below with those captured values.
 *   4. `make test` — green means the math reproduces verified hardware output.
 *
 * Compile is handled by the Makefile (it adds the -I paths and -lm).
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>

/* Pull in the driver source to reach the static compensation functions.
 * This also drags in the i2c_bus_* declarations; we stub them below so the
 * linker is satisfied (the compensation path never calls them). */
#include "bmp388.c"

/* ---- linker stubs: never called by the math, just need to exist ---- */
bus_err_t i2c_bus_init(void) { return BUS_OK; }
bus_err_t i2c_bus_probe(uint8_t addr) { (void)addr; return BUS_OK; }
bus_err_t i2c_bus_read(const i2c_device_t *dev, uint8_t *data,
                       uint32_t loc, size_t len) {
    (void)dev; (void)data; (void)loc; (void)len; return BUS_OK;
}
bus_err_t i2c_bus_write(const i2c_device_t *dev, const uint8_t *data,
                        uint32_t loc, size_t len) {
    (void)dev; (void)data; (void)loc; (void)len; return BUS_OK;
}

/* ============================================================
 * CAPTURED VALUES — fill these in from your verified sensor.
 * ============================================================ */

/* from the same run */
#define RAW_TEMP   0x7FE800
#define RAW_PRES   0x721000
#define EXPECT_TEMP_C    22.83f
#define EXPECT_PRES_PA   97726.0f

/* Tolerances — how close is "still correct". Loosen if float noise trips them. */
#define TEMP_TOL   0.10f          /* +/- 0.1 C                                  */
#define PRES_TOL   5.0f           /* +/- 5 Pa                                   */

/* The calibration coefficients as your driver SCALED them (the floats stored
 * in bmp388_calib_t after unpacking), NOT the raw NVM integers. Capture these
 * by logging sensor->calib fields after bmp388_init, or reconstruct from the
 * NVM bytes with your scaling. Fill each TODO. */
static bmp388_calib_t make_captured_calib(void)
{
    bmp388_calib_t c = {
        .t1  =  7.1114240000e+06f,
        .t2  =  1.8007121980e-05f,
        .t3  = -3.5527136788e-14f,
        .p1  = -1.6077995300e-02f,
        .p2  = -3.5235658288e-05f,
        .p3  =  8.1490725279e-09f,
        .p4  =  0.0000000000e+00f,   /* genuinely ~0 at this scale */
        .p5  =  2.1020000000e+05f,
        .p6  =  4.7806250000e+02f,
        .p7  = -5.0781250000e-02f,
        .p8  = -3.0517578125e-04f,
        .p9  =  6.0897065168e-11f,
        .p10 =  6.7501559897e-14f,
        .p11 = -1.6263032587e-18f,
    };
    return c;
}

/* ============================================================
 * TESTS
 * ============================================================ */

static void test_temperature(void)
{
    bmp388_calib_t cal = make_captured_calib();
    float t_lin = 0.0f;
    float temp = compensate_temp(RAW_TEMP, &cal);

    printf("  temp: got %.4f, expected %.4f (tol %.2f)\n",
           temp, (float)EXPECT_TEMP_C, (float)TEMP_TOL);
    assert(fabsf(temp - EXPECT_TEMP_C) < TEMP_TOL);
}

static void test_pressure(void)
{
    bmp388_calib_t cal = make_captured_calib();

    /* pressure compensation needs t_lin; compute temp first to get it.
     * NOTE: if you refactor compensate_press to take t_lin as a parameter
     * (recommended — removes the shared-struct coupling), pass it here
     * instead of relying on cal->t_lin being set as a side effect. */
    float t_lin = compensate_temp(RAW_TEMP, &cal);

    float pres = compensate_press(RAW_PRES, &cal, t_lin);   /* or (RAW_PRES, &cal, t_lin) after refactor */

    printf("  pres: got %.4f, expected %.4f (tol %.2f)\n",
           pres, (float)EXPECT_PRES_PA, (float)PRES_TOL);
    assert(fabsf(pres - EXPECT_PRES_PA) < PRES_TOL);
}

static void test_sanity_bounds(void)
{
    /* Cheap physical-plausibility floor — catches gross bugs even if the
     * captured expected values were themselves off. BMP388 rated ranges. */
    bmp388_calib_t cal = make_captured_calib();
    float t_lin = 0.0f;
    float temp = compensate_temp(RAW_TEMP, &cal);
    float pres = compensate_press(RAW_PRES, &cal, temp);

    assert(temp > -40.0f && temp < 85.0f);       /* sensor operating range   */
    assert(pres > 30000.0f && pres < 110000.0f); /* sensor pressure range    */
    printf("  sanity bounds: ok\n");
}

int main(void)
{
    printf("bmp388 compensation tests:\n");
    test_temperature();
    test_pressure();
    test_sanity_bounds();
    printf("bmp388: all tests pass\n");
    return 0;
}