#include "accelerometer.h"
#include "fall_detector.h"
#include "services.h"
#include "adxl345.h"

#include "nrf_log.h"

void accelerometer_start(void) {
    adxl345_init();
    adxl345_setup();
    adxl345_start_free_fall_mode();
    adxl345_start_activity_detection_mode();
    adxl345_start_inactivity_detection_mode();
}

uint8_t accelerometer_free_fall(void) { return adxl345_free_fall(); }

uint8_t accelerometer_activity(void) { return adxl345_activity(); }

uint8_t accelerometer_inactivity(void) { return adxl345_inactivity(); }

void clear_accelerometer_flags(void) {
    adxl345_clear_free_fall();
    adxl345_clear_activity();
    adxl345_clear_inactivity();
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
    NRF_LOG_DEBUG(
        "It reads which type of event has generated the interruption and runs "
        "its internal state machine.");

    adxl345_handle_interrupt();
    update_fall_detector_state();

    if (fall_detected()) {
        NRF_LOG_INFO("\r\nFall Detected!");
        notify_fall_detection(true);
        clear_fall_detected();
    }
}
