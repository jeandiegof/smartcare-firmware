#include "gpio.h"

#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"

void gpio_init(void) {
    ret_code_t err_code;

    if (!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

}

void enable_gpio_interrupt(uint8_t pin, interrupt_callback callback, nrf_gpiote_polarity_t sense, nrf_gpio_pin_pull_t pull) {
    if (callback == NULL) {
        NRF_LOG_ERROR("\r\nInterrupt callback is NULL.");
        return;
    }

    nrf_drv_gpiote_in_config_t interrupt_config = {
        .sense = sense,            
        .pull = pull,
        .is_watcher = false,
        .hi_accuracy = true,
        .skip_gpio_setup = false,
    };

    if (!nrf_drv_gpiote_in_is_set(pin)) {
        ret_code_t err_code = nrf_drv_gpiote_in_init(pin, &interrupt_config, callback);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(pin, true);
    } else {
        NRF_LOG_INFO("\r\nGPIO already used.");
    }
}
