#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "MAX30100_Registers.h"
#include "ringbuffer.h"
#include "I2C.c"
#include "MAX30100.h"

bool begin()
{
    if(getPartId() != EXPECTED_PART_ID)
    	return false;

    setMode(DEFAULT_MODE);
    setLedsPulseWidth(DEFAULT_PULSE_WIDTH);
    setSamplingRate(DEFAULT_SAMPLING_RATE);
    setLedsCurrent(DEFAULT_IR_LED_CURRENT, DEFAULT_RED_LED_CURRENT);
    setHighresModeEnabled(true);

    bufferInit(redBuffer,16, uint16_t);
    bufferInit(irBuffer,16, uint16_t);
    redBuffer_ptr = &redBuffer;
    irBuffer_ptr = &irBuffer;

    return true;    
}


uint8_t getPartId()
{
	return readRegister(MAX30100_REG_PART_ID);
}

uint8_t readRegister(uint8_t address)
{
	uint8_t partId [1] = {0};
	I2C_Master_ReadReg(SLAVE_ADDR, address, 1);
	CopyArray(ReceiveBuffer, partId, 1);
	return partId[0];
}

void writeRegister(uint8_t address, uint8_t data)
{
	uint8_t tBuffer [1] = {0};
	tBuffer[0] = data;
	I2C_Master_WriteReg(SLAVE_ADDR, address, tBuffer, 1);
}

void setMode(Mode mode)
{
    writeRegister(MAX30100_REG_MODE_CONFIGURATION, mode);
}

void setLedsPulseWidth(LEDPulseWidth ledPulseWidth)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    writeRegister(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xfc) | ledPulseWidth);
}

void setSamplingRate(SamplingRate samplingRate)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    writeRegister(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xe3) | (samplingRate << 2));
}

void setLedsCurrent(LEDCurrent irLedCurrent, LEDCurrent redLedCurrent)
{
    writeRegister(MAX30100_REG_LED_CONFIGURATION, redLedCurrent << 4 | irLedCurrent);
}

void setHighresModeEnabled(bool enabled)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    if (enabled) {
        writeRegister(MAX30100_REG_SPO2_CONFIGURATION, previous | MAX30100_SPC_SPO2_HI_RES_EN);
    } else {
        writeRegister(MAX30100_REG_SPO2_CONFIGURATION, previous & ~MAX30100_SPC_SPO2_HI_RES_EN);
    }
}

void resetFifo()
{
    writeRegister(MAX30100_REG_FIFO_WRITE_POINTER, 0);
    writeRegister(MAX30100_REG_FIFO_READ_POINTER, 0);
    writeRegister(MAX30100_REG_FIFO_OVERFLOW_COUNTER, 0);
}

void resume()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig &= ~MAX30100_MC_SHDN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

void shutdown()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig |= MAX30100_MC_SHDN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

float retrieveTemperature()
{
    int8_t tempInteger = readRegister(MAX30100_REG_TEMPERATURE_DATA_INT);
    float tempFrac = readRegister(MAX30100_REG_TEMPERATURE_DATA_FRAC);

    return tempFrac * 0.0625 + tempInteger;
}

uint8_t retrieveTemperatureInteger()
{
    return readRegister(MAX30100_REG_TEMPERATURE_DATA_INT);
}

bool isTemperatureReady()
{
    return !(readRegister(MAX30100_REG_MODE_CONFIGURATION) & MAX30100_MC_TEMP_EN);
}

void startTemperatureSampling()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig |= MAX30100_MC_TEMP_EN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

void burstRead(uint8_t baseAddress, uint8_t *buffer, uint8_t length)
{
	I2C_Master_ReadReg(SLAVE_ADDR, baseAddress, length);
	CopyArray(ReceiveBuffer, buffer, length);
}

void readFifoData()
{
	uint8_t buffer[MAX30100_FIFO_DEPTH*4];
    uint8_t toRead, i;
    uint16_t redWrite, irWrite;

    toRead = (readRegister(MAX30100_REG_FIFO_WRITE_POINTER) - readRegister(MAX30100_REG_FIFO_READ_POINTER)) & (MAX30100_FIFO_DEPTH-1);

    if(toRead)
    {
    	burstRead(MAX30100_REG_FIFO_DATA, buffer, 4 * toRead);
    	
    	for (i=0 ; i < toRead ; ++i)
    	{
    		irWrite = (uint16_t)((buffer[i*4] << 8) | buffer[i*4 + 1]);
    		redWrite = ((buffer[i*4 + 2] << 8) | buffer[i*4 + 3]);
    		bufferWrite(irBuffer_ptr,irWrite);
    		bufferWrite(redBuffer_ptr,redWrite);
    	}	
    }

}

void update()
{
	readFifoData();
}

bool getRawValues(uint16_t *ir, uint16_t *red)
{
	if(!isBufferEmpty(irBuffer_ptr) && !isBufferEmpty(redBuffer_ptr))
	{
		bufferRead(irBuffer_ptr, *ir);
		bufferRead(redBuffer_ptr, *red);
		return true;
	}
	else
		return false;
}
