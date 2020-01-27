#pragma once

#include "base_service.h"

#include <stdint.h>

uint32_t add_base_service_button_char(ble_base_service_t *p_service,
                                   const ble_base_service_init_t *p_service_init);
uint32_t update_base_service_button_char(ble_base_service_t *p_service, uint8_t value);

uint32_t add_base_service_bpm_char(ble_base_service_t *p_service,
                                   const ble_base_service_init_t *p_service_init);
uint32_t update_base_service_bpm_char(ble_base_service_t *p_service, uint8_t value);

uint32_t add_base_service_arrhythmia_char(ble_base_service_t *p_service,
                                   const ble_base_service_init_t *p_service_init);
uint32_t update_base_service_arrhythmia_char(ble_base_service_t *p_service, uint8_t value);

uint32_t add_base_service_fall_detection_char(ble_base_service_t *p_service,
                                   const ble_base_service_init_t *p_service_init);
uint32_t update_base_service_fall_detection_char(ble_base_service_t *p_service, uint8_t value);

uint32_t add_base_service_battery_char(ble_base_service_t *p_service,
                                   const ble_base_service_init_t *p_service_init);
uint32_t update_base_service_battery_char(ble_base_service_t *p_service, uint8_t value);
