#include "MAX30100_PulseOximeter.h"
#include "MAX30100.h"
#include "MAX30100_BeatDetector.h"
#include "MAX30100_BeatDetector.c"

#include "MAX30100_Filters.h"

#include "MAX30100_SpO2Calculator.h"
#include "MAX30100_SpO2Calculator.c"

bool pulseOxBegin(PulseOximeterDebuggingMode debuggingMode_)
{
	debuggingMode = debuggingMode_;

	bool ready = begin();

	if(!ready)
	{
		if(debuggingMode != PULSEOXIMETER_DEBUGGINGMODE_NONE)
			sendString("Failed to initialize the HRM sensor");
		return false;
	}

	setMode(MAX30100_MODE_SPO2_HR);
    setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);

    setDCAlpha(DC_REMOVER_ALPHA, 'R');
    setDCAlpha(DC_REMOVER_ALPHA, 'I');

    state = PULSEOXIMETER_STATE_IDLE;

    return true;
}

void pulseOxCheckSample()
{
	uint16_t rawIRValue, rawRedValue;
	float irACValue, redACValue, filteredPulseValue;
	bool beatDetected; 

	// Dequeue all available samples
	while(getRawValues(&rawIRValue, &rawRedValue))
	{
        irACValue = dcStepIr(rawIRValue);
        redACValue = dcStepRed(rawRedValue);

        filteredPulseValue = butterworthStep(-irACValue);
		beatDetected = beatDetectorAddSample(filteredPulseValue);

		if (beatDetectorGetRate() > 0) {
            state = PULSEOXIMETER_STATE_DETECTING;
            spO2CalcUpdate(irACValue, redACValue, beatDetected);
        } else if (state == PULSEOXIMETER_STATE_DETECTING) {
            state = PULSEOXIMETER_STATE_IDLE;
            spO2CalcReset();
        }

        switch (debuggingMode) {
            case PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES:
                sendString("I-");
                sendInt((unsigned int) rawIRValue);
                sendData('\t');
                sendString("R-");
                sendInt((unsigned int) rawRedValue);
                sendData('\n');
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES:
                sendString("IRac: ");
                sendFloat(irACValue);
                sendData('\t');
                sendString("Rac: ");
                sendFloat(redACValue);
                sendData('\n');     
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT:
                sendString("R: ");
                sendFloat(filteredPulseValue);
                sendData('\t');
                sendString("TH: ");
                sendFloat(beatDetectorGetCurrentThreshold());
                sendData('\t');
                switch (stateBeat)
                {
                    case BEATDETECTOR_STATE_INIT:
                        sendString("INIT");
                        break;
                    case BEATDETECTOR_STATE_WAITING:
                        sendString("WAIT");
                        break;
                    case BEATDETECTOR_STATE_FOLLOWING_SLOPE:
                        sendString("FSLOPE");
                        break;
                    case BEATDETECTOR_STATE_MAYBE_DETECTED:
                        sendString("MDET");
                        break;
                    case BEATDETECTOR_STATE_MASKING:
                        sendString("MASK");
                        break;
                    default:
                        sendString("ERR");
                        break; 
                }
                sendData('\n');
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_PULSEPLOTTER:
                sendFloat(filteredPulseValue);
                sendData('\n');
                break;

            default:
                break;
        }

        if (beatDetected && onBeatDetected) {
            onBeatDetected();
        }        
	}
}

void pulseOxCheckCurrentBias()
{
    // Follower that adjusts the red led current in order to have comparable DC baselines between
    // red and IR leds. The numbers are really magic: the less possible to avoid oscillations

    if (millis() - tsLastBiasCheck > CURRENT_ADJUSTMENT_PERIOD_MS) {
        bool changed = false;
        if (getDCW('I') - getDCW('R') > 70000 && redLedCurrentIndex < MAX30100_LED_CURR_50MA) {
            ++redLedCurrentIndex;
            changed = true;
        } else if (getDCW('R') - getDCW('I') > 70000 && redLedCurrentIndex > 0) {
            --redLedCurrentIndex;
            changed = true;
        }

        if (changed) {
            setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);
            tsLastCurrentAdjustment = millis();

            if (debuggingMode != PULSEOXIMETER_DEBUGGINGMODE_NONE) {
                sendString("Icurr:");
                sendInt(redLedCurrentIndex);
                sendData('\n');
            }

        }

        tsLastBiasCheck = millis();
    }

}

void pulseOxUpdate()
{
    update();

    pulseOxCheckSample();
    pulseOxCheckCurrentBias();
}

float pulseOxGetHeartRate()
{
    return beatDetectorGetRate();
}

uint8_t pulseOxGetSpO2()
{
    return spO2CalcGetSpO2();
}

uint8_t pulseOxGetRedLedCurrentBias()
{
    return redLedCurrentIndex;
}

void pulseOxSetOnBeatDetectedCallback(void (*cb)())
{
    onBeatDetected = cb;
}

void pulseOxSetIRLedCurrent(LEDCurrent irLedNewCurrent)
{
    irLedCurrent = irLedNewCurrent;
    setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);
}

void pulseOxiShutdown()
{
    shutdown();
}

void pulseOxResume()
{
    resume();
}
