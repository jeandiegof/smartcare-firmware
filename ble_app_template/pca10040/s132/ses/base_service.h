#pragma once

#include "ble_srv_common.h"

#include <stdint.h>
#include <stdbool.h>

#define BASE_SERVICE_UUID_BASE                                                               \
    {                                                                                             \
        0x09, 0x06, 0xb0, 0x7c, 0x66, 0x54, 0x20, 0xb5, 0x5a, 0x4d, 0x2c, 0x39, 0x12, 0xaf, 0xce, \
            0x45                                                                                  \
    }

#define BASE_SERVICE_UUID 0x1500

#define BLE_BASE_SERVICE_DEF(_name)          \
    static ble_base_service_t _name; \
    NRF_SDH_BLE_OBSERVER(_name##_obs, BLE_HRS_BLE_OBSERVER_PRIO, on_ble_base_service_evt, &_name)

typedef struct {
    uint8_t initial_button;
    ble_srv_cccd_security_mode_t button_char_attr_md;
    ble_srv_cccd_security_mode_t bpm_char_attr_md;
    ble_srv_cccd_security_mode_t arrhythmia_char_attr_md;
    ble_srv_cccd_security_mode_t fall_detection_char_attr_md;
    ble_srv_cccd_security_mode_t battery_char_attr_md;
} ble_base_service_init_t;

typedef struct {
    uint16_t service_handle;
    ble_gatts_char_handles_t button_handles;
    ble_gatts_char_handles_t bpm_handles;
    ble_gatts_char_handles_t arrhythmia_handles;
    ble_gatts_char_handles_t fall_detection_handles;
    ble_gatts_char_handles_t battery_handles;
    uint16_t conn_handle;
    uint8_t uuid_type;
} ble_base_service_t;

uint32_t ble_base_service_init(ble_base_service_t *service,
                            const ble_base_service_init_t *init);
void on_ble_base_service_evt(ble_evt_t const *p_ble_evt, void *p_context);
uint32_t ble_base_service_button_update(ble_base_service_t *p_service, uint8_t value);
uint32_t ble_base_service_bpm_update(ble_base_service_t *p_service, uint8_t value);
uint32_t ble_base_service_arrhythmia_update(ble_base_service_t *p_service, uint8_t value);
uint32_t ble_base_service_fall_detection_update(ble_base_service_t *p_service, uint8_t value);
uint32_t ble_base_service_battery_update(ble_base_service_t *p_service, uint8_t value);
