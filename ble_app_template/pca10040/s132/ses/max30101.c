#include "max30101.h"
#include "gpio.h"

#include "app_error.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_assert.h"
#include "nrf_drv_twi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "twi.h"

#include <stdint.h>

#define MAX30101_ADDRESS 0x57

static uint8_t current_ma_to_register(uint8_t current_ma) {
    uint16_t register_value = (((uint16_t)current_ma * 0x00ff) / 51);
    return register_value > 0x00ff ? 0xff : register_value;
}

// Mode configurations
#define MODE_SHDN 0b10000000
#define MODE_RESET 0b01000000

typedef enum SamplingRate {
    MAX30101_SR_50 = 0,
    MAX30101_SR_100 = 1,
    MAX30101_SR_200 = 2,
    MAX30101_SR_400 = 3,
    MAX30101_SR_800 = 4,
    MAX30101_SR_1000 = 5,
    MAX30101_SR_1600 = 6,
    MAX30101_SR_3200 = 7
} SamplingRate;

typedef enum LEDPulseWidth {
    MAX3010_LED_PW_69 = 0x00,
    MAX3010_LED_PW_118 = 0x01,
    MAX3010_LED_PW_215 = 0x02,
    MAX3010_LED_PW_411 = 0x03,
} LEDPulseWidth;

typedef enum ADCRange {
    MAX30101_RANGE2048 = 0x00,
    MAX30101_RANGE4096 = 0x01,
    MAX30101_RANGE8192 = 0x02,
    MAX30101_RANGE16384 = 0x03,
} ADCRange;

enum MAX30101Registers {
    // STATUS
    INTERRUPT_STATUS_1 = 0x00,
    INTERRUPT_STATUS_2 = 0x01,
    INTERRUPT_ENABLE_1 = 0x02,
    INTERRUPT_ENABLE_2 = 0x03,
    // FIFO
    FIFO_WRITE_POINTER = 0x04,
    OVERFLOW_COUNTER = 0x05,
    FIFO_READ_POINTER = 0x06,
    FIFO_DATA_REGISTER = 0x07,
    // CONFIGURATION
    FIFO_CONFIG = 0x08,
    MODE_CONFIG = 0x09,
    SPO2_CONFIG = 0x0A,
    LED1_PULSE_AMPLITUDE = 0x0C,
    LED2_PULSE_AMPLITUDE = 0x0D,
    LED3_PULSE_AMPLITUDE = 0x0E,
    LED4_PULSE_AMPLITUDE = 0x0F,
    MULTI_LED_MODE_1 = 0x11,
    MULTI_LED_MODE_2 = 0x12,
    // DIE TEMPERATURE
    DIE_TEMP_INTEGER = 0x1F,
    DIE_TEMP_FRACTION = 0x20,
    DIE_TEMP_CONFIG = 0x21,
    // PART ID
    REVISION_ID = 0xFE,
    PART_ID = 0xFF,
};

enum MAX30101Modes {
    HR_ONLY_ENABLED = 0b010,
    SPO2_ENABLED = 0b011,
    MULTI_LED = 0b111,
};
typedef uint8_t Mode;

enum MAX30101Pins {
    SDA_PIN = 02,
    SCL_PIN = 03,
    INT_PIN = 04,
};

static const nrf_drv_twi_t* _twi;

static void set_mode(Mode mode) {
    uint8_t current_mode_config = 0;
    twi_read_data(_twi, MAX30101_ADDRESS, MODE_CONFIG, &current_mode_config,
                  sizeof(current_mode_config));

    const uint8_t mode_config[] = {MODE_CONFIG,
                                   (current_mode_config & 0b11111000) | mode};
    twi_write_data(_twi, MAX30101_ADDRESS, mode_config, sizeof(mode_config));
}

static bool reset(void) {
    const uint8_t mode_config[] = {MODE_CONFIG, MODE_RESET};
    twi_write_data(_twi, MAX30101_ADDRESS, mode_config, sizeof(mode_config));

    for (uint8_t cnt = 0; cnt < 5; cnt++) {
        uint8_t conf = 0xff;
        twi_read_data(_twi, MAX30101_ADDRESS, MODE_CONFIG, &conf, sizeof(conf));
        if (!(conf & MODE_RESET)) {
            return true;
        }
        nrf_delay_ms(10);
    }
    return false;
}

static void set_leds_current(uint8_t red_current, uint8_t ir_current,
                             uint8_t green1_current, uint8_t green2_current) {
    const uint8_t red_data[2] = {LED1_PULSE_AMPLITUDE,
                                 current_ma_to_register(red_current)};
    twi_write_data(_twi, MAX30101_ADDRESS, red_data, sizeof(red_data));

    const uint8_t ir_data[2] = {LED2_PULSE_AMPLITUDE,
                                current_ma_to_register(ir_current)};
    twi_write_data(_twi, MAX30101_ADDRESS, ir_data, sizeof(ir_data));

    const uint8_t green1_data[2] = {LED3_PULSE_AMPLITUDE,
                                    current_ma_to_register(green1_current)};
    twi_write_data(_twi, MAX30101_ADDRESS, green1_data, sizeof(green1_data));

    const uint8_t green2_data[2] = {LED3_PULSE_AMPLITUDE,
                                   current_ma_to_register(green2_current)};
    twi_write_data(_twi, MAX30101_ADDRESS, green2_data, sizeof(green2_data));
}

static void set_leds_slots(uint8_t slot1, uint8_t slot2, uint8_t slot3,
                           uint8_t slot4) {
    const uint8_t data_register1[2] = {MULTI_LED_MODE_1,
                                       slot2 << 4 | slot1};
    twi_write_data(_twi, MAX30101_ADDRESS, data_register1,
                   sizeof(data_register1));

    const uint8_t data_register2[2] = {MULTI_LED_MODE_2,
                                       slot4 << 4 | slot3};
    twi_write_data(_twi, MAX30101_ADDRESS, data_register2,
                   sizeof(data_register2));
}

static void set_sampling_rate(SamplingRate sampling_rate) {
    const uint8_t data_register[2] = {SPO2_CONFIG, sampling_rate << 2};
    twi_write_data(_twi, MAX30101_ADDRESS, data_register,
                   sizeof(data_register));
}

static void set_pulse_width(LEDPulseWidth pw) {
    const uint8_t data_register[2] = {SPO2_CONFIG, pw};
    twi_write_data(_twi, MAX30101_ADDRESS, data_register,
                   sizeof(data_register));
}

static void set_adc_range(ADCRange adc_range) {
    const uint8_t data_register[2] = {SPO2_CONFIG, adc_range << 5};
    twi_write_data(_twi, MAX30101_ADDRESS, data_register,
                   sizeof(data_register));
}

uint32_t read_hr_sample(void) {
    // This depends on the number of slots configured. For 1 led in the first slot, we'll read 3 bytes
    uint8_t read_data[3] = {0};
    twi_read_data(_twi, MAX30101_ADDRESS, FIFO_DATA_REGISTER, read_data, sizeof(read_data));
    (val[3]<<16 | val[4]<<8 | val[5])
}

void setup(void) {
    _twi = twi_init(SCL_PIN, SDA_PIN);
    ASSERT(reset());
    set_mode(MULTI_LED);
    nrf_delay_ms(50);
    set_leds_current(
        0, 0,
        51, 51);  // Max value for led current = 51 mA (TODO: clip this value).
    set_leds_slots(0b011, 0, 0, 0);  // 0x011 -> green. Datasheet Pag. 22.
    set_sampling_rate(MAX30101_SR_100);
    set_pulse_width(MAX3010_LED_PW_411);
    set_adc_range(MAX30101_RANGE16384);
}