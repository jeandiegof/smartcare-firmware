#include "services.h"
#include "base_service.h"

#include "nrf_sdh_ble.h"
#include "sdk_macros.h"

#include <string.h>

BLE_BASE_SERVICE_DEF(_base_service);

static uint32_t init_base_service(void) {
    ble_base_service_init_t base_init;

    memset(&base_init, 0, sizeof(base_init));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&base_init.button_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&base_init.button_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&base_init.bpm_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&base_init.bpm_char_attr_md.write_perm);

    uint32_t err_code = ble_base_service_init(&_base_service, &base_init);
    return err_code;
}

uint32_t init_smartcare_services(void) {
    // TODO: for some reason, adding another service was causing a memory error.
    // I've just put everything in the same service for now. We need to fix this.
    VERIFY_SUCCESS(init_base_service());
    return NRF_SUCCESS;
}

uint32_t notify_emergency(uint8_t value) {
    return ble_base_service_button_update(&_base_service, value);
}

uint32_t notify_bpm(uint8_t value) {
    return ble_base_service_bpm_update(&_base_service, value);
}

uint32_t notify_arrhythmia(uint8_t value) {
    return ble_base_service_arrhythmia_update(&_base_service, value);
}

uint32_t notify_fall_detection(uint8_t value) {
    return ble_base_service_fall_detection_update(&_base_service, value);
}

uint32_t notify_battery_change(uint8_t value) {
    return ble_base_service_battery_update(&_base_service, value);
}