#pragma once

#include <stdint.h>

void adxl345_init(void);
void adxl345_setup(void);
const uint8_t * adxl345_request_axis_data(void);
void adxl345_start_free_fall_mode(void);
void adxl345_start_activity_detection_mode(void);
void adxl345_start_inactivity_detection_mode(void);
uint8_t adxl345_fall_count(void);
uint8_t adxl345_activity_count(void);
uint8_t adxl345_inactivity_count(void);
void adxl345_handle_interrupt(void);
