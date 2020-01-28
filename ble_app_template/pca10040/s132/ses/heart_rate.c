#include "heart_rate.h"
#include "heartbeat_detector/MAX30100_BeatDetector.h"
#include "nrf_drv_systick.h"
#include "nrf_log.h"
#ifdef MAX30100
#include "max30100.h"
#else
#include "max30101.h"
#endif

#ifndef min
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif

#define POLE 0.05

#define INIT_HOLDOFF_MS 2000 // how long to wait before counting
#define MASKING_HOLDOFF_MS 40 // non-retriggerable window after beat detection
#define MIN_THRESHOLD 20
#define MAX_THRESHOLD 800
#define STEP_RESILIENCY 30 // maximum negative jump that triggers the beat edge
#define THRESHOLD_FALLOFF_TARGET 0.3 // thr chasing factor of the max value when beat
#define THRESHOLD_DECAY_FACTOR 0.99 // thr chasing factor when no beat
#define INVALID_READOUT_DELAY_MS 2000 // in ms, no-beat time to cause a reset

typedef enum BeatDetectorState {
    BeatdetectorStateInit,
    BeatdetectorStateWaiting,
    BeatdetectorStateFollowingSlope,
    BeatdetectorStateMaybeDetected,
    BeatdetectorStateMasking,
} BeatDetectorState;

static float _current_threshold = MIN_THRESHOLD;

void heart_rate_start(void) {
#ifdef MAX30100
    max30100_init();
    max30100_setup();
#else
    max30101_init();
    max30101_setup();
#endif
}

static float high_pass_filter(float sample) {
    static float _previous_sample = 0.0;
    static float _previous_retult = 0.0;
    _previous_retult = sample - _previous_sample * POLE;
    _previous_sample = sample;
    return _previous_retult;
}

// http://www.schwietering.com/jayduino/filtuino/
// order=1 alpha1=0.12
// Sampling rate 50 hz
static float butterworth_filter(float sample) {
    static float v0 = 0.0;
    static float v1 = 0.0;
    v0 = v1;
    v1 = (2.836306788762870124e-1 * sample) + (0.43273864224742591977 * v0);
    return (v0 + v1);
}

static void detect_heart_beat(float sample) {
    bool beat_detected = false;
    static BeatDetectorState _stateBeat = BeatdetectorStateInit;
    static nrf_drv_systick_state_t _current_time;

    switch (_stateBeat) {
        case BeatdetectorStateInit:
            if (nrf_drv_systick_test(&_current_time, INIT_HOLDOFF_MS)) {
                nrf_drv_systick_get(&_current_time);
                _stateBeat = BeatdetectorStateWaiting;
            }
            break;

        case BeatdetectorStateWaiting:
            if (sample > _current_threshold) {
                _current_threshold = min(sample, MAX_THRESHOLD);
                _stateBeat = BeatdetectorStateFollowingSlope;
            }

            // Tracking lost, resetting
            if (millis() - tsLastBeat > INVALID_READOUT_DELAY_MS) {
                beatPeriod = 0;
                lastMaxValue = 0;
            }

            beatDetectorDecreaseThreshold();
            break;

        case BeatdetectorStateFollowingSlope:
            if (sample < _current_threshold) {
                _stateBeat = BeatdetectorStateMaybeDetected;
            } else {
                _current_threshold = min(sample, MAX_THRESHOLD);
            }
            break;

        case BeatdetectorStateMaybeDetected:
            if (sample + STEP_RESILIENCY < _current_threshold) {
                // Found a beat
                beat_detected = true;
                lastMaxValue = sample;
                _stateBeat = BeatdetectorStateMasking;
                uint32_t delta = millis() - tsLastBeat;
                if (delta) {
                    beatPeriod = BPFILTER_ALPHA * delta +
                            (1 - BPFILTER_ALPHA) * beatPeriod;
                }

                tsLastBeat = millis();
            } else {
                _stateBeat = BeatdetectorStateFollowingSlope;
            }
            break;

        case BeatdetectorStateMasking:
            if (millis() - tsLastBeat > MASKING_HOLDOFF_MS {
    BeatdetectorStateWaiting;
               beatDetectorDecreaseThreshold();
            break;
    }

    return beat_detected;
}

static void process_new_sample(uint32_t sample) {
    float filtered_sample = high_pass_filter((float)sample);
    filtered_sample = butterworth_filter(filtered_sample);
    const bool beat_detected = detect_heart_beat(filtered_sample);
}

void handle_heart_rate_interruption(void) {
    const uint32_t sample = read_hr_sample();
    NRF_LOG_DEBUG("It processes the sample signal to get bpm's.");
    process_new_sample(sample);
    find_heart_rate()
}
