#pragma once

#include "ble_srv_common.h"

#include <stdint.h>
#include <stdbool.h>

#define ACCELEROMETER_SERVICE_UUID_BASE {\
    0xa2, 0x86, 0x4e, 0x8b, 0x35, 0xe5, 0x0f, 0x8c, 0x94, 0x41, 0xac, 0xb0, 0x00, 0x88, 0x20, 0xf7\
}

#define ACCELEROMETER_SERVICE_UUID 0x1500
#define ACCELEROMETER_FREE_FALL_CHAR_UUID 0x1501

#define BLE_ACCELEROMETER_DEF(_name)          \
    static ble_accelerometer_service_t _name; \
    NRF_SDH_BLE_OBSERVER(_name##_obs, BLE_HRS_BLE_OBSERVER_PRIO, on_ble_accelerometer_evt, &_name)

typedef struct {
    uint8_t initial_accelerometer_status;
    ble_srv_cccd_security_mode_t free_fall_char_attr_md;
} ble_accelerometer_service_init_t;

typedef struct {
    uint16_t service_handle;
    ble_gatts_char_handles_t fall_detection_handles;
    ble_gatts_char_handles_t samples_handles;
    uint16_t conn_handle;
    uint8_t uuid_type;
} ble_accelerometer_service_t;

uint32_t ble_accelerometer_init(ble_accelerometer_service_t *service,
                            const ble_accelerometer_service_init_t *init);
void on_ble_accelerometer_evt(ble_evt_t const *p_ble_evt, void *p_context);
uint32_t ble_accelerometer_free_fall_update(ble_accelerometer_service_t *p_service, uint8_t value);
