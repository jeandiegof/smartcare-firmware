#include "accelerometer.h"
#include "adxl345.h"

#include "nrf_log.h"

void accelerometer_start(void) {
    adxl345_init();
    adxl345_setup();
    adxl345_start_free_fall_mode();
}

uint8_t accelerometer_read_register(uint8_t reg) {
    uint8_t a = 0xAA;
    read_data(reg, &a, 1);
    return a;
}

uint8_t accelerometer_fall_count(void) {
    return adxl345_fall_count();
}

void accelerometer_print_axis_data(void) {
    const uint8_t * samples = adxl345_request_axis_data();
    
    int16_t data_packed[3] = {
        (int16_t)(((uint16_t)samples[1] << 8) | samples[0]),
        (int16_t)(((uint16_t)samples[3] << 8) | samples[2]),
        (int16_t)(((uint16_t)samples[5] << 8) | samples[4])};

    float x = data_packed[0] / 256.f;
    float y = data_packed[1] / 256.f;
    float z = data_packed[2] / 256.f;

    NRF_LOG_INFO("X: %d\tY: %d\tZ: %d", (int16_t)(x * 100), (int16_t)(y * 100), (int16_t)(z * 100));
}
