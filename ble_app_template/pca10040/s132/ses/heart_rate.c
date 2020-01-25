#include "heart_rate.h"
#ifdef MAX30100
#include "max30100.h"
#else
#include "max30101.h"
#endif

void heart_rate_start(void) {
#ifdef MAX30100
    max30100_init();
    max30100_setup();
#else
    max30101_init();
    max30101_setup();
#endif
}
