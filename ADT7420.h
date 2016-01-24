/*
 * ADT7420.h
 *
 *  Created on: 2016Äê1ÔÂ22ÈÕ
 *      Author: KuangQi
 */

#ifndef ADT7420_H_
#define ADT7420_H_

#include "mbed.h"

class ADT7420 {
public:
	enum I2CAddress
	{
		ADDRESS0 = (0x48 << 1),
		ADDRESS1 = (0x49 << 1),
		ADDRESS2 = (0x4A << 1),
		ADDRESS3 = (0x4B << 1)
	};

    enum Resolution
	{
        RES_13BIT = (0 << 7),
		RES_16BIT = (1 << 7)
    };

    enum ConvertMode
	{
    	MODE_CONT     = (0 << 5),
		MODE_ONE_SHOT = (1 << 5),
		MODE_1SPS     = (2 << 5),
		MODE_SHUTDOWN = (3 << 5)
    };

	ADT7420(PinName sda, PinName scl, I2CAddress addr = ADDRESS0, int freq = 100000);
	bool isOK();
	bool dataAvailable();
	int readID();
	void setResolution(Resolution resolution);
	void setConvertMode(ConvertMode mode);
	float temperature();

#ifdef MBED_OPERATORS
    /** A shorthand for temperature()
     *
     * @returns The current temperature measurement in ¡ãC.
     */
    operator float();
#endif

private:
    enum Register {
        REG_TEMPH   = 0x00,
		REG_TEMPL   = 0x01,
		REG_STATUS  = 0x02,
        REG_CONFIG  = 0x03,
        REG_THYST   = 0x0A,
        REG_ID      = 0x0B
    };

	I2C sensor;
	Resolution res;

	uint8_t i2c_addr;
	uint8_t reg_config = 0;

    const uint8_t READY_BIT = (1 << 7);
    const uint8_t RESOL_BIT = (1 << 7);
    const uint8_t MODE_BIT  = (3 << 6);
};

#endif /* ADT7420_H_ */
