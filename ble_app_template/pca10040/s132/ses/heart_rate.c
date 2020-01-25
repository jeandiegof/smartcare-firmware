#include "heart_rate.h"
#include "nrf_log.h"
#ifdef MAX30100
#include "max30100.h"
#else
#include "max30101.h"
#endif

#define POLE 0.05

void heart_rate_start(void) {
#ifdef MAX30100
    max30100_init();
    max30100_setup();
#else
    max30101_init();
    max30101_setup();
#endif
}

typedef enum {
    Idle,
} PulseDetectorState;

static PulseDetectorState _pulse_detector_state = Idle;

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

static void process_new_sample(uint32_t sample) {
    float filtered_sample = high_pass_filter((float)sample);
    filtered_sample = butterworth_filter(filtered_sample);
}

void handle_heart_rate_interruption(void) {
    const uint32_t sample = read_hr_sample();
    NRF_LOG_DEBUG("It processes the sample signal to get bpm's.");
    process_new_sample(sample);
}
