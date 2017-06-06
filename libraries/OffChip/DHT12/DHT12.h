#ifndef _DHT21_H_
#define _DHT21_H_

#include "stm32f10x.h"
#include "SoftwareI2C.h"
#include "I2C.h"

#define DHT12_ADDRESS   0xB8

class DHT12:public Sensor{

private:
	float V_Humid;
	float V_Temper;
	uint8_t readData[5];
public:
	DHT12();
	void Update(void);
	void DHT12_ReadByte(void);
	uint16_t GetHumid(void);
	uint16_t GetTemper(void);
};

#endif
