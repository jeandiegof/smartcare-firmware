#include "emergency_chars.h"
#include "utils.h"

#include <string.h>

uint32_t add_emergency_status_char(ble_emergency_service_t *p_emergency,
                                   const ble_emergency_service_init_t *p_emergency_init) {
    ble_gatts_attr_md_t cccd_md = get_characteristic_configuration_descriptor_metadata();
    ble_gatts_char_md_t char_md = get_characteristic_metadata(&cccd_md, false, true, true);
    ble_gatts_attr_md_t attr_md =
        get_attribute_metadata(p_emergency_init->emergency_status_char_attr_md.read_perm,
                               p_emergency_init->emergency_status_char_attr_md.write_perm);

    ble_uuid_t ble_uuid =
        get_ble_uuid_structure(p_emergency->uuid_type, EMERGENCY_STATUS_CHAR_UUID);
    ble_gatts_attr_t attr_char_value = get_u8_attribute_structure(&ble_uuid, &attr_md);

    return sd_ble_gatts_characteristic_add(p_emergency->service_handle, &char_md, &attr_char_value,
                                           &p_emergency->emergency_status_handles);
}
