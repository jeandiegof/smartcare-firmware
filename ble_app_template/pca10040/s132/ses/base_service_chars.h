#pragma once

#include "base_service.h"

#include <stdint.h>

uint32_t add_base_service_button_char(ble_base_service_t *p_service,
                                   const ble_base_service_init_t *p_service_init);
uint32_t update_base_service_button_char(ble_base_service_t *p_service, uint8_t value);
