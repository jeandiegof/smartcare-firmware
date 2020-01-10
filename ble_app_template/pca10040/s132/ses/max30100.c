#include "max30101.h"
#include "twi.h"

#include "app_error.h"
#include "nrf.h"
#include "nrf_drv_twi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_gpiote.h"
#include <nrfx_gpiote.h>

#include <stdint.h>

enum MAX30100Registers {
    // STATUS
    INTERRUPT_STATUS = 0x00,
    INTERRUPT_ENABLE = 0x01,
    // FIFO
    FIFO_WRITE_POINTER = 0x02,
    OVERFLOW_COUNTER = 0x03,
    FIFO_READ_POINTER = 0x04,
    FIFO_DATA_REGISTER = 0x05,
    // CONFIGURATION
    MODE_CONFIG = 0x06,
    SPO2_CONFIG = 0x07,
    LED_CONFIG = 0x09,
    // TEMPERATURE
    TEMP_INTEGER = 0x16,
    TEMP_FRACTION = 0x17,
    // PART ID
    REVISION_ID = 0xFE,
    PART_ID = 0xFF,
};

enum MAX30100Pins {
    SDA_PIN = 02,
    SCL_PIN = 03,
    INT_PIN = 04,
};

enum MAX30100Modes {
    HR_ONLY_ENABLED = 0b010,
    SPO2_ENABLED = 0b011,
};
typedef uint8_t Mode;

enum SamplingRate {
    SAMPLING_RATE_50HZ = 0b000,
    SAMPLING_RATE_100HZ = 0b001,
    SAMPLING_RATE_167HZ = 0b010,
    SAMPLING_RATE_200HZ = 0b011,
    SAMPLING_RATE_400HZ = 0b100,
    SAMPLING_RATE_600HZ = 0b101,
    SAMPLING_RATE_800HZ = 0x110,
    SAMPLING_RATE_1000HZ = 0b111,
};
typedef uint8_t SamplingRate;

enum PulseWidth {
    PULSE_WIDTH_200US_13BITS = 0b00,
    PULSE_WIDTH_400US_14BITS = 0b01,
    PULSE_WIDTH_800US_15BITS = 0b10,
    PULSE_WIDTH_1600US_16BITS = 0b11,
};
typedef uint8_t PulseWidth;

enum LedCurrent {
    CURRENT_0_0MA = 0b0000,
    CURRENT_4_4MA = 0b0001,
    CURRENT_7_6MA = 0b0010,
    CURRENT_11_0MA = 0b0011,
    CURRENT_14_2MA = 0b0100,
    CURRENT_17_4MA = 0b0101,
    CURRENT_20_8MA = 0b0110,
    CURRENT_24_0MA = 0b0111,
    CURRENT_27_1MA = 0b1000,
    CURRENT_30_6MA = 0b1001,
    CURRENT_33_8MA = 0b1010,
    CURRENT_37_0MA = 0b1011,
    CURRENT_40_2MA = 0b1100,
    CURRENT_43_6MA = 0b1101,
    CURRENT_46_8MA = 0b1110,
    CURRENT_50_0MA = 0b1111,
};
typedef uint8_t LedCurrent;

enum HighResolutionMode {
    HIGH_RESOLUTION_DISABLED = 0b0,
    HIGH_RESOLUTION_ENABLED = 0b1,
};
typedef uint8_t HighResolutionMode;

enum Interrupt {
    INTERRUPT_FIFO_ALMOST_FULL = 0b10000000,
    INTERRUPT_TEMPERATURE_READY = 0b01000000,
    INTERRUPT_HEART_RATE_DATA_READY = 0b00100000,
    INTERRUPT_SPO2_DATA_READY = 0b00010000,
    INTERRUPT_POWER_READY = 0b00000001,
};
typedef uint8_t Interrupt;

typedef struct __attribute__((packed)) {
    uint16_t red_led;
    uint16_t ir_led;
} RawData;

#define MAX30100_ADDRESS 0x57
#define CIRCULAR_QUEUE_SIZE 10

// TWI
static const nrf_drv_twi_t* _twi;

// local variables
static RawData _raw_data_queue[CIRCULAR_QUEUE_SIZE] = {0};
static typeof(CIRCULAR_QUEUE_SIZE) _circular_queue_index = 0;

// local functions
static void set_mode(Mode mode);
static void set_sampling_rate(SamplingRate sampling_rate);
static void set_pulse_width(PulseWidth pulse_width);
static void set_leds_current(LedCurrent red_led, LedCurrent ir_led);
static void set_leds_current(LedCurrent red_led, LedCurrent ir_led);
static void set_high_resolution_mode(HighResolutionMode high_resolution);
static void enable_interrupts(uint8_t interrupts);
static void disable_interrupts(uint8_t interrupts);
static void clear_interrupts(void);
static void read_data_from_fifo(RawData* raw_data);

static void enable_gpio_interrupt();
static void on_gpio_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

// public interface
void max30100_init(void) {
    _twi = twi_init(SCL_PIN, SDA_PIN);
    enable_gpio_interrupt();
}

void max30100_setup(void) {
    set_mode(SPO2_ENABLED);
    set_sampling_rate(SAMPLING_RATE_1000HZ);
    set_pulse_width(PULSE_WIDTH_1600US_16BITS);
    set_leds_current(CURRENT_11_0MA, CURRENT_11_0MA);
    set_high_resolution_mode(HIGH_RESOLUTION_ENABLED);
    enable_interrupts(INTERRUPT_POWER_READY | INTERRUPT_SPO2_DATA_READY);
    clear_interrupts();
}

static void set_mode(Mode mode) {
    uint8_t current_mode_config = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, MODE_CONFIG, &current_mode_config, sizeof(current_mode_config));

    const uint8_t mode_config[] = {MODE_CONFIG, (current_mode_config & 0b11111000) | mode};
    twi_write_data(_twi, MAX30100_ADDRESS, mode_config, sizeof(mode_config));
}

static void set_sampling_rate(SamplingRate sampling_rate) {
    uint8_t current_spo2_config = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, SPO2_CONFIG, &current_spo2_config, sizeof(current_spo2_config));

    const uint8_t sampling_rate_config[] = {
        SPO2_CONFIG,
        (current_spo2_config & 0b11100011) | (sampling_rate << 2)};
    twi_write_data(_twi, MAX30100_ADDRESS, sampling_rate_config, sizeof(sampling_rate_config));
}

static void set_pulse_width(PulseWidth pulse_width) {
    uint8_t current_spo2_config = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, SPO2_CONFIG, &current_spo2_config, sizeof(current_spo2_config));

    const uint8_t pulse_width_config[] = {
        SPO2_CONFIG,
        (current_spo2_config & 0b11111100) | pulse_width};
    twi_write_data(_twi, MAX30100_ADDRESS, pulse_width_config, sizeof(pulse_width_config));
}

static void set_leds_current(LedCurrent red_led, LedCurrent ir_led) {
    const uint8_t led_config[] = {
        LED_CONFIG,
        (red_led << 4) | ir_led};
    twi_write_data(_twi, MAX30100_ADDRESS, led_config, sizeof(led_config));
}

static void set_high_resolution_mode(HighResolutionMode high_resolution) {
    uint8_t current_spo2_config = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, SPO2_CONFIG, &current_spo2_config, sizeof(current_spo2_config));

    const uint8_t high_resolution_config[] = {
        SPO2_CONFIG,
        (current_spo2_config & 0b10111111) | (high_resolution << 6)};
    twi_write_data(_twi, MAX30100_ADDRESS, high_resolution_config, sizeof(high_resolution_config));
}

static void read_data_from_fifo(RawData *raw_data) {
    uint8_t data[4];
    twi_read_data(_twi, MAX30100_ADDRESS, FIFO_DATA_REGISTER, data, sizeof(data));

    raw_data->ir_led = (data[0] << 8) | data[1];
    raw_data->red_led = (data[2] << 8) | data[3];

    NRF_LOG_INFO("IR: %05d | %05d", raw_data->ir_led, raw_data->red_led);
}

static void enable_interrupts(uint8_t interrupts) {
    uint8_t current_enabled_interrupts = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, INTERRUPT_ENABLE, &current_enabled_interrupts, sizeof(current_enabled_interrupts));

    const uint8_t interrupt_enable[] = {
        INTERRUPT_ENABLE, 
        current_enabled_interrupts | interrupts
    };
    twi_write_data(_twi, MAX30100_ADDRESS, interrupt_enable, sizeof(interrupt_enable));
}

static void disable_interrupts(uint8_t interrupts) {
    uint8_t current_disabled_interrupts = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, INTERRUPT_ENABLE, &current_disabled_interrupts, sizeof(current_disabled_interrupts));

    const uint8_t interrupt_enable[] = {
        INTERRUPT_ENABLE, 
        current_disabled_interrupts & (~interrupts)
    };
    twi_write_data(_twi, MAX30100_ADDRESS, interrupt_enable, sizeof(interrupt_enable));
}

static void clear_interrupts(void) {
    uint8_t interrupt_status = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, INTERRUPT_STATUS, &interrupt_status, sizeof(interrupt_status));
}

static void enable_gpio_interrupt() {
    ret_code_t err_code;

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    if (!nrf_drv_gpiote_in_is_set(INT_PIN))
    {
        err_code = nrf_drv_gpiote_in_init(INT_PIN, &in_config, on_gpio_interrupt);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(INT_PIN, true);
    }
    else
    {
        NRF_LOG_INFO("\r\nGPIO %d already used.", INT_PIN);
    }
}

// callbacks
static void on_gpio_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    uint8_t interrupt_status = 0;
    twi_read_data(_twi, MAX30100_ADDRESS, INTERRUPT_STATUS, &interrupt_status, sizeof(interrupt_status));

    if (interrupt_status & INTERRUPT_POWER_READY) {
        // TODO: shouldn't be called from inside the interruption
        max30100_setup();
    }

    if (interrupt_status & INTERRUPT_SPO2_DATA_READY) {
        // TODO: shouldn't be called from inside the interruption
        const uint8_t index = _circular_queue_index++ % CIRCULAR_QUEUE_SIZE;
        read_data_from_fifo(&_raw_data_queue[index]);
    }
}
