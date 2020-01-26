#include "buttons.h"
#include "gpio.h"
#include "events.h"

#include "nrf_log.h"

#include <stdint.h>

enum ButtonPin {
    LeftButtonPin = 31,
    RightButtonPin = 14,
};
typedef uint8_t ButtonPin;

static void left_button_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    set_event(LeftButtonInterruptionEvt);
}

static void right_button_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    set_event(RightButtonInterruptionEvt);
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