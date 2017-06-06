#ifndef __AHRS_Quater_H_
#define __AHRS_Quater_H_

#include "Vector3.h"
#include "Matrix3.h"
#include "AHRS.h"
#include "Mathtool.h"

#define RtA 		57.324841f	//弧度转化为角度
#define AtR    	0.0174533f  //角度转化为弧度	

#define so3_comp_params_Kp  1.0f
#define so3_comp_params_Ki  0.05f

struct MPU6050Filter_tag			// IMU滤波后的值
{
	Vector3<int> acc;
	Vector3f gyro;
	s16 accel_x_f;	// ¼ÓËÙ¶È¼ÆxÂË²¨ºóµÄÖµ
	s16 accel_y_f;	// ¼ÓËÙ¶È¼ÆyÂË²¨ºóµÄÖµ
	s16 accel_z_f;	// ¼ÓËÙ¶È¼ÆzÂË²¨ºóµÄÖµ
	s16 gyro_x_c;	// ÍÓÂÝÒÇ±ê¶¨ºóµÄÖµ
	s16 gyro_y_c;	// ÍÓÂÝÒÇ±ê¶¨ºóµÄÖµ
	s16 gyro_z_c;	// ÍÓÂÝÒÇ±ê¶¨ºóµÄÖµ
};
 
class AHRS_Quater:public AHRS{

	private:
		
	Vector3f mAngle;
	Matrix3<float> mRotateMatrix;
	MPU6050Filter_tag g_MPU6050Data_Filter;
	float q0, q1, q2 , q3 ;	    /** quaternion of sensor frame relative to auxiliary frame */
	float dq0, dq1, dq2 , dq3;	/** quaternion of sensor frame relative to auxiliary frame */
	float gyro_bias[3]; /** bias estimation */
	float q0q0, q0q1, q0q2, q0q3;
	float q1q1, q1q2, q1q3;
	float q2q2, q2q3;
	float q3q3;
	unsigned char bFilterInit;
	
	public:
		AHRS_Quater(InertialSensor &ins,Compass *compass=0, Barometer *baro=0):AHRS(ins,compass,baro)
		{
			q0 = 1.0f;
			gyro_bias[0] = 0.0f;
			gyro_bias[1] = 0.0f;
			gyro_bias[2] = 0.0f;
		}
		virtual bool Update(); 
		virtual bool Ready();
		void IMU_Filter();
		void ToEuler(float *roll, float *pitch, float *yaw) 
		{//弧度
			*pitch = asin(-2 * q1 * q3 + 2 * q0* q2); // pitch  * RtA
			*roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1); // roll   * RtA
			*yaw = atan2(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3));//yaw   * RtA
		}
		void NonlinearSO3AHRSinit(float ax, float ay, float az, float mx, float my, float mz);
		void NonlinearSO3AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float twoKp, float twoKi, float dt) ;
		Vector3f GetAngle(Vector3<int> acc, Vector3<float> gyro,float deltaT);
		Vector3f GetAngle(Vector3<int> acc, Vector3<float> gyro,Vector3<int> mag,float deltaT);
		
};


#endif
