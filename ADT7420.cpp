/*
 * ADT7420.cpp
 *
 *  Created on: Jan. 22nd, 2016
 *      Author: KuangQi
 */

#include "ADT7420.h"

ADT7420::ADT7420(PinName sda, PinName scl, I2CAddress addr, int freq)
: sensor(sda, scl), i2c_addr(addr), res(RES_13BIT)
{
	sensor.frequency(freq);
}

// Probe for the ADT7420 using a Zero Length Transfer
// Return true if a device is found on the specific address
bool ADT7420::isOK()
{

    if (sensor.write(i2c_addr, NULL, 0) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Check if there are new converted data available
// Return true if there are new data available
bool ADT7420::dataAvailable()
{
	uint8_t ret = 0;
	uint8_t reg = REG_STATUS;
	MBED_ASSERT(0 == sensor.write(i2c_addr, (char*)(&reg), 1, true));
	MBED_ASSERT(0 == sensor.read(i2c_addr, (char*)(&ret), 1, false));
	return (ret & READY_BIT) == 0;
}

// Read the ID of ADT7420 sensor
// Should always return 0xCB
int ADT7420::getID()
{
	uint8_t ret = 0;
	uint8_t reg = REG_ID;

	MBED_ASSERT(0 == sensor.write(i2c_addr, (char*)(&reg), 1, true));
	MBED_ASSERT(0 == sensor.read(i2c_addr, (char*)(&ret), 1, false));
	return ret;
}

void ADT7420::setResolution(Resolution resolution)
{
	uint16_t reg = (REG_CONFIG & 0xff);

	reg_config &= (~RESOL_BIT);
	reg_config |= (resolution & RESOL_BIT);
	reg |= ((reg_config << 8) & 0xff00);

	res = resolution;

	MBED_ASSERT(0 == sensor.write(i2c_addr, (char*)(&reg), 2));
}

void ADT7420::setConvertMode(ConvertMode mode)
{
	uint16_t reg = (REG_CONFIG & 0xff);

	reg_config &= (~MODE_BIT);
	reg_config |= (mode & MODE_BIT);
	reg |= ((reg_config << 8) & 0xff00);

	MBED_ASSERT(0 == sensor.write(i2c_addr, (char*)(&reg), 2));
}

float ADT7420::temperature()
{
	uint16_t ret = 0;
	uint16_t adc_code = 0;
	uint8_t reg = REG_TEMPH;

	MBED_ASSERT(0 == sensor.write(i2c_addr, (char*)(&reg), 1, true));
	MBED_ASSERT(0 == sensor.read(i2c_addr, (char*)(&adc_code), 2, false));

	//Little-Endian conversion
	ret |= ((adc_code << 8) & 0xff00);
	ret |= ((adc_code >> 8) & 0x00ff);

	// Debug only, see if the resolution is correct
	//printf("(%d)\r\n", (int)(ret & 7));

	if(res == RES_13BIT)
	{
		ret >>= 3;
		if(ret & (1 << 12)) // Negative temperature
			return (ret - 8192L) / 16.0f;
		else                // Positive temperature
			return ret / 16.0f;
	}

	if(res == RES_16BIT)
	{
		if(ret & (1 << 15)) // Negative temperature
			return (ret - 65536L) / 128.0f;
		else                // Positive temperature
			return ret / 128.0f;
	}

	return 0.0 / 0.0; // Return NaN
}

#ifdef MBED_OPERATORS
ADT7420::operator float()
{
    //Return the current temperature reading
    return temperature();
}
#endif

