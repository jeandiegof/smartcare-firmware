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
    switch (_current_state) {
        // TODO: Add timeout so that we go back to WAITING_FOR_FREE_FALL if we are 
        // spending too much time on WAITING_FOR_ACTIVITY or WAITING_FOR_INACTIVITY
        case WAITING_FOR_FREE_FALL: 
            if (accelerometer_free_fall()) {
                _current_state = WAITING_FOR_ACTIVITY;
                clear_accelerometer_flags();
            }
            break;
        case WAITING_FOR_ACTIVITY: 
            if (accelerometer_activity()) {
                _current_state = WAITING_FOR_INACTIVITY;
                clear_accelerometer_flags();
            }
            break;
        case WAITING_FOR_INACTIVITY:
            if (accelerometer_inactivity()) {
                _current_state = WAITING_FOR_FREE_FALL;
                clear_accelerometer_flags();
                _fall_detected = true;
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
