#pragma once

#include "emergency.h"

#include <stdint.h>

uint32_t add_emergency_status_char(ble_emergency_service_t *p_service,
                                   const ble_emergency_service_init_t *p_service_init);
uint32_t update_emergency_status_char(ble_emergency_service_t *p_service, uint8_t value);
