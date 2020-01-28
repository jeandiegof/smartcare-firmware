#include "events.h"
#include <stddef.h>


Events _events = NULL;

inline void set_event(Event event) { _events |= event; }

inline void clear_event(Event event) { _events &= ~event; }

inline void consume_event(Event event, delegate_function event_handler) {
    if (_events & event) {
        if (event_handler != NULL) event_handler();
        clear_event(event);
    }
}
