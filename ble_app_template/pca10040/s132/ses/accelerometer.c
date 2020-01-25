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

uint8_t accelerometer_free_fall(void) {
    return adxl345_free_fall();
}

void clear_accelerometer_free_fall(void) {
    adxl345_clear_free_fall();
}

uint8_t accelerometer_activity(void) {
    return adxl345_activity();
}

void clear_accelerometer_activity() {
    adxl345_clear_activity();
}

uint8_t accelerometer_inactivity(void) {
    return adxl345_inactivity();
}

void clear_accelerometer_inactivity(void) {
    adxl345_clear_inactivity();
}

bool is_interrupt_available(void) {
    return adxl345_is_interrupt_available();
}

void handle_interrupts(void) {
    adxl345_handle_interrupts();
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
