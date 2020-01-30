#pragma once

#include <stdint.h>
#include <stdbool.h>

void adxl345_init(void);
void adxl345_setup(void);
const uint8_t * adxl345_request_axis_data(void);
void adxl345_start_free_fall_mode(void);
void adxl345_start_activity_detection_mode(void);
void adxl345_start_inactivity_detection_mode(void);
bool adxl345_free_fall(void);
void adxl345_clear_free_fall(void);
bool adxl345_activity(void);
void adxl345_clear_activity(void);
bool adxl345_inactivity(void);
void adxl345_clear_inactivity(void);
void adxl345_handle_interrupt(void);
