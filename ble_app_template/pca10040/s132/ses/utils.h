#pragma once

#include "ble_srv_common.h"

#include <stdio.h>

enum CharacteristicProperty {
    Read = 0b00000001,
    Write = 0b00000010,
    Notify = 0b00000100,
};
typedef uint8_t CharacteristicProperty;

uint32_t add_vendor_specific_base_uuid(ble_uuid128_t base_uuid, uint8_t* uuid_type);
uint32_t add_service(uint16_t uuid, uint8_t uuid_type, uint16_t* handle);
ble_gatts_attr_md_t get_characteristic_configuration_descriptor_metadata(void);
ble_gatts_char_md_t get_characteristic_metadata(ble_gatts_attr_md_t* cccd_md, uint8_t properties);
ble_gatts_attr_md_t get_attribute_metadata(ble_gap_conn_sec_mode_t read_permission,
                                           ble_gap_conn_sec_mode_t write_permission);
ble_uuid_t get_ble_uuid_structure(uint8_t type, uint16_t uuid);
ble_gatts_attr_t get_u8_attribute_structure(ble_uuid_t* ble_uuid, ble_gatts_attr_md_t* attr_md);
ble_gatts_value_t get_gatts_value_structure(uint8_t *value, uint8_t size);
uint32_t notify_value(uint16_t connection_handle, uint16_t value_handle, ble_gatts_value_t* gatts_value);
bool is_connected(uint16_t handle);
