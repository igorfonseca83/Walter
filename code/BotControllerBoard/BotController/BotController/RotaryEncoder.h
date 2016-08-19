/* 
* RotaryEncoder.h
*
* Created: 23.04.2016 23:13:21
* Author: JochenAlt
*/


#ifndef __ROTARYENCODE_H__
#define __ROTARYENCODE_H__

#include "Arduino.h"
#include "AMS_AS5048B.h"
#include "Config.h"

class RotaryEncoder
{
public:
	RotaryEncoder() { 
		currentSensorAngle = 0;
		passedCheck = false;
		configData = NULL;
		setupData=NULL;
		communicationWorks = false;
		failedReadingCounter = 0;
	};
	void setup( RotaryEncoderConfig* config, RotaryEncoderSetupData* setupData);
	RotaryEncoderConfig& getConfig() { return *configData;};
	void setNullAngle(float angle);
	float getNullAngle();

	float getAngle();
	float getRawSensorAngle();

	bool getNewAngleFromSensor();
	bool fetchSample(bool raw,float& avr, float& variance);

	float checkEncoderVariance();
	bool isOk() {
		return communicationWorks & passedCheck & (failedReadingCounter < 8);;
	}
	static void switchConflictingSensor(bool powerOn);

private:
	bool fetchSample(bool raw,uint8_t no, float sample[], float& avr, float& variance);
	bool isClockwise() {return setupData->clockwise;}
	uint8_t i2CAddress(bool after) {
		if (after)
			return setupData->I2CAddress + I2C_ADDRESS_ADDON;
		else
			return setupData->I2CAddress;
	}
	
	bool doProgI2CAddress() {
		return setupData->programmI2CAddress;
	}

	AMS_AS5048B sensor;
	float currentSensorAngle;
	RotaryEncoderSetupData* setupData;
	RotaryEncoderConfig* configData;
	bool passedCheck;
	bool communicationWorks;
	uint8_t failedReadingCounter;
}; //RotaryEncode

#endif //__ROTARYENCODE_H__
