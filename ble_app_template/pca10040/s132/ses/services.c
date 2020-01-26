#include "services.h"
#include "emergency.h"
#include "ble_accelerometer.h"

#include "nrf_sdh_ble.h"
#include "sdk_macros.h"

#include <string.h>

BLE_EMERGENCY_DEF(_emergency_service);
BLE_ACCELEROMETER_DEF(_accelerometer_service);

static uint32_t init_emergency_service(void) {
    ble_emergency_service_init_t emergency_init;

    memset(&emergency_init, 0, sizeof(emergency_init));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&emergency_init.emergency_status_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&emergency_init.emergency_status_char_attr_md.write_perm);

    uint32_t err_code = ble_emergency_init(&_emergency_service, &emergency_init);
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
    VERIFY_SUCCESS(init_emergency_service());
    VERIFY_SUCCESS(init_accelerometer_service());
    return NRF_SUCCESS; 
}