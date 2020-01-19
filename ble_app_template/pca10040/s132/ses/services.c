#include "services.h"
#include "emergency.h"

#include "nrf_sdh_ble.h"
#include "sdk_macros.h"

#include <string.h>

BLE_EMERGENCY_DEF(_emergency_service);

uint32_t init_emergency_service(void) {
    ble_emergency_service_init_t emergency_init;

    memset(&emergency_init, 0, sizeof(emergency_init));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&emergency_init.emergency_status_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&emergency_init.emergency_status_char_attr_md.write_perm);

    uint32_t err_code = ble_emergency_init(&_emergency_service, &emergency_init);
    return err_code;
}

uint32_t init_smartcare_services(void) { 
    VERIFY_SUCCESS(init_emergency_service());
    return NRF_SUCCESS; 
}