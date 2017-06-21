#ifndef __ATTITUDE_H_
#define __ATTITUDE_H_

#include "Vector3.h"
#include "Matrix3.h"

#define RtA 		57.324841f	//角度转化为弧度	
#define AtR    	0.0174533f  //弧度转化为角度
 
class AttitudeCalculation{

	private:
		
	Vector3f mAngle;
	Matrix3<float> mRotateMatrix;
	float q0, q1, q2 , q3 ;	    /** quaternion of sensor frame relative to auxiliary frame */
	float dq0, dq1, dq2 , dq3;	/** quaternion of sensor frame relative to auxiliary frame */
	float gyro_bias[3]; /** bias estimation */
	float q0q0, q0q1, q0q2, q0q3;
	float q1q1, q1q2, q1q3;
	float q2q2, q2q3;
	float q3q3;
	unsigned char bFilterInit;
	
	public:
		AttitudeCalculation()
		{
			q0 = 1.0f;
		}
		
		void NonlinearSO3AHRSinit(float ax, float ay, float az, float mx, float my, float mz);
		void NonlinearSO3AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float twoKp, float twoKi, float dt) ;
		Vector3f GetAngle(Vector3<int> acc, Vector3<float> gyro,float deltaT);
		Vector3f GetAngle(Vector3<int> acc, Vector3<float> gyro,Vector3<int> mag,float deltaT);
		
};




#endif
