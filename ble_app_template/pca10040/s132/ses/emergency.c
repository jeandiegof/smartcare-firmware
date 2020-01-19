#include "emergency.h"
#include "emergency_chars.h"
#include "utils.h"

#include "nrf_log.h"
#include "nrf_assert.h"

#include <string.h>

static void on_connect(ble_emergency_service_t *p_emergency, ble_evt_t const *p_ble_evt);
static void on_disconnect(ble_emergency_service_t *p_emergency, ble_evt_t const *p_ble_evt);

static uint32_t add_characteristics(ble_emergency_service_t *service,
                                    const ble_emergency_service_init_t *init);

uint32_t ble_emergency_init(ble_emergency_service_t *service,
                            const ble_emergency_service_init_t *init) {
    ASSERT(service != NULL && init != NULL);

    service->conn_handle = BLE_CONN_HANDLE_INVALID;

    uint32_t err_code = add_vendor_specific_base_uuid((ble_uuid128_t)EMERGENCY_SERVICE_UUID_BASE,
                                                      &service->uuid_type);
    VERIFY_SUCCESS(err_code);

    err_code = add_service(EMERGENCY_SERVICE_UUID, service->uuid_type, &service->service_handle);
    VERIFY_SUCCESS(err_code);

    err_code = add_characteristics(service, init);
    return err_code;
}

static uint32_t add_characteristics(ble_emergency_service_t *service,
                                    const ble_emergency_service_init_t *init) {
    add_emergency_status_char(service, init);
}

uint32_t ble_emergency_status_update(ble_emergency_service_t *p_service, uint8_t value) {
    update_emergency_status_char(p_service, value);
}

void on_ble_emergency_evt(ble_evt_t const *p_ble_evt, void *p_context) {
    ble_emergency_service_t *p_emergency = (ble_emergency_service_t *)p_context;
    ASSERT(p_emergency != NULL && p_ble_evt != NULL);

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("EMERGENCY CONNECTED");
            on_connect(p_emergency, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("EMERGENCY DISCONNECTED");
            on_disconnect(p_emergency, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

static void on_connect(ble_emergency_service_t *p_emergency, ble_evt_t const *p_ble_evt)
{
    p_emergency->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

static void on_disconnect(ble_emergency_service_t *p_emergency, ble_evt_t const *p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_emergency->conn_handle = BLE_CONN_HANDLE_INVALID;
}