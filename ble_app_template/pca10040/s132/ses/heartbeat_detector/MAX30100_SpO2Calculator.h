#ifndef MAX30100_SPO2CALCULATOR_H_
#define MAX30100_SPO2CALCULATOR_H_

#include <stdint.h>

#define CALCULATE_EVERY_N_BEATS 3

void spO2CalcUpdate(float irACValue, float redACValue, bool beatDetected);
void spO2CalcReset();
uint8_t spO2CalcGetSpO2();

// SaO2 Look-up Table
// http://www.ti.com/lit/an/slaa274b/slaa274b.pdf
const uint8_t spO2LUT[43] = {100,100,100,100,99,99,99,99,99,99,98,98,98,98,98,97,97,97,97,97,97,96,96,96,96,96,96,95,95,95,95,95,95,94,94,94,94,94,93,93,93,93,93};

float irACValueSqSum = 0;
float redACValueSqSum = 0;
uint8_t beatsDetectedNum = 0;
uint32_t samplesRecorded = 0;
uint8_t spO2 = 0;

#endif /* MAX30100_SPO2CALCULATOR_H_ */
