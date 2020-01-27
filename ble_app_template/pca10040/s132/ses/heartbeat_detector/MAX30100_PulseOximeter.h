#ifndef MAX30100_PULSEOXIMETER_H_
#define MAX30100_PULSEOXIMETER_H_

#define SAMPLING_FREQUENCY                  100
#define CURRENT_ADJUSTMENT_PERIOD_MS        500
#define DEFAULT_IR_LED_CURRENT              MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT_START               MAX30100_LED_CURR_27_1MA
#define DC_REMOVER_ALPHA                    0.95

#include <stdint.h>

#include "MAX30100.h"
#include "MAX30100_BeatDetector.h"
#include "MAX30100_Filters.h"
#include "MAX30100_SpO2Calculator.h"

typedef enum PulseOximeterState {
    PULSEOXIMETER_STATE_INIT,
    PULSEOXIMETER_STATE_IDLE,
    PULSEOXIMETER_STATE_DETECTING
} PulseOximeterState;

typedef enum PulseOximeterDebuggingMode {
    PULSEOXIMETER_DEBUGGINGMODE_NONE,
    PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES,
    PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES,
    PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT,
    PULSEOXIMETER_DEBUGGINGMODE_PULSEPLOTTER
} PulseOximeterDebuggingMode;

PulseOximeterDebuggingMode debuggingMode;
PulseOximeterState state = PULSEOXIMETER_STATE_INIT;
LEDCurrent irLedCurrent = DEFAULT_IR_LED_CURRENT;
uint8_t redLedCurrentIndex = (uint8_t)RED_LED_CURRENT_START;

uint32_t tsFirstBeatDetected = 0;
uint32_t tsLastBeatDetected = 0;
uint32_t tsLastBiasCheck = 0;
uint32_t tsLastCurrentAdjustment = 0;

void (*onBeatDetected)();
bool pulseOxBegin(PulseOximeterDebuggingMode debuggingMode_);
void pulseOxCheckSample();
void pulseOxCheckCurrentBias();
void pulseOxUpdate();
float pulseOxGetHeartRate();
uint8_t pulseOxGetSpO2();
uint8_t pulseOxGetRedLedCurrentBias();
void pulseOxSetOnBeatDetectedCallback(void (*cb)());
void pulseOxSetIRLedCurrent(LEDCurrent irLedNewCurrent);
void pulseOxiShutdown();
void pulseOxResume();

#endif /* MAX30100_PULSEOXIMETER_H_ */
