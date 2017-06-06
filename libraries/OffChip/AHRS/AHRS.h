#ifndef _AHRS_H_
#define _AHRS_H_

#include "stm32f10x.h"
#include "InertialSensor.h"
#include "Compass.h"
#include "Barometer.h"

class AHRS
{
protected:
	InertialSensor &_ins;      //inertial sensor
	Compass *_compass;         //compass
	Barometer *_baro;          //barometer

	Vector3f _acc;             //3-aixs acceleration data
	Vector3f _gyro;            //3-aixs gyroscope data
	Vector3f _mag;             //3-aixs compass magnetic
	float _pressure;           //air pressure

	float _roll;
	float _pitch;
	float _yaw;
	//Quaternion _q;             //quternion: q1,q2,q3,q4
public:
	AHRS(InertialSensor &ins,Compass *compass=0, Barometer *baro=0):_ins(ins),_compass(compass),_baro(baro){};
	virtual bool Update()=0;             //update inertial sensor data
  Vector3f GetAcc()  {return _acc; }         // get acceleration data
	Vector3f GetGyro() {return _gyro;}        //get gyroscope data
	Vector3f GetMag()  {return _mag;}   
	float GetPressure(){return _pressure;}
	float Roll() { return _roll; }
	float Pitch() { return _pitch;}
	float Yaw() { return _yaw;}
	 virtual bool Ready()=0;
};

#endif

