#include "max30101.h"

#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf.h"

#include <stdint.h>

enum MAX30101Registers
{
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