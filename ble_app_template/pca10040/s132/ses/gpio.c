#include "gpio.h"

#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_assert.h"

void gpio_init(void) {
    ret_code_t err_code;

    if (!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
}

ret_code_t enable_gpio_interrupt(uint8_t pin, interrupt_callback callback, nrf_gpiote_polarity_t sense, nrf_gpio_pin_pull_t pull) {
    ASSERT(callback != NULL);

    nrf_drv_gpiote_in_config_t interrupt_config = {
        .sense = sense,            
        .pull = pull,
        .is_watcher = false,
        .hi_accuracy = true,
        .skip_gpio_setup = false,
    };

    VERIFY_SUCCESS(nrf_drv_gpiote_in_init(pin, &interrupt_config, callback));
    nrf_drv_gpiote_in_event_enable(pin, true);
    return NRF_SUCCESS;
}
