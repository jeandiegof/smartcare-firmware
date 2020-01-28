#include "heart_rate.h"
#include "app_timer.h"
#include <stdbool.h>
#include "nrf_drv_systick.h"
#include "nrf_log.h"
#ifdef MAX30100
#include "max30100.h"
#else
#include "max30101.h"
#endif

#ifndef min
#define min(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#define POLE 0.05

#define INIT_HOLDOFF_TICKS \
    APP_TIMER_TICKS(2000)  // how long to wait before counting
#define MASKING_HOLDOFF_TICKS \
    APP_TIMER_TICKS(40)  // non-retriggerable window after beat detection
#define MIN_THRESHOLD 20
#define MAX_THRESHOLD 800
#define STEP_RESILIENCY 30  // maximum negative jump that triggers the beat edge
#define THRESHOLD_FALLOFF_TARGET \
    0.3  // thr chasing factor of the max value when beat
#define THRESHOLD_DECAY_FACTOR 0.99    // thr chasing factor when no beat
#define INVALID_READOUT_DELAY_MS 2000  // in ms, no-beat time to cause a reset

typedef enum {
    BeatdetectorStateInit,
    BeatdetectorStateWaiting,
    BeatdetectorStateFollowingSlope,
    BeatdetectorStateMaybeDetected,
    BeatdetectorStateMasking,
} BeatDetectorState;

static float _current_threshold = MIN_THRESHOLD;
static uint32_t _last_time_measured_ticks = 0;
static float _beatPeriod = 0;
static float _lastMaxValue = 0;

void heart_rate_start(void) {
#ifdef MAX30100
    max30100_init();
    max30100_setup();
#else
    max30101_init();
    max30101_setup();
#endif
    _last_time_measured_ticks = app_timer_cnt_get();
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

static void beatDetectorDecreaseThreshold(void) {
    // When a valid beat rate readout is present, target the
    const uint8_t SAMPLING_PERIOD = 1000 / 50;
    if (_lastMaxValue > 0 && _beatPeriod > 0) {
        _current_threshold -= _lastMaxValue * (1 - THRESHOLD_FALLOFF_TARGET) /
                     (_beatPeriod / SAMPLING_PERIOD);
    } else {
        // Asymptotic decay
        _current_threshold *= THRESHOLD_DECAY_FACTOR;
    }

    if (_current_threshold < MIN_THRESHOLD) {
        _current_threshold = MIN_THRESHOLD;
    }
}

static float get_available_bpm(void) {
    if (_beatPeriod != 0) {
        return 1.0 / _beatPeriod * 1000.0 * 60.0;
    } else {
        return 0;
    }
}

static bool detect_heart_beat(float sample) {
    bool beat_detected = false;
    static BeatDetectorState _stateBeat = BeatdetectorStateInit;
    static uint32_t _time_last_beat_ticks = 0;

    switch (_stateBeat) {
        case BeatdetectorStateInit:
            if (app_timer_cnt_diff_compute(app_timer_cnt_get(),
                                           INIT_HOLDOFF_TICKS) > 0) {
                _last_time_measured_ticks = app_timer_cnt_get();
                _stateBeat = BeatdetectorStateWaiting;
            }
            break;

        case BeatdetectorStateWaiting:
            if (sample > _current_threshold) {
                _current_threshold = min(sample, MAX_THRESHOLD);
                _stateBeat = BeatdetectorStateFollowingSlope;
            }

            // Tracking lost, resetting
            if (app_timer_cnt_diff_compute(
                    app_timer_cnt_get(),
                    INVALID_READOUT_DELAY_MS + _time_last_beat_ticks) > 0) {
                _beatPeriod = 0;
                _lastMaxValue = 0;
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
                _lastMaxValue = sample;
                _stateBeat = BeatdetectorStateMasking;
                uint32_t delta = app_timer_cnt_get() - _time_last_beat_ticks;
                if (delta) {
                    const float alpha = 0.15;
                    _beatPeriod = alpha * delta + (1 - alpha) * _beatPeriod;
                }

                _time_last_beat_ticks = app_timer_cnt_get();
            } else {
                _stateBeat = BeatdetectorStateFollowingSlope;
            }
            break;

        case BeatdetectorStateMasking:
            if (app_timer_cnt_get() - _time_last_beat_ticks >
                MASKING_HOLDOFF_TICKS) {
                _stateBeat = BeatdetectorStateWaiting;
            }
            beatDetectorDecreaseThreshold();
            break;
    }

    return beat_detected;
}

static void process_new_sample(uint32_t sample) {
    float filtered_sample = high_pass_filter((float)sample);
    filtered_sample = butterworth_filter(filtered_sample);
    const bool beat_detected = detect_heart_beat(filtered_sample);
    if (beat_detected) {
        NRF_LOG_DEBUG("There is an available bpm: %d.", get_available_bpm());
    }
}

void handle_heart_rate_interruption(void) {
    const uint32_t sample = read_hr_sample();
    NRF_LOG_DEBUG("It processes the sample signal to get bpm's.");
    process_new_sample(sample);
}
