#include "battery/adc.h"

#include "nrf_drv_saadc.h"
#include "nrf_log.h"

// Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS 600
// The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of
// conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_PRE_SCALING_COMPENSATION 6
// Typical forward voltage drop of the diode.
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS 270
// Maximum digital value for 10-bit ADC conversion.
#define ADC_RES_10BIT 1024

#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE)*ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

static void adc_event_handler(nrf_drv_saadc_evt_t const *p_event);

static nrf_saadc_value_t _adc_buf[2];
static battery_update_cb _battery_update_cb;

void adc_init(battery_update_cb battery_update_cb) {
    ret_code_t err_code = nrf_drv_saadc_init(NULL, adc_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_saadc_channel_config_t config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
    err_code = nrf_drv_saadc_channel_init(0, &config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&_adc_buf[0], 1);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&_adc_buf[1], 1);
    APP_ERROR_CHECK(err_code);

    _battery_update_cb = battery_update_cb;
}

void request_battery_level(void) {
    const ret_code_t err_code = nrf_drv_saadc_sample();
    APP_ERROR_CHECK(err_code);
}

static void adc_event_handler(nrf_drv_saadc_evt_t const *p_event) {
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {
        const nrf_saadc_value_t adc_result = p_event->data.done.p_buffer[0];
        const uint32_t err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 1);
        APP_ERROR_CHECK(err_code);

        const uint16_t batt_lvl_in_milli_volts =
            ADC_RESULT_IN_MILLI_VOLTS(adc_result) + DIODE_FWD_VOLT_DROP_MILLIVOLTS;
        const uint8_t percentage_batt_lvl = battery_level_in_percent(batt_lvl_in_milli_volts);

        if (_battery_update_cb) {
            _battery_update_cb(percentage_batt_lvl);
        }
    }
}
