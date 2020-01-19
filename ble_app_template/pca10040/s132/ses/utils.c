#include "utils.h"

#include "ble_srv_common.h"

#include <string.h>

uint32_t add_vendor_specific_base_uuid(ble_uuid128_t base_uuid, uint8_t* uuid_type) {
    return sd_ble_uuid_vs_add(&base_uuid, uuid_type);
}

uint32_t add_service(uint16_t uuid, uint8_t uuid_type, uint16_t* handle) {
    ble_uuid_t ble_uuid = get_ble_uuid_structure(uuid_type, uuid);
    return sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, handle);
}

ble_gatts_attr_md_t get_characteristic_configuration_descriptor_metadata(void) {
    ble_gatts_attr_md_t cccd_md;

    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    return cccd_md;
}

ble_gatts_char_md_t get_characteristic_metadata(ble_gatts_attr_md_t* cccd_md, bool read, bool write,
                                                bool notify) {
    ble_gatts_char_md_t char_md;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = read;
    char_md.char_props.write = write;
    char_md.char_props.notify = notify;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf = NULL;
    char_md.p_user_desc_md = NULL;
    char_md.p_cccd_md = (notify) ? cccd_md : NULL;
    char_md.p_sccd_md = NULL;

    return char_md;
}

ble_gatts_attr_md_t get_attribute_metadata(ble_gap_conn_sec_mode_t read_permission,
                                           ble_gap_conn_sec_mode_t write_permission) {
    ble_gatts_attr_md_t attr_md;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm = read_permission;
    attr_md.write_perm = write_permission;
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 0;

    return attr_md;
}

ble_uuid_t get_ble_uuid_structure(uint8_t type, uint16_t uuid) {
    return (ble_uuid_t){.type = type, .uuid = uuid};
}

ble_gatts_attr_t get_u8_attribute_structure(ble_uuid_t* ble_uuid, ble_gatts_attr_md_t* attr_md) {
    ble_gatts_attr_t attr_char_value;

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid = ble_uuid;
    attr_char_value.p_attr_md = attr_md;
    attr_char_value.init_len = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = sizeof(uint8_t);

    return attr_char_value;
}

ble_gatts_value_t get_gatts_value_structure(uint8_t *value, uint8_t size) {
    ble_gatts_value_t gatts_value = {0};

    gatts_value.len = size;
    gatts_value.offset = 0;
    gatts_value.p_value = value;

    return gatts_value;
}

uint32_t notify_value(uint16_t connection_handle, uint16_t value_handle, ble_gatts_value_t* gatts_value) {
    ble_gatts_hvx_params_t hvx_params = {0};

    hvx_params.handle = value_handle;
    hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = gatts_value->offset;
    hvx_params.p_len = &gatts_value->len;
    hvx_params.p_data = gatts_value->p_value;

    return sd_ble_gatts_hvx(connection_handle, &hvx_params);
}

bool is_connected(uint16_t handle) {
    return handle != BLE_CONN_HANDLE_INVALID;
}
