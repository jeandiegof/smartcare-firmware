#include "base_service_chars.h"
#include "utils.h"

#include "sdk_macros.h"

#include <string.h>

#define BASE_SERVICE_BUTTON_CHAR_UUID 0x1501
#define BASE_SERVICE_BPM_CHAR_UUID 0x1502
#define BASE_SERVICE_ARRHYTHMIA_CHAR_UUID 0x1503
#define BASE_SERVICE_FALL_DETECTION_CHAR_UUID 0x1504
#define BASE_SERVICE_BATTERY_CHAR_UUID 0x1505

uint32_t add_base_service_button_char(ble_base_service_t *p_base,
                                   const ble_base_service_init_t *p_base_init) {
    ble_gatts_attr_md_t cccd_md = get_characteristic_configuration_descriptor_metadata();
    ble_gatts_char_md_t char_md = get_characteristic_metadata(&cccd_md, Notify);
    ble_gatts_attr_md_t attr_md =
        get_attribute_metadata(p_base_init->button_char_attr_md.read_perm,
                               p_base_init->button_char_attr_md.write_perm);

    ble_uuid_t ble_uuid =
        get_ble_uuid_structure(p_base->uuid_type, BASE_SERVICE_BUTTON_CHAR_UUID);
    ble_gatts_attr_t attr_char_value = get_u8_attribute_structure(&ble_uuid, &attr_md);

    return sd_ble_gatts_characteristic_add(p_base->service_handle, &char_md, &attr_char_value,
                                           &p_base->button_handles);
}

uint32_t update_base_service_button_char(ble_base_service_t *p_service, uint8_t value) {
    ble_gatts_value_t gatts_value = get_gatts_value_structure(&value, sizeof(value));

    const uint16_t value_handle = p_service->button_handles.value_handle;
    const uint16_t connection_handle = p_service->conn_handle;

    uint32_t err_code = sd_ble_gatts_value_set(connection_handle, value_handle, &gatts_value);
    VERIFY_SUCCESS(err_code);

    if (is_connected(p_service->conn_handle)) {
        return notify_value(connection_handle, value_handle, &gatts_value);
    }
    return NRF_ERROR_FORBIDDEN;
}

uint32_t add_base_service_bpm_char(ble_base_service_t *p_base,
                                   const ble_base_service_init_t *p_base_init) {
    ble_gatts_attr_md_t cccd_md = get_characteristic_configuration_descriptor_metadata();
    ble_gatts_char_md_t char_md = get_characteristic_metadata(&cccd_md, Read | Notify);
    ble_gatts_attr_md_t attr_md =
        get_attribute_metadata(p_base_init->bpm_char_attr_md.read_perm,
                               p_base_init->bpm_char_attr_md.write_perm);

    ble_uuid_t ble_uuid =
        get_ble_uuid_structure(p_base->uuid_type, BASE_SERVICE_BPM_CHAR_UUID);
    ble_gatts_attr_t attr_char_value = get_u8_attribute_structure(&ble_uuid, &attr_md);

    return sd_ble_gatts_characteristic_add(p_base->service_handle, &char_md, &attr_char_value,
                                           &p_base->bpm_handles);
}

uint32_t update_base_service_bpm_char(ble_base_service_t *p_service, uint8_t value) {
    ble_gatts_value_t gatts_value = get_gatts_value_structure(&value, sizeof(value));

    const uint16_t value_handle = p_service->bpm_handles.value_handle;
    const uint16_t connection_handle = p_service->conn_handle;

    uint32_t err_code = sd_ble_gatts_value_set(connection_handle, value_handle, &gatts_value);
    VERIFY_SUCCESS(err_code);

    if (is_connected(p_service->conn_handle)) {
        return notify_value(connection_handle, value_handle, &gatts_value);
    }
    return NRF_ERROR_FORBIDDEN;
}

uint32_t add_base_service_arrhythmia_char(ble_base_service_t *p_base,
                                   const ble_base_service_init_t *p_base_init) {
    ble_gatts_attr_md_t cccd_md = get_characteristic_configuration_descriptor_metadata();
    ble_gatts_char_md_t char_md = get_characteristic_metadata(&cccd_md, Notify);
    ble_gatts_attr_md_t attr_md =
        get_attribute_metadata(p_base_init->arrhythmia_char_attr_md.read_perm,
                               p_base_init->arrhythmia_char_attr_md.write_perm);

    ble_uuid_t ble_uuid =
        get_ble_uuid_structure(p_base->uuid_type, BASE_SERVICE_ARRHYTHMIA_CHAR_UUID);
    ble_gatts_attr_t attr_char_value = get_u8_attribute_structure(&ble_uuid, &attr_md);

    return sd_ble_gatts_characteristic_add(p_base->service_handle, &char_md, &attr_char_value,
                                           &p_base->arrhythmia_handles);
}

uint32_t update_base_service_arrhythmia_char(ble_base_service_t *p_service, uint8_t value) {
    ble_gatts_value_t gatts_value = get_gatts_value_structure(&value, sizeof(value));

    const uint16_t value_handle = p_service->arrhythmia_handles.value_handle;
    const uint16_t connection_handle = p_service->conn_handle;

    uint32_t err_code = sd_ble_gatts_value_set(connection_handle, value_handle, &gatts_value);
    VERIFY_SUCCESS(err_code);

    if (is_connected(p_service->conn_handle)) {
        return notify_value(connection_handle, value_handle, &gatts_value);
    }
    return NRF_ERROR_FORBIDDEN;
}

uint32_t add_base_service_fall_detection_char(ble_base_service_t *p_base,
                                   const ble_base_service_init_t *p_base_init) {
    ble_gatts_attr_md_t cccd_md = get_characteristic_configuration_descriptor_metadata();
    ble_gatts_char_md_t char_md = get_characteristic_metadata(&cccd_md, Notify);
    ble_gatts_attr_md_t attr_md =
        get_attribute_metadata(p_base_init->fall_detection_char_attr_md.read_perm,
                               p_base_init->fall_detection_char_attr_md.write_perm);

    ble_uuid_t ble_uuid =
        get_ble_uuid_structure(p_base->uuid_type, BASE_SERVICE_FALL_DETECTION_CHAR_UUID);
    ble_gatts_attr_t attr_char_value = get_u8_attribute_structure(&ble_uuid, &attr_md);

    return sd_ble_gatts_characteristic_add(p_base->service_handle, &char_md, &attr_char_value,
                                           &p_base->fall_detection_handles);
}

uint32_t update_base_service_fall_detection_char(ble_base_service_t *p_service, uint8_t value) {
    ble_gatts_value_t gatts_value = get_gatts_value_structure(&value, sizeof(value));

    const uint16_t value_handle = p_service->fall_detection_handles.value_handle;
    const uint16_t connection_handle = p_service->conn_handle;

    uint32_t err_code = sd_ble_gatts_value_set(connection_handle, value_handle, &gatts_value);
    VERIFY_SUCCESS(err_code);

    if (is_connected(p_service->conn_handle)) {
        return notify_value(connection_handle, value_handle, &gatts_value);
    }
    return NRF_ERROR_FORBIDDEN;
}

uint32_t add_base_service_battery_char(ble_base_service_t *p_base,
                                   const ble_base_service_init_t *p_base_init) {
    ble_gatts_attr_md_t cccd_md = get_characteristic_configuration_descriptor_metadata();
    ble_gatts_char_md_t char_md = get_characteristic_metadata(&cccd_md, Notify);
    ble_gatts_attr_md_t attr_md =
        get_attribute_metadata(p_base_init->battery_char_attr_md.read_perm,
                               p_base_init->battery_char_attr_md.write_perm);

    ble_uuid_t ble_uuid =
        get_ble_uuid_structure(p_base->uuid_type, BASE_SERVICE_BATTERY_CHAR_UUID);
    ble_gatts_attr_t attr_char_value = get_u8_attribute_structure(&ble_uuid, &attr_md);

    return sd_ble_gatts_characteristic_add(p_base->service_handle, &char_md, &attr_char_value,
                                           &p_base->battery_handles);
}

uint32_t update_base_service_battery_char(ble_base_service_t *p_service, uint8_t value) {
    ble_gatts_value_t gatts_value = get_gatts_value_structure(&value, sizeof(value));

    const uint16_t value_handle = p_service->battery_handles.value_handle;
    const uint16_t connection_handle = p_service->conn_handle;

    uint32_t err_code = sd_ble_gatts_value_set(connection_handle, value_handle, &gatts_value);
    VERIFY_SUCCESS(err_code);

    if (is_connected(p_service->conn_handle)) {
        return notify_value(connection_handle, value_handle, &gatts_value);
    }
    return NRF_ERROR_FORBIDDEN;
}
