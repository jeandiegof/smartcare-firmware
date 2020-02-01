#include "heart_rate.h"
#include "app_timer.h"
#include <stdbool.h>
#include "nrf_drv_systick.h"
#include "nrf_log.h"
#include <stdint.h>

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

#define INIT_HOLDOFF_TICKS APP_TIMER_TICKS(2000)   // how long to wait before counting
#define MASKING_HOLDOFF_TICKS APP_TIMER_TICKS(40)  // non-retriggerable window after beat detection
#define MIN_THRESHOLD -1.0                         // 20
#define MAX_THRESHOLD 800.0
#define STEP_RESILIENCY 20.0          // maximum negative jump that triggers the beat edge
#define THRESHOLD_FALLOFF_TARGET 0.3  // thr chasing factor of the max value when beat
#define THRESHOLD_DECAY_FACTOR 0.99   // thr chasing factor when no beat
#define INVALID_READOUT_DELAY_TICKS APP_TIMER_TICKS(2000)  // in ms, no-beat time to cause a reset

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
static float _lastMaxValue = 0.0;

void heart_rate_start(void) {
#ifdef MAX30100
    max30100_init();
    max30100_setup();
#else
    max30101_init();
    max30101_setup();
#endif
    _last_time_measured_ticks = app_timer_cnt_get();
    NRF_LOG_INFO("last measured time %d", _last_time_measured_ticks);
}

static float high_pass_filter(float sample) {
    //    static float _previous_sample = 0.0;
    //    static float _previous_retult = 0.0;
    //    _previous_retult = sample - _previous_sample * POLE;
    //    _previous_sample = sample;
    //    return _previous_retult;

    //    static float _average_estimation = 0.0;
    //    _average_estimation = sample * POLE + _average_estimation * (1.0 - POLE);
    //    return sample - _average_estimation;
    return sample;
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
    if (_lastMaxValue > 0 && _beat_period_ms > 0) {
        _current_threshold -=
            _lastMaxValue * (1 - THRESHOLD_FALLOFF_TARGET) / (_beat_period_ms / SAMPLING_PERIOD);
    } else {
        _current_threshold *= THRESHOLD_DECAY_FACTOR;
    }

    if (_current_threshold < MIN_THRESHOLD) {
        _current_threshold = MIN_THRESHOLD;
    }
}

static float get_available_bpm(void) {
    if (_beat_period_ms != 0) {
        return 1.0 / _beat_period_ms * 1000.0 * 60.0;
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
            if (app_timer_cnt_diff_compute(app_timer_cnt_get(), _last_time_measured_ticks) >
                INIT_HOLDOFF_TICKS) {
                _last_time_measured_ticks = app_timer_cnt_get();
                _stateBeat = BeatdetectorStateWaiting;
            }
            break;

        case BeatdetectorStateWaiting:
            // NRF_LOG_INFO("3 %d, %d", (int)sample, (int)_current_threshold);
            if (sample > _current_threshold) {
                // NRF_LOG_INFO("4");
                _current_threshold = min(sample, MAX_THRESHOLD);
                _stateBeat = BeatdetectorStateFollowingSlope;
            }

            if (app_timer_cnt_diff_compute(
                    app_timer_cnt_diff_compute(app_timer_cnt_get(), _time_last_beat_ticks),
                    _last_time_measured_ticks) > INVALID_READOUT_DELAY_TICKS) {
                // NRF_LOG_INFO("5");
                _beat_period_ms = 0.0;
                _lastMaxValue = 0.0;
                _last_time_measured_ticks = app_timer_cnt_get();
            }

            beatDetectorDecreaseThreshold();
            break;

        case BeatdetectorStateFollowingSlope:
            // NRF_LOG_INFO("6 %d, %d", (int)sample, (int)_current_threshold);
            if (sample < _current_threshold) {
                // NRF_LOG_INFO("6.1");
                _stateBeat = BeatdetectorStateMaybeDetected;
            } else {
                // NRF_LOG_INFO("6.2");
                _current_threshold = (float)min(sample, MAX_THRESHOLD);
            }
            break;

        case BeatdetectorStateMaybeDetected:
            // NRF_LOG_INFO("7 %d, %d", (int)sample, (int)_current_threshold);

            if ((int)sample + (int)STEP_RESILIENCY < (int)_current_threshold) {
                beat_detected = true;
                _lastMaxValue = sample;
                _stateBeat = BeatdetectorStateMasking;
                const uint32_t delta_ticks =
                    app_timer_cnt_diff_compute(app_timer_cnt_get(), _time_last_beat_ticks);
                if (delta_ticks) {
                    const float alpha = 0.2;
                    _beat_period_ms =
                        (alpha * (float)TICKS_TO_MS(delta_ticks) + (1.0 - alpha) * _beat_period_ms);
                }
                _time_last_beat_ticks = app_timer_cnt_get();
            } else {
                _stateBeat = BeatdetectorStateFollowingSlope;
            }
            break;

        case BeatdetectorStateMasking:
            if (app_timer_cnt_diff_compute(
                    app_timer_cnt_diff_compute(app_timer_cnt_get(), _time_last_beat_ticks),
                    _last_time_measured_ticks) > MASKING_HOLDOFF_TICKS) {
                _stateBeat = BeatdetectorStateWaiting;
                _last_time_measured_ticks = app_timer_cnt_get();
            }
            beatDetectorDecreaseThreshold();
            break;
    }

    return beat_detected;
}

static void process_new_sample(float sample) {
    const float filtered_sample = filter(sample);
    //    NRF_LOG_INFO("%d", (int)filtered_sample);
    const bool beat_detected = detect_heart_beat(filtered_sample);

    if (beat_detected) {
        NRF_LOG_INFO("There is an available bpm: %d.", (int)get_available_bpm());
    }
}

void handle_heart_rate_interruption(void) {
    //    wait_for_heart_rate_sample();
    const float sample = -1.0 * (float)read_hr_sample();
    process_new_sample(sample);
}
