#pragma once

#include <stdint.h>

void max30101_init(void);
void max30101_setup(void);
void wait_for_heart_rate_sample(void);
uint32_t read_hr_sample(void);