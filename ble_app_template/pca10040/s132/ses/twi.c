#include "twi.h"

#include "app_error.h"
#include "nrf_drv_twi.h"


#define TWI_INSTANCES 2
static uint8_t _twi_index = 0; 
static const nrf_drv_twi_t _twis[TWI_INSTANCES] = {NRF_DRV_TWI_INSTANCE(0), NRF_DRV_TWI_INSTANCE(1)};

const nrf_drv_twi_t* twi_init(uint8_t scl_pin, uint8_t sda_pin) {
    ret_code_t err_code;

    if (_twi_index >= TWI_INSTANCES) {
        return NULL;
    }

    const nrf_drv_twi_config_t twi_config = {
        .scl = scl_pin,
        .sda = sda_pin,
        .frequency = NRF_DRV_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init = false};

    err_code = nrf_drv_twi_init(&_twis[_twi_index], &twi_config, NULL, NULL);

    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&_twis[_twi_index]);

    return &_twis[_twi_index++];
}

void twi_read_data(const nrf_drv_twi_t* twi, uint8_t address, uint8_t reg, uint8_t *data, uint8_t size) {
    ret_code_t err_code = nrf_drv_twi_tx(twi, address, &reg, 1, true);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_twi_rx(twi, address, (uint8_t *)data, size);
    APP_ERROR_CHECK(err_code);
}

void twi_write_data(const nrf_drv_twi_t* twi, uint8_t address, const uint8_t *data, uint8_t size) {
    ret_code_t err_code = nrf_drv_twi_tx(twi, address, data, size, true);
    APP_ERROR_CHECK(err_code);
}