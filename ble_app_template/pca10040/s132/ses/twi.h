#pragma once

#include "nrf_drv_twi.h"

#include <stdint.h>

const nrf_drv_twi_t* twi_init(uint8_t scl_pin, uint8_t sda_pin);
void twi_read_data(const nrf_drv_twi_t* twi, uint8_t address, uint8_t reg, uint8_t *data, uint8_t size);
void twi_write_data(const nrf_drv_twi_t* twi, uint8_t address, const uint8_t *data, uint8_t size);