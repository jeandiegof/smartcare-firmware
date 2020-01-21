#pragma once

#include <stdint.h>

void heart_rate_start(void);
void wait_for_heart_rate_sample(void);
uint32_t read_hr_sample(void);