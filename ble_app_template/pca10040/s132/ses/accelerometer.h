#pragma once

#include <stdint.h>

void accelerometer_start(void);
uint8_t accelerometer_fall_count(void);
void accelerometer_print_axis_data(void);
uint8_t accelerometer_read_register(uint8_t reg);
void handle_accelerometer_interruption(void);
