#include "accelerometer.h"
#include "adxl345.h"

#include "nrf_log.h"

void accelerometer_start(void) {
    adxl345_init();
    adxl345_setup();
    adxl345_start_free_fall_mode();
    adxl345_start_activity_detection_mode();
    adxl345_start_inactivity_detection_mode();
}

uint8_t accelerometer_fall_count(void) { return adxl345_fall_count(); }

uint8_t accelerometer_activity_count(void) { return adxl345_activity_count(); }

uint8_t accelerometer_inactivity_count(void) {
    return adxl345_inactivity_count();
}

void accelerometer_print_axis_data(void) {
    const uint8_t* samples = adxl345_request_axis_data();

    int16_t data_packed[3] = {
        (int16_t)(((uint16_t)samples[1] << 8) | samples[0]),
        (int16_t)(((uint16_t)samples[3] << 8) | samples[2]),
        (int16_t)(((uint16_t)samples[5] << 8) | samples[4])};

    float x = data_packed[0] / 256.f;
    float y = data_packed[1] / 256.f;
    float z = data_packed[2] / 256.f;

    NRF_LOG_INFO("X: %d\tY: %d\tZ: %d", (int16_t)(x * 100), (int16_t)(y * 100),
                 (int16_t)(z * 100));
}

void handle_accelerometer_interruption(void) {
    NRF_LOG_DEBUG("It reads which type of event has generated the interruption and runs its internal state machine.");

    adxl345_handle_interrupt();

    NRF_LOG_INFO("\r\nFalls: %d", accelerometer_fall_count());
    NRF_LOG_INFO("\r\nActivity: %d", accelerometer_activity_count());
    NRF_LOG_INFO("\r\nInactivity: %d", accelerometer_inactivity_count());
}
