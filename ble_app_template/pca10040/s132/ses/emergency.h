#pragma once

#include "ble_srv_common.h"

#include <stdint.h>
#include <stdbool.h>

#define EMERGENCY_SERVICE_UUID_BASE                                                               \
    {                                                                                             \
        0x09, 0x06, 0xb0, 0x7c, 0x66, 0x54, 0x20, 0xb5, 0x5a, 0x4d, 0x2c, 0x39, 0x12, 0xaf, 0xce, \
            0x45                                                                                  \
    }

#define EMERGENCY_SERVICE_UUID 0x1500
#define EMERGENCY_STATUS_CHAR_UUID 0x1501

#define BLE_EMERGENCY_DEF(_name)          \
    static ble_emergency_service_t _name; \
    NRF_SDH_BLE_OBSERVER(_name##_obs, BLE_HRS_BLE_OBSERVER_PRIO, on_ble_emergency_evt, &_name)

typedef struct {
    uint8_t initial_emergency_status;
    ble_srv_cccd_security_mode_t emergency_status_char_attr_md;
} ble_emergency_service_init_t;

typedef struct {
    uint16_t service_handle;
    ble_gatts_char_handles_t emergency_status_handles;
    uint16_t conn_handle;
    uint8_t uuid_type;
} ble_emergency_service_t;

uint32_t ble_emergency_init(ble_emergency_service_t *service,
                            const ble_emergency_service_init_t *init);
void on_ble_emergency_evt(ble_evt_t const *p_ble_evt, void *p_context);