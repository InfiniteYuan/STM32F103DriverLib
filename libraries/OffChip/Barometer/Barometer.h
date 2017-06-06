#ifndef _BAROMETER_H_
#define _BAROMETER_H_

#include "stm32f10x.h"
#include "Sensor.h"

class Barometer: public Sensor
{
public:
	virtual bool Initialize()=0;
	virtual bool Update(float &pressure)=0;
};

#endif

