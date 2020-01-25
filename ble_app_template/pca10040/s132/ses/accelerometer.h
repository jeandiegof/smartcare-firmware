#pragma once

#include <stdint.h>
#include <stdbool.h>

void accelerometer_start(void);
uint8_t accelerometer_free_fall(void);
void clear_accelerometer_free_fall(void);
uint8_t accelerometer_activity(void);
void clear_accelerometer_activity();
uint8_t accelerometer_inactivity(void);
void clear_accelerometer_inactivity(void);
bool is_interrupt_available(void);
void handle_interrupts(void);
void accelerometer_print_axis_data(void);
