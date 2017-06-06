#ifndef _COMPASS_H_
#define _COMPASS_H_

#include "stm32f10x.h"
#include "Mathtool.h"
#include "Sensor.h"

class Compass : public Sensor
{
protected:
		bool _mag_calibrating;
		Vector3f mOffsetRatio;
		Vector3f mOffsetBias;
		unsigned char mHealth;
public:
	virtual bool Initialize()=0;
	virtual bool Update(Vector3f &mag)=0;

	void StartMagCalibrating(){_mag_calibrating = true;}
	void StopMagCalibrating(){_mag_calibrating = false;}
	bool MagCalibrating(){ return _mag_calibrating;}
	
	void SetOffsetBias(Vector3f bias){ mOffsetBias = bias;}
	Vector3f GetOffsetBias(){ return mOffsetBias;}
	
	void SetOffsetRatio(Vector3f ratio){ mOffsetRatio = ratio;}
	Vector3f GetOffsetRatio(){ return mOffsetRatio;}
};


#endif

