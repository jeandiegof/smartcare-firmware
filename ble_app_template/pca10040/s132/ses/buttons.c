#include "buttons.h"
#include "gpio.h"
#include "events.h"

#include "nrf_drv_systick.h"
#include "nrf_log.h"

#include <stdint.h>

#define BUTTON_DEBOUCING_TIME_US 50000

enum ButtonPin {
    LeftButtonPin = 15, // 31
    RightButtonPin = 14,
};
typedef uint8_t ButtonPin;

static nrf_drv_systick_state_t left_button_last_click;
static nrf_drv_systick_state_t right_button_last_click;

static void left_button_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    if (nrf_drv_systick_test(&left_button_last_click, BUTTON_DEBOUCING_TIME_US)) {
        nrf_drv_systick_get(&left_button_last_click);
        set_event(LeftButtonInterruptionEvt);
    }
}

static void right_button_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    if (nrf_drv_systick_test(&right_button_last_click, BUTTON_DEBOUCING_TIME_US)) {
        nrf_drv_systick_get(&right_button_last_click);
        set_event(RightButtonInterruptionEvt);
    }
}

void handle_left_button_interruption(void) {
    NRF_LOG_INFO("Left button pressed");
    // TODO: notify using notify_emergency from https://github.com/jeandiegof/smartcare-firmware/pull/7
}

void handle_right_button_interruption(void) {
    NRF_LOG_INFO("Right button pressed");
    // TODO: notify using notify_emergency from https://github.com/jeandiegof/smartcare-firmware/pull/7
}

void buttons_start(void) {
    APP_ERROR_CHECK(enable_gpio_interrupt(LeftButtonPin, left_button_callback, NRF_GPIOTE_POLARITY_HITOLO, NRF_GPIO_PIN_PULLUP));
    APP_ERROR_CHECK(enable_gpio_interrupt(RightButtonPin, right_button_callback, NRF_GPIOTE_POLARITY_HITOLO, NRF_GPIO_PIN_PULLUP));
}