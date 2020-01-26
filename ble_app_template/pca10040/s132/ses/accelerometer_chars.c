#include "accelerometer_chars.h"
#include "utils.h"

#include "sdk_macros.h"

#include <string.h>

uint32_t add_free_fall_char(ble_accelerometer_service_t *p_accelerometer,
                                   const ble_accelerometer_service_init_t *p_accelerometer_init) {
    ble_gatts_attr_md_t cccd_md = get_characteristic_configuration_descriptor_metadata();
    ble_gatts_char_md_t char_md = get_characteristic_metadata(&cccd_md, Notify);
    ble_gatts_attr_md_t attr_md =
        get_attribute_metadata(p_accelerometer_init->free_fall_char_attr_md.read_perm,
                               p_accelerometer_init->free_fall_char_attr_md.write_perm);

    ble_uuid_t ble_uuid =
        get_ble_uuid_structure(p_accelerometer->uuid_type, ACCELEROMETER_FREE_FALL_CHAR_UUID);
    ble_gatts_attr_t attr_char_value = get_u8_attribute_structure(&ble_uuid, &attr_md);

    return sd_ble_gatts_characteristic_add(p_accelerometer->service_handle, &char_md, &attr_char_value,
                                           &p_accelerometer->fall_detection_handles);
}

uint32_t update_free_fall_char(ble_accelerometer_service_t *p_service, uint8_t value) {
    ble_gatts_value_t gatts_value = get_gatts_value_structure(&value, sizeof(value));

    const uint16_t value_handle = p_service->fall_detection_handles.value_handle;
    const uint16_t connection_handle = p_service->conn_handle;

    uint32_t err_code = sd_ble_gatts_value_set(connection_handle, value_handle, &gatts_value);
    VERIFY_SUCCESS(err_code);

    if (is_connected(p_service->conn_handle)) {
        notify_value(connection_handle, value_handle, &gatts_value);
    }
}
