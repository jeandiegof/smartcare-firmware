#pragma once

#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include <stdint.h>

typedef void (*interrupt_callback)(nrf_drv_gpiote_pin_t, nrf_gpiote_polarity_t);

void gpio_init(void);
void enable_gpio_interrupt(uint8_t pin, interrupt_callback callback, nrf_gpiote_polarity_t sense, nrf_gpio_pin_pull_t pull);