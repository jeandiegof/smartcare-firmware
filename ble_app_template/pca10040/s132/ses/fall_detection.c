#include "accelerometer.h"

enum FallDetectorState {
    WAITING_FOR_FREE_FALL = 0x01,
    WAITING_FOR_ACTIVITY = 0x02,
    WAITING_FOR_INACTIVITY = 0x03,
};
typedef uint8_t FallDetectorState;

static FallDetectorState _current_state = WAITING_FOR_FREE_FALL;
bool _fall_detected = false;


void update_fall_detector_state(void) {
    if (is_interrupt_available()) {
        handle_interrupts();
        NRF_LOG_INFO("\r\nFalls: %d, Activity: %d, Inactivity: %d",
            accelerometer_free_fall(), accelerometer_activity(), accelerometer_inactivity()
        );
    }

    switch (_current_state) {
        case WAITING_FOR_FREE_FALL: 
            if (accelerometer_free_fall()) {
                _current_state = WAITING_FOR_ACTIVITY;
                clear_accelerometer_free_fall();
            }
            break;
        case WAITING_FOR_ACTIVITY: 
            if (accelerometer_activity()) {
                _current_state = WAITING_FOR_INACTIVITY;
                clear_accelerometer_activity();
            }
            break;
        case WAITING_FOR_INACTIVITY:
            if (accelerometer_inactivity()) {
                _current_state = WAITING_FOR_FREE_FALL;
                clear_accelerometer_inactivity();
                _fall_detected = true;
            }
            break;            
    }
}

bool fall_detected(void) {
    return _fall_detected
}