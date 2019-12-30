#include "heart_rate.h"
#include "max30100.h"

void heart_rate_start(void) {
    max30100_init();
    max30100_setup();
}
