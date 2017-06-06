#ifndef _INERTIAL_SENSOR_H_
#define _INERTIAL_SENSOR_H_

#include "stm32f10x.h"
#include "Mathtool.h"
#include "Sensor.h"

class InertialSensor : public Sensor
{
	protected:
		Vector3<s16> _acc_offset;  //acc offset
		Vector3<s16> _gyro_offset; //gyro offset
	  Vector3<int16_t> _acc_raw;
	  Vector3<int16_t> _gyro_raw;
	  Vector3f _acc_g;
	  Vector3f _gyro_rad;
	
	
		bool _gyro_calibrating;
		Vector3<int> _gyro_calibrate_sum;  //sum of gyro for average aclibrate
	  u16 _gyro_calibrate_cnt;       //count of sum gyro data
	
	
	public:
		virtual bool Initialize(void) = 0;
		virtual bool Update(Vector3f &acc, Vector3f &gyro)=0;
		s16 AccRawX() { return _acc_raw.x; }
		s16 AccRawY() { return _acc_raw.y; }
		s16 AccRawZ() { return _acc_raw.z; }
		s16 GyroRawX() { return _gyro_raw.x; }
		s16 GyroRawY() { return _gyro_raw.y; }
		s16 GyroRawZ() { return _gyro_raw.z; }	
		Vector3<s16> AccRaw() { return _acc_raw;}
		Vector3<s16> GyroRaw() { return _gyro_raw;}
		float AccX() { return _acc_g.x; }
		float AccY() { return _acc_g.y; }
		float AccZ() { return _acc_g.z; }
		float GyroX() { return _gyro_rad.x; }
		float GyroY() { return _gyro_rad.y; }
		float GyroZ() { return _gyro_rad.z; }
		Vector3f Acc() { return _acc_g; }
		Vector3f Gyro() { return _gyro_rad; }
		
		Vector3<s16> AccOffset() { return _acc_offset;}
		Vector3<s16> GyroOffset() { return _gyro_offset;}
		void SetAccOffset(Vector3<s16> accOff) { _acc_offset = accOff; }
		void SetGyroOffset(Vector3<s16> gyroOff) { _gyro_offset = gyroOff;}
		void StartGyroCalibrating()
    { 
			_gyro_calibrating = true;
			_gyro_calibrate_cnt = 0;
			_gyro_calibrate_sum.Zero();
		} 
		void StopGyroCalibrating()  { _gyro_calibrating = false;} 
		bool GyroCalibrating()  { return _gyro_calibrating; }
		bool Ready() { return !GyroCalibrating(); }
};


#endif

