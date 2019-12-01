// project includes
#include "battery/telemetry.h"
#include "battery/adc.h"

// nordic includes
#include "app_timer.h"
#include "app_error.h"

// std includes

// local defines
#define BATTERY_LEVEL_MEASUREMENT_INTERVAL APP_TIMER_TICKS(10000)  // ms

// modules
APP_TIMER_DEF(m_battery_timer);

static void battery_telemetry_timeout();
static void battery_timer_init();

void battery_telemetry_init(battery_update_cb battery_update_fn) {
    battery_timer_init();
    adc_init(battery_update_fn);
}

void battery_telemetry_start() {
    ret_code_t err_code = NRF_SUCCESS;
    err_code = app_timer_start(m_battery_timer, BATTERY_LEVEL_MEASUREMENT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

void battery_telemetry_stop() {
    ret_code_t err_code = NRF_SUCCESS;
    err_code = app_timer_stop(m_battery_timer);
    APP_ERROR_CHECK(err_code);
}

static void battery_telemetry_timeout() { request_battery_level(); }

static void battery_timer_init() {
    ret_code_t err_code = NRF_SUCCESS;
    // TODO: app_timer_init needs to be called before creating the timer
    // err_code = app_timer_init();
    // APP_ERROR_CHECK(err_code);

    err_code =
        app_timer_create(&m_battery_timer, APP_TIMER_MODE_REPEATED, battery_telemetry_timeout);
    APP_ERROR_CHECK(err_code);
}
