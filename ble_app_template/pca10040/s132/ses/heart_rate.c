#include "nrf_log.h"
#include "heart_rate.h"
#ifdef MAX30100
#include "max30100.h"
#else
#include "max30101.h"
#endif

void heart_rate_start(void) {
#ifdef MAX30100
    max30100_init();
    max30100_setup();
#else
    max30101_init();
    max30101_setup();
#endif
}

void handle_heart_rate_interruption(void) {
    const uint32_t sample = read_hr_sample();
    NRF_LOG_DEBUG("It processes the sample signal to get bpm's.");
}
