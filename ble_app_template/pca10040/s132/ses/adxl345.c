#include "adxl345.h"
#include "events.h"
#include "twi.h"
#include "gpio.h"

#include "app_error.h"
#include "nrf.h"
#include "nrf_log.h"

enum ADXL345Registers {
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

typedef enum InterruptSource {
    FREE_FALL = 0b00000100,
    ACTIVITY = 0b00010000,
    INACTIVITY = 0b00001000,
} InterruptSource;

#define ADXL345_ADDR 0x53U

// GPIOs
enum ADXL345Pins {
    SDA_PIN = 27,
    SCL_PIN = 28,
    INT1_PIN = 30,
    INT2_PIN = 29,
};

// TWI
static const nrf_drv_twi_t *_twi;

// local variables
static bool _free_fall = false;
static bool _activity = false;
static bool _inactivity = false;
static uint8_t m_samples[6] = {0};

// local functions
static void enable_measurement_mode(void);
static void setup_free_fall_mode(void);
static void map_free_fall_interrupt_to_int1(void);
static void enable_free_fall_interrupt(void);
static void setup_activity_detection_mode(void);
static void map_activity_interrupt_to_int1(void);
static void enable_activity_interrupt(void);
static void setup_inactivity_detection_mode(void);
static void map_inactivity_interrupt_to_int1(void);
static void enable_inactivity_interrupt(void);
static void clear_interrupts(void);
static void on_int1_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void adxl345_init(void) {
    _twi = twi_init(SCL_PIN, SDA_PIN);
    APP_ERROR_CHECK(enable_gpio_interrupt(INT1_PIN, &on_int1_interrupt, NRF_GPIOTE_POLARITY_LOTOHI, NRF_GPIO_PIN_PULLUP));
}

void adxl345_setup(void) {
    enable_measurement_mode();
}

void adxl345_start_free_fall_mode(void) {
    setup_free_fall_mode();
    map_free_fall_interrupt_to_int1();
    enable_free_fall_interrupt();
    clear_interrupts();
}

void adxl345_start_activity_detection_mode(void) {
    setup_activity_detection_mode();
    map_activity_interrupt_to_int1();
    enable_activity_interrupt();
    clear_interrupts();
}

void adxl345_start_inactivity_detection_mode(void) {
    setup_inactivity_detection_mode();
    map_inactivity_interrupt_to_int1();
    enable_inactivity_interrupt();
    clear_interrupts();
}

const uint8_t *adxl345_request_axis_data(void) {
    twi_read_data(_twi, ADXL345_ADDR, DATAX0, m_samples, sizeof(m_samples));
    return m_samples;
}

bool adxl345_free_fall(void) { return _free_fall; }

void adxl345_clear_free_fall(void) { _free_fall = false; }

bool adxl345_activity(void) { return _activity; }

void adxl345_clear_activity(void) { _activity = false; }

bool adxl345_inactivity(void) { return _inactivity; }

void adxl345_clear_inactivity(void) { _inactivity = false; }

static void enable_measurement_mode(void) {
    uint8_t current_measurement_mode = 0;
    twi_read_data(_twi, ADXL345_ADDR, POWER_CTL, &current_measurement_mode, sizeof(current_measurement_mode));

    const uint8_t measurement_mode[2] = {POWER_CTL, current_measurement_mode | 0b00001000};
    twi_write_data(_twi, ADXL345_ADDR, measurement_mode, sizeof(measurement_mode));
}

// Free-fall detection
static void setup_free_fall_mode(void) {
    uint8_t free_fall_threshold[2] = {THRESH_FF, 0x0A}; // 0.625g
    twi_write_data(_twi, ADXL345_ADDR, free_fall_threshold, sizeof(free_fall_threshold));

    uint8_t free_fall_time[2] = {TIME_FF, 0x14}; // 100ms
    twi_write_data(_twi, ADXL345_ADDR, free_fall_time, sizeof(free_fall_time));
}

static void map_free_fall_interrupt_to_int1(void) {
    uint8_t current_interrupts_map = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_MAP, &current_interrupts_map, sizeof(current_interrupts_map));

    const uint8_t interrupt_map[2] = {INT_MAP, current_interrupts_map & 0b11111011};
    twi_write_data(_twi, ADXL345_ADDR, interrupt_map, sizeof(interrupt_map));
}

static void enable_free_fall_interrupt(void) {
    uint8_t current_interrupts_enabled = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_ENABLE, &current_interrupts_enabled, sizeof(current_interrupts_enabled));

    const uint8_t interrupt_enable[2] = {INT_ENABLE, current_interrupts_enabled | 0b00000100};
    twi_write_data(_twi, ADXL345_ADDR, interrupt_enable, sizeof(interrupt_enable));
}

// Activity/Impact detection
static void setup_activity_detection_mode(void) {
    const uint8_t activity_threshold[2] = {THRESH_ACT, 0x80}; // 8.0g
    twi_write_data(_twi, ADXL345_ADDR, activity_threshold, sizeof(activity_threshold));

    uint8_t current_activity_inactivity_control = 0;
    twi_read_data(_twi, ADXL345_ADDR, ACT_INACT_CTL, &current_activity_inactivity_control, sizeof(current_activity_inactivity_control));

    const uint8_t activity_enable[2] = {ACT_INACT_CTL, current_activity_inactivity_control | 0b11110000}; // AC-coupled, X, Y, Z
    twi_write_data(_twi, ADXL345_ADDR, activity_enable, sizeof(activity_enable));
}

static void map_activity_interrupt_to_int1(void) {
    uint8_t current_interrupts_map = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_MAP, &current_interrupts_map, sizeof(current_interrupts_map));

    const uint8_t interrupt_map[2] = {INT_MAP, current_interrupts_map & 0b11101111};
    twi_write_data(_twi, ADXL345_ADDR, interrupt_map, sizeof(interrupt_map));
}

static void enable_activity_interrupt(void) {
    uint8_t current_interrupts_enabled = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_ENABLE, &current_interrupts_enabled, sizeof(current_interrupts_enabled));

    const uint8_t interrupt_enable[2] = {INT_ENABLE, current_interrupts_enabled | 0b00010000};
    twi_write_data(_twi, ADXL345_ADDR, interrupt_enable, sizeof(interrupt_enable));
}

// Inactivity detection
static void setup_inactivity_detection_mode(void) {
    const uint8_t inactivity_threshold[2] = {THRESH_INACT, 0x03}; // 0.1875g
    twi_write_data(_twi, ADXL345_ADDR, inactivity_threshold, sizeof(inactivity_threshold));

    const uint8_t inactivity_time[2] = {TIME_INACT, 0x01}; // 1s
    twi_write_data(_twi, ADXL345_ADDR, inactivity_time, sizeof(inactivity_time));

    uint8_t current_activity_inactivity_control = 0;
    twi_read_data(_twi, ADXL345_ADDR, ACT_INACT_CTL, &current_activity_inactivity_control, sizeof(current_activity_inactivity_control));

    const uint8_t inactivity_enable[2] = {ACT_INACT_CTL, current_activity_inactivity_control | 0b00001111}; // AC-coupled, X, Y, Z
    twi_write_data(_twi, ADXL345_ADDR, inactivity_enable, sizeof(inactivity_enable));
}

static void map_inactivity_interrupt_to_int1(void) {
    uint8_t current_interrupts_map = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_MAP, &current_interrupts_map, sizeof(current_interrupts_map));

    const uint8_t interrupt_map[2] = {INT_MAP, current_interrupts_map & 0b11110111};
    twi_write_data(_twi, ADXL345_ADDR, interrupt_map, sizeof(interrupt_map));
}

static void enable_inactivity_interrupt(void) {
    uint8_t current_interrupts_enabled = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_ENABLE, &current_interrupts_enabled, sizeof(current_interrupts_enabled));

    const uint8_t interrupt_enable[2] = {INT_ENABLE, current_interrupts_enabled | 0b00001000};
    twi_write_data(_twi, ADXL345_ADDR, interrupt_enable, sizeof(interrupt_enable));
}

// Interrupts
static void clear_interrupts(void) {
    uint8_t interrupt_status = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_SOURCE, &interrupt_status, sizeof(interrupt_status));
}

void on_int1_interrupt(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    set_event(AccelerometerInterruptionEvt);
}

static uint8_t read_interrupt_source(void) {
    uint8_t interrupt_source = 0;
    twi_read_data(_twi, ADXL345_ADDR, INT_SOURCE, &interrupt_source, sizeof(interrupt_source));
    return interrupt_source;
}

void adxl345_handle_interrupt(void) {
    InterruptSource interrupt_source = read_interrupt_source();

    if (interrupt_source & FREE_FALL) {
        _free_fall = true;
    }
    if (interrupt_source & ACTIVITY) {
        _activity = true;
    }
    if (interrupt_source & INACTIVITY) {
        _inactivity = true;
    }
}
