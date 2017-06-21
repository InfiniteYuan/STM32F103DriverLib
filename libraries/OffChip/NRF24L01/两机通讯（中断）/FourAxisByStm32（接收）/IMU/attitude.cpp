#include "attitude.h"


void AttitudeCalculation::NonlinearSO3AHRSinit(float ax, float ay, float az, float mx, float my, float mz)
{
		float initialRoll, initialPitch;
		float cosRoll, sinRoll, cosPitch, sinPitch;
		float magX, magY;
		float initialHdg, cosHeading, sinHeading;

		q0 =1;
		//计算绕x轴旋转pitch 和绕y轴旋转yaw  此工程中绕x轴为ROll 绕y 为pitch
		//计算cos 旋转角
		initialRoll = atan2(-ay, -az);
		initialPitch = atan2(ax, -az);

		cosRoll = cosf(initialRoll);
		sinRoll = sinf(initialRoll);
		cosPitch = cosf(initialPitch);
		sinPitch = sinf(initialPitch);

		magX = mx * cosPitch + my * sinRoll * sinPitch + mz * cosRoll * sinPitch;

		magY = my * cosRoll - mz * sinRoll;

		initialHdg = atan2f(-magY, magX);
		//计算cos 二分之一旋转角
		cosRoll = cosf(initialRoll * 0.5f);
		sinRoll = sinf(initialRoll * 0.5f);

		cosPitch = cosf(initialPitch * 0.5f);
		sinPitch = sinf(initialPitch * 0.5f);

		cosHeading = cosf(initialHdg * 0.5f);
		sinHeading = sinf(initialHdg * 0.5f);

		q0 = cosRoll * cosPitch * cosHeading + sinRoll * sinPitch * sinHeading;
		q1 = sinRoll * cosPitch * cosHeading - cosRoll * sinPitch * sinHeading;
		q2 = cosRoll * sinPitch * cosHeading + sinRoll * cosPitch * sinHeading;
		q3 = cosRoll * cosPitch * sinHeading - sinRoll * sinPitch * cosHeading;

		// auxillary variables to reduce number of repeated operations, for 1st pass
		q0q0 = q0 * q0;
		q0q1 = q0 * q1;
		q0q2 = q0 * q2;
		q0q3 = q0 * q3;
		q1q1 = q1 * q1;
		q1q2 = q1 * q2;
		q1q3 = q1 * q3;
		q2q2 = q2 * q2;
		q2q3 = q2 * q3;
		q3q3 = q3 * q3;
}


void AttitudeCalculation::NonlinearSO3AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float twoKp, float twoKi, float dt)
{
float recipNorm;
		float halfex = 0.0f, halfey = 0.0f, halfez = 0.0f;

		// Make filter converge to initial solution faster
		// This function assumes you are in static position.
		// WARNING : in case air reboot, this can cause problem. But this is very unlikely happen.
		if(bFilterInit == 0) {
			NonlinearSO3AHRSinit(ax,ay,az,mx,my,mz);
			bFilterInit = 1;
		}
				
		//! If magnetometer measurement is available, use it.
		if(!((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))) {
			float hx, hy, hz, bx, bz;
			float halfwx, halfwy, halfwz;
		
			// Normalise magnetometer measurement
			// Will sqrt work better? PX4 system is powerful enough?
			//磁力计归一化
			recipNorm = 1.0 / sqrt(mx * mx + my * my + mz * mz);
			mx *= recipNorm;
			my *= recipNorm;
			mz *= recipNorm;
		
			// Reference direction of Earth's magnetic field参考地球磁场的方向
			//将载体坐标系变为地球坐标系
			hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
			hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
			hz = 2.0f * mx * (q1q3 - q0q2) + 2.0f * my * (q2q3 + q0q1) + 2.0f * mz * (0.5f - q1q1 - q2q2);
			
			//算出北方向量作为一个对比的参考量
			bx = sqrt(hx * hx + hy * hy);
			bz = hz;
		
			// Estimated direction of magnetic field  估计磁场的方向
			//将算出的参考向量变回到集体坐标系
			halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
			halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
			halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);
						
		
			// Error is sum of cross product between estimated direction and measured direction of field vectors
			//对比参考向量和姿态向量的
			halfex += (my * halfwz - mz * halfwy);
			halfey += (mz * halfwx - mx * halfwz);
			halfez += (mx * halfwy - my * halfwx);
			
		}

		//增加一个条件：  加速度的模量与G相差不远时。 0.75*G < normAcc < 1.25*G
		// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
		if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) 	
		{
			float halfvx, halfvy, halfvz;
		
			// Normalise accelerometer measurement
			//归一化，得到单位加速度
			recipNorm = 1.0 / sqrt(ax * ax + ay * ay + az * az);
			//【ax,ay,az】与[halfvx,halfvy,halfvz]都表示垂直向下的向量
			//做向量积得到误差[halfex,halfey,halfez] 利用误差来修正bcn矩阵
			ax *= recipNorm;
			ay *= recipNorm;
			az *= recipNorm;

			// 估计重力和磁场的方向  上一次四元素在机体坐标系下换算出来的重力的单位向量
			halfvx = q1q3 - q0q2;
			halfvy = q0q1 + q2q3;
			halfvz = q0q0 - 0.5f + q3q3;
		
			// Error is sum of cross product between estimated direction and measured direction of field vectors
			//向量乘积
			halfex += ay * halfvz - az * halfvy;
			halfey += az * halfvx - ax * halfvz;
			halfez += ax * halfvy - ay * halfvx;
		}

		// Apply feedback only when valid data has been gathered from the accelerometer or magnetometer
		if(halfex != 0.0f && halfey != 0.0f && halfez != 0.0f) {
			// Compute and apply integral feedback if enabled
			if(twoKi > 0.0f) {
				gyro_bias[0] += twoKi * halfex * dt;
				gyro_bias[1] += twoKi * halfey * dt;
				gyro_bias[2] += twoKi * halfez * dt;
				
				gx += gyro_bias[0];
				gy += gyro_bias[1];
				gz += gyro_bias[2];
			}
			else {
				gyro_bias[0] = 0.0f;	// prevent integral windup
				gyro_bias[1] = 0.0f;
				gyro_bias[2] = 0.0f;
			}

			// Apply proportional feedback
			gx += twoKp * halfex;
			gy += twoKp * halfey;
			gz += twoKp * halfez;

		}
		
		// Time derivative of quaternion. q_dot = 0.5*q\otimes omega.
		//! q_k = q_{k-1} + dt*\dot{q}
		//! \dot{q} = 0.5*q \otimes P(\omega)
		dq0 = 0.5f*(-q1 * gx - q2 * gy - q3 * gz);
		dq1 = 0.5f*(q0 * gx + q2 * gz - q3 * gy);
		dq2 = 0.5f*(q0 * gy - q1 * gz + q3 * gx);
		dq3 = 0.5f*(q0 * gz + q1 * gy - q2 * gx); 

		q0 += dt*dq0;
		q1 += dt*dq1;
		q2 += dt*dq2;
		q3 += dt*dq3;
		
		// Normalise quaternion
		recipNorm = 1.0 / sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
		q0 *= recipNorm;
		q1 *= recipNorm;
		q2 *= recipNorm;
		q3 *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		q0q0 = q0 * q0;
		q0q1 = q0 * q1;
		q0q2 = q0 * q2;
		q0q3 = q0 * q3;
		q1q1 = q1 * q1;
		q1q2 = q1 * q2;
		q1q3 = q1 * q3;
		q2q2 = q2 * q2;
		q2q3 = q2 * q3;
		q3q3 = q3 * q3;   
}

Vector3f AttitudeCalculation::GetAngle(Vector3<int> acc, Vector3<float> gyro,float deltaT)
{
		NonlinearSO3AHRSupdate(gyro.x,gyro.y,gyro.z,acc.x,acc.y,acc.z,0,0,0,1,0.05,deltaT);
		 mAngle.y= asin(-2 * q1 * q3 + 2 * q0* q2)* RtA; // pitch
		 mAngle.x= atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* RtA; // roll
		 mAngle.z = atan2(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3))* RtA;//yaw
		return mAngle;
}


Vector3f AttitudeCalculation::GetAngle(Vector3<int> acc, Vector3<float> gyro,Vector3<int> mag,float deltaT)
{
		NonlinearSO3AHRSupdate(gyro.x,gyro.y,gyro.z,acc.x,acc.y,acc.z,mag.x,mag.y,mag.z,100,0.05,deltaT);
		
		 mAngle.y= asin(-2 * q1 * q3 + 2 * q0* q2)* RtA; // pitch
		 mAngle.x= atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* RtA; // roll
		 mAngle.z = atan2(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3))* RtA;//yaw
		return mAngle;
}
