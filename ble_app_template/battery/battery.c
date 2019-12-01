// project includes
#include "battery/battery.h"
#include "battery/telemetry.h"

// nordic includes
#include "nrf_log.h"
#include "app_error.h"
#include "ble_bas.h"

// std includes
#include <string.h>

BLE_BAS_DEF(m_bas);

static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt);
static void on_battery_level_update(uint8_t battery_level_percentage);

void bas_init() {
    ble_bas_init_t bas_init;
    memset(&bas_init, 0, sizeof(bas_init));

    bas_init.evt_handler = on_bas_evt;
    bas_init.support_notification = true;
	bas_init.p_report_ref = NULL;
    bas_init.initial_batt_level = 100;

	bas_init.bl_rd_sec = SEC_OPEN;
	bas_init.bl_cccd_wr_sec = SEC_OPEN;
	bas_init.bl_report_rd_sec = SEC_OPEN;

    const ret_code_t err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    battery_telemetry_init(&on_battery_level_update);
}

static void on_bas_evt(ble_bas_t* p_bas, ble_bas_evt_t* p_evt) {
    switch (p_evt->evt_type)
    {
        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
            NRF_LOG_INFO("Battery notifications enabled.");
            battery_telemetry_start();
            break;
        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
            NRF_LOG_INFO("Battery notifications disabled.");
            battery_telemetry_stop();
            break;

        default:
            // No implementation needed.
            break;
    }
}

static void on_battery_level_update(uint8_t battery_level_percentage) {
    NRF_LOG_INFO("Updating battery");
    uint32_t err_code = ble_bas_battery_level_update(&m_bas, battery_level_percentage, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
        APP_ERROR_HANDLER(err_code);
    }
}