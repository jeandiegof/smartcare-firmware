#pragma once

#include <stdint.h>

uint32_t init_smartcare_services(void);
uint32_t notify_emergency(uint8_t value);
uint32_t notify_bpm(uint8_t value);
uint32_t notify_arrhythmia(uint8_t value);
uint32_t notify_fall_detection(uint8_t value);
uint32_t notify_battery_change(uint8_t value);
