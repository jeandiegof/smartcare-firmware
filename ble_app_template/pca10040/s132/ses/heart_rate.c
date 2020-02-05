#include "heart_rate.h"
#include "services.h"
#include "app_timer.h"
#include <stdbool.h>
#include "nrf_drv_systick.h"
#include "nrf_log.h"
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

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

#define POLE (float)0.01

#define TICKS_TO_MS(ticks)                                                        \
    ((uint32_t)ROUNDED_DIV(ticks * (1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1)), \
                           (uint64_t)APP_TIMER_CLOCK_FREQ))

#define INIT_HOLDOFF_TICKS APP_TIMER_TICKS(2000)    // how long to wait before counting
#define MASKING_HOLDOFF_TICKS APP_TIMER_TICKS(200)  // non-retriggerable window after beat detection
#define MIN_THRESHOLD -300.0                        // 20
#define MAX_THRESHOLD 800.0
#define STEP_RESILIENCY 30.0           // maximum negative jump that triggers the beat edge
#define THRESHOLD_FALLOFF_TARGET 0.3   // thr chasing factor of the max value when beat
#define THRESHOLD_DECAY_FACTOR 0.99    // thr chasing factor when no beat
#define INVALID_READOUT_DELAY_MS 1500  // no-beat time to cause a reset

typedef enum {
    BeatdetectorStateInit,
    BeatdetectorStateWaiting,
    BeatdetectorStateFollowingSlope,
    BeatdetectorStateMaybeDetected,
    BeatdetectorStateMasking,
} BeatDetectorState;

static float _current_threshold = MIN_THRESHOLD;
static uint32_t _last_time_measured_ticks = 0;
static float _beat_period_ms = 0.0;
static float _last_mat_sample_value = 0.0;
static bool _first_peak = true;

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

#define NZEROS 4
#define NPOLES 4
#define GAIN 1.750284672e+01

static float filter(float sample) {
    static float _xv[NZEROS + 1] = {0.0};
    static float _yv[NPOLES + 1] = {0.0};

    _xv[0] = _xv[1];
    _xv[1] = _xv[2];
    _xv[2] = _xv[3];
    _xv[3] = _xv[4];
    _xv[4] = sample / GAIN;
    _yv[0] = _yv[1];
    _yv[1] = _yv[2];
    _yv[2] = _yv[3];
    _yv[3] = _yv[4];
    _yv[4] = (_xv[0] + _xv[4]) - 2 * _xv[2] + (-0.4504454301 * _yv[0]) + (2.0825733172 * _yv[1]) +
             (-3.7926844229 * _yv[2]) + (3.1594633021 * _yv[3]);

    return _yv[4];
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

#define SAMPLING_PERIOD (1000 / 50)

static void beatDetectorDecreaseThreshold(void) {
    // When a valid beat rate readout is present, target the
    if (_last_mat_sample_value > 0 && _beat_period_ms > 0) {
        _current_threshold -= _last_mat_sample_value * (1 - THRESHOLD_FALLOFF_TARGET) /
                              (_beat_period_ms / SAMPLING_PERIOD);
    } else {
        _current_threshold *= THRESHOLD_DECAY_FACTOR;
    }

    if (_current_threshold < MIN_THRESHOLD) {
        _current_threshold = MIN_THRESHOLD;
    }
}

static float get_available_bpm(void) {
    if (_beat_period_ms != 0) {
        return (60.0 / (_beat_period_ms / 1000.0));
    } else {
        return 0;
    }
}

static bool detect_heart_beat(float sample) {
    bool beat_detected = false;
    static BeatDetectorState _heart_beat_state = BeatdetectorStateInit;
    static uint32_t _time_last_beat_ticks = 0;

    switch (_heart_beat_state) {
        case BeatdetectorStateInit:
            if (app_timer_cnt_diff_compute(app_timer_cnt_get(), _last_time_measured_ticks) >
                INIT_HOLDOFF_TICKS) {
                _last_time_measured_ticks = app_timer_cnt_get();
                _heart_beat_state = BeatdetectorStateWaiting;
            }
            break;

        case BeatdetectorStateWaiting:
            if (sample > _current_threshold) {
                _current_threshold = min(sample, MAX_THRESHOLD);
                _heart_beat_state = BeatdetectorStateFollowingSlope;
            }

            if (app_timer_cnt_diff_compute(
                    app_timer_cnt_diff_compute(app_timer_cnt_get(), _time_last_beat_ticks),
                    _last_time_measured_ticks) > APP_TIMER_TICKS(INVALID_READOUT_DELAY_MS)) {
                _first_peak = true;
                _last_mat_sample_value = 0.0;
                _last_time_measured_ticks = app_timer_cnt_get();
            }

            beatDetectorDecreaseThreshold();
            break;

        case BeatdetectorStateFollowingSlope:
            if (sample < _current_threshold) {
                _heart_beat_state = BeatdetectorStateMaybeDetected;
            } else {
                _current_threshold = (float)min(sample, MAX_THRESHOLD);
            }
            break;

        case BeatdetectorStateMaybeDetected:
            if (sample + STEP_RESILIENCY < _current_threshold) {
                static uint32_t _previous_beat_detected_at_ms = 0;
                static uint32_t _beat_detected_at_ms = 0;
                const float alpha = 0.3;
                _time_last_beat_ticks = app_timer_cnt_get();
                _first_peak = _first_peak ? false : _first_peak;
                _last_mat_sample_value = sample;
                _heart_beat_state = BeatdetectorStateMasking;
                _previous_beat_detected_at_ms = _beat_detected_at_ms;
                _beat_detected_at_ms = TICKS_TO_MS(app_timer_cnt_get());

                if (_first_peak) {
                    break;
                }

                beat_detected = true;

                const uint32_t peaks_distance_ms =
                    abs(_beat_detected_at_ms - _previous_beat_detected_at_ms);

                if (peaks_distance_ms >= INVALID_READOUT_DELAY_MS) {
                    break;
                }

                _beat_period_ms =
                    alpha * (float)peaks_distance_ms + (1.0 - alpha) * _beat_period_ms;
            } else {
                _heart_beat_state = BeatdetectorStateFollowingSlope;
            }
            break;

        case BeatdetectorStateMasking:
            if (app_timer_cnt_diff_compute(
                    app_timer_cnt_diff_compute(app_timer_cnt_get(), _time_last_beat_ticks),
                    _last_time_measured_ticks) > MASKING_HOLDOFF_TICKS) {
                _heart_beat_state = BeatdetectorStateWaiting;
                _last_time_measured_ticks = app_timer_cnt_get();
            }
            beatDetectorDecreaseThreshold();
            break;
    }

    return beat_detected;
}

static void process_new_sample(float sample) {
    const float filtered_sample = filter(sample);
    const bool beat_detected = detect_heart_beat(filtered_sample);
    if (beat_detected) {
        const uint8_t heart_rate = (uint8_t)get_available_bpm();
        if (heart_rate > 160 || heart_rate < 40) {
            NRF_LOG_INFO("Invalid heart_rate detected %d.", heart_rate);
            return;
        }
        notify_bpm(heart_rate);
        NRF_LOG_INFO("There is an available heart_rate: %d.", heart_rate);
    }
}

void handle_heart_rate_interruption(void) {
    const float sample = -1.0 * (float)read_hr_sample();
    process_new_sample(sample);
}
