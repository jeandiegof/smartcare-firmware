#pragma once

#include <stdint.h>

void accelerometer_start(void);
uint8_t accelerometer_fall_count(void);
void accelerometer_print_axis_data(void);