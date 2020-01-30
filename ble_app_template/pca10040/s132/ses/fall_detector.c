#include "accelerometer.h"
#include "nrf_log.h"
#include "app_timer.h"
#include "app_error.h"

#include <stdint.h>

enum FallDetectorState {
    WAITING_FOR_FREE_FALL = 0x01,
    WAITING_FOR_ACTIVITY = 0x02,
    WAITING_FOR_INACTIVITY = 0x03,
};
typedef uint8_t FallDetectorState;

static FallDetectorState _current_state = WAITING_FOR_FREE_FALL;
bool _fall_detected = false;

APP_TIMER_DEF(m_app_timer_id);

void handle_free_fall(void);
void handle_activity(void);
void handle_inactivity(void);
void reset_fall_detector_state(void * p_context);
void start_fall_detector_timer(uint32_t timer_timeout_ms);
void stop_fall_detector_timer(void);

void fall_detector_init(void) {
    ret_code_t err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_SINGLE_SHOT, reset_fall_detector_state);
    APP_ERROR_CHECK(err_code);
}

void update_fall_detector_state(void) {
    switch (_current_state) {
        case WAITING_FOR_FREE_FALL: 
            if (accelerometer_free_fall()) {
                NRF_LOG_DEBUG("Detected free fall while waiting for free fall.");
                handle_free_fall();
            }
            break;
        case WAITING_FOR_ACTIVITY: 
            if (accelerometer_activity()) {
                NRF_LOG_DEBUG("Detected activity while waiting for activity.");
                handle_activity();
            }
            break;
        case WAITING_FOR_INACTIVITY:
            if (accelerometer_inactivity()) {
                NRF_LOG_DEBUG("Detected inactivity while waiting for inactivity.");
                handle_inactivity();
            }
            break;
    }
}

bool fall_detected(void) {
    return _fall_detected;
}

bool clear_fall_detected(void) {
    _fall_detected = false;
}

void handle_free_fall(void) {
    _current_state = WAITING_FOR_ACTIVITY;
    clear_accelerometer_flags();
    start_fall_detector_timer(500);
}

void handle_activity(void) {
    stop_fall_detector_timer();
    _current_state = WAITING_FOR_INACTIVITY;
    clear_accelerometer_flags();
    start_fall_detector_timer(1500);
}

void handle_inactivity(void) {
    stop_fall_detector_timer();
    _current_state = WAITING_FOR_FREE_FALL;
    clear_accelerometer_flags();
    _fall_detected = true;
}

void reset_fall_detector_state(void * p_context) {
    _current_state = WAITING_FOR_FREE_FALL;
}

void start_fall_detector_timer(uint32_t timer_timeout_ms) {
    ret_code_t err_code = app_timer_start(m_app_timer_id, APP_TIMER_TICKS(timer_timeout_ms), NULL);
    APP_ERROR_CHECK(err_code);
}

void stop_fall_detector_timer(void) {
    ret_code_t err_code = app_timer_stop(m_app_timer_id);
    APP_ERROR_CHECK(err_code);
}
