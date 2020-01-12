#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

// from random uuid: f364adc9-b000-4042-ba50-05ca45bf8abc (reversed order)
#define CUSTOM_SERVICE_UUID_BASE                           \
    {                                                      \
        0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA,    \
            0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3 \
    }

// values can be chosen randomly
#define CUSTOM_SERVICE_UUID 0x1400
#define CUSTOM_VALUE_CHAR_UUID 0x1401

#define BLE_CUS_DEF(_name)                          \
    static ble_cus_t _name;                         \
    NRF_SDH_BLE_OBSERVER(_name##_obs,               \
                         BLE_HRS_BLE_OBSERVER_PRIO, \
                         ble_cus_on_ble_evt, &_name)

typedef struct
{
    uint8_t initial_custom_value;                           /**< Initial custom value */
    ble_srv_cccd_security_mode_t custom_value_char_attr_md; /**< Initial security level for Custom characteristics attribute */
} ble_cus_init_t;

struct ble_cus_s
{
    uint16_t service_handle;                       /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t custom_value_handles; /**< Handles related to the Custom Value characteristic. */
    uint16_t conn_handle;                          /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t uuid_type;
};

typedef struct ble_cus_s ble_cus_t;

uint32_t ble_cus_init(ble_cus_t *p_cus, const ble_cus_init_t *p_cus_init);
void ble_cus_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context);