#include "adxl345.h"

#include "app_error.h"
#include "boards.h"
#include "nrf_drv_twi.h"
#include "nrf_log.h"
#include "boards.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log_ctrl.h"
#include <nrfx_gpiote.h>
#include "nrf.h"
#include <stdint.h>

enum ADXL345Registers
{
    DEVID = 0x00,
    THRESH_TAP = 0x1D,
    OFSX = 0x1E,
    OFSY = 0x1F,
    OFSZ = 0x20,
    DUR = 0x21,
    LATENT = 0x22,
    WINDOW = 0x23,
    THRESH_ACT = 0x24,
    THRESH_INACT = 0x25,
    TIME_INACT = 0x26,
    ACT_INACT_CTL = 0x27,
    THRESH_FF = 0x28,
    TIME_FF = 0x29,
    TAP_AXES = 0x2A,
    ACT_TAP_STATUS = 0x2B,
    BW_RATE = 0x2C,
    POWER_CTL = 0x2D,
    INT_ENABLE = 0x2E,
    INT_MAP = 0x2F,
    INT_SOURCE = 0x30,
    DATA_FORMAT = 0x31,
    DATAX0 = 0x32,
    DATAX1 = 0x33,
    DATAY0 = 0x34,
    DATAY1 = 0x35,
    DATAZ0 = 0x36,
    DATAZ1 = 0x37,
    FIFO_CTL = 0x38,
    FIFO_STATUS = 0x39,
};
typedef uint8_t ADXL345Registers;

#define ADXL345_ADDR 0x53U

// GPIOs
#define SCL_PIN     27
#define SDA_PIN     26
#define INT1_PIN    02

// TWI
#define TWI_INSTANCE_ID 0
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

// local variables
static volatile uint8_t _fall_count = 0;
static volatile uint8_t _interrupt_count = 0;
static uint8_t m_samples[6] = {0};

// local functions
static void twi_handler(nrf_drv_twi_evt_t const *p_event, void *p_context);
static void on_adxl345_data();
static void write_data(uint8_t *data, uint8_t size);
//void read_data(uint8_t reg, uint8_t *data, uint8_t size);
static void setup_free_fall_mode();
static void enable_interrupts();
static void enable_gpio_interrupt();
static void wait_for_ongoing_transaction(void);
void on_int2_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void adxl345_init(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_adxl345_config = {
        .scl = SCL_PIN,
        .sda = SDA_PIN,
        .frequency = NRF_DRV_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init = false};

    err_code = nrf_drv_twi_init(&m_twi, &twi_adxl345_config, NULL, NULL);

    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

void adxl345_setup(void)
{
    uint8_t reg[2] = {POWER_CTL, 0x08};
    write_data(reg, sizeof(reg));
}

void adxl345_start_free_fall_mode(void)
{
    setup_free_fall_mode();
    enable_gpio_interrupt();
    enable_interrupts();
}

const uint8_t * adxl345_request_axis_data(void)
{
    read_data(DATAX0, m_samples, 6);
    return m_samples;
}

uint8_t adxl345_fall_count(void) {
    return _fall_count;
} 

static void write_data(uint8_t *data, uint8_t size)
{
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, ADXL345_ADDR, data, size, true);
    APP_ERROR_CHECK(err_code);
}

void read_data(uint8_t reg, uint8_t *data, uint8_t size)
{
    ret_code_t err_code1 = nrf_drv_twi_tx(&m_twi, ADXL345_ADDR, &reg, 1, true);
    APP_ERROR_CHECK(err_code1);

    ret_code_t err_code = nrf_drv_twi_rx(&m_twi, ADXL345_ADDR, (uint8_t*)data, size);
    APP_ERROR_CHECK(err_code);
}

static void setup_free_fall_mode()
{
    // values taken from https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf
    uint8_t free_fall_threshold[2] = {THRESH_FF, 0x05};
    write_data(free_fall_threshold, sizeof(free_fall_threshold));

    uint8_t free_fall_time[2] = {TIME_FF, 0x02};
    write_data(free_fall_time, sizeof(free_fall_time));
}

static void enable_interrupts()
{
    // map only free fall interrupt to INT1 pin
    uint8_t interrupt_map[2] = {INT_MAP, 0b11111011};
    write_data(interrupt_map, sizeof(interrupt_map));

    // enable free fall interupt
    uint8_t interrupt_enable[2] = {INT_ENABLE, 0b00000100};
    write_data(interrupt_enable, sizeof(interrupt_enable));
}

static void enable_gpio_interrupt()
{
    ret_code_t err_code;

    if (!nrf_drv_gpiote_is_init()) {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    if (!nrf_drv_gpiote_in_is_set(INT1_PIN)) {
        err_code = nrf_drv_gpiote_in_init(INT1_PIN, &in_config, on_int2_interrupt);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(INT1_PIN, true);
    } else { 
        NRF_LOG_INFO("\r\nGPIO ALREADY USED");
    }
}

// callbacks

void on_int2_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    _interrupt_count++;

    uint8_t interrupt_source = 0;
    read_data(INT_SOURCE, &interrupt_source, sizeof(interrupt_source));
    NRF_LOG_INFO("interrupt source: %02x", interrupt_source);
    if (interrupt_source & 0b00000100)
    {
        _fall_count++;
    }
}
