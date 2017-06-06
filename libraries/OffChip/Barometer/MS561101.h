#ifndef _MS561101_H_
#define _MS561101_H_

#include "stm32f10x.h"
#include "Barometer.h"
#include "I2C.h"

class MS561101: public Barometer
{
protected:
	I2C &mI2C;
	u8  mRomData[12];
	u16 mC[6];
	bool mConvertPressure;
	u8  mRawT[3];
	u8  mRawP[3];
public:
	MS561101(I2C &i2c);
	virtual bool Initialize();
	virtual bool Update(float &pressure);
	float Temperature();
	float Pressure();
};

//#define MS561101BA_Pin_CSB_HIGH

#ifdef MS561101BA_Pin_CSB_HIGH
	#define MS561101BA_ADDR 0xEC
#else
	#define MS561101BA_ADDR 0xEE
#endif


// registers of the device
#define MS561101BA_D1 0x40
#define MS561101BA_D2 0x50
#define MS561101BA_RESET 0x1E
#define MS561101BA_D1D2_SIZE 3	// D1 and D2 result size (bytes)

// OSR (Over Sampling Ratio) constants
#define MS561101BA_OSR_256 0x00
#define MS561101BA_OSR_512 0x02
#define MS561101BA_OSR_1024 0x04
#define MS561101BA_OSR_2048 0x06
#define MS561101BA_OSR_4096 0x08

#define MS561101BA_PROM_BASE_ADDR 0xA2 // by adding ints from 0 to 6 we can read all the prom configuration values. 
// C1 will be at 0xA2 and all the subsequent are multiples of 2
#define MS561101BA_PROM_REG_COUNT 6 // number of registers in the PROM
#define MS561101BA_PROM_REG_SIZE 2 // size in bytes of a prom registry.

#endif




