#pragma once

#include <stdint.h>

typedef uint8_t Events;
typedef void (*delegate_function)(void);

typedef enum {
    AccelerometerInterruptionEvt = (1 << 0),
    HeartrateInterruptionEvt = (1 << 1),
} Event;

void set_event(Event event);
void clear_event(Event event);
void consume_event(Event event, delegate_function event_handler);
