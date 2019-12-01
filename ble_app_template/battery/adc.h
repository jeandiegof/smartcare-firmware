#include <stdint.h>

typedef void (*battery_update_cb)(uint8_t);
void adc_init(battery_update_cb battery_update_fn);
void request_battery_level(void);