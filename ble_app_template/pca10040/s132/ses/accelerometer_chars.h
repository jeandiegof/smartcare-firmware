#pragma once

#include "ble_accelerometer.h"

#include <stdint.h>

uint32_t add_free_fall_char(ble_accelerometer_service_t *p_service,
                                   const ble_accelerometer_service_init_t *p_service_init);
uint32_t update_free_fall_char(ble_accelerometer_service_t *p_service, uint8_t value);
