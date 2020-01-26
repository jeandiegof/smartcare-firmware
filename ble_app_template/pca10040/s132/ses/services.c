#include "services.h"
#include "base_service.h"
#include "ble_accelerometer.h"

#include "nrf_sdh_ble.h"
#include "sdk_macros.h"

#include <string.h>

BLE_BASE_SERVICE_DEF(_base_service);
BLE_ACCELEROMETER_DEF(_accelerometer_service);

static uint32_t init_base_service(void) {
    ble_base_service_init_t base_init;

    memset(&base_init, 0, sizeof(base_init));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&base_init.base_status_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&base_init.base_status_char_attr_md.write_perm);

    uint32_t err_code = ble_base_service_init(&_base_service, &base_init);
    return err_code;
}

static uint32_t init_accelerometer_service(void) {
    ble_accelerometer_service_init_t accelerometer_init;

    memset(&accelerometer_init, 0, sizeof(accelerometer_init));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&accelerometer_init.free_fall_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&accelerometer_init.free_fall_char_attr_md.write_perm);

    uint32_t err_code = ble_accelerometer_init(&_accelerometer_service, &accelerometer_init);
    return err_code;
}

uint32_t init_smartcare_services(void) { 
    VERIFY_SUCCESS(init_base_service());
    // TODO: for some reason, adding another service was causing a memory error.
    // I've just put everything in the same service for now. We need to fix this.
    // VERIFY_SUCCESS(init_accelerometer_service());
    return NRF_SUCCESS; 
}

uint32_t notify_emergency(uint8_t value) {
    return ble_base_service_button_update(&_base_service, value);
}