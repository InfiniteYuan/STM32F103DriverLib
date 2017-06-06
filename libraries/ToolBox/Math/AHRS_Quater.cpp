#include "AHRS_Quater.h"

//---------------------------------------------------------------------------------------------------
// Fast inverse square-root
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root

float invSqrt(float x) {
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

bool AHRS_Quater::Update(){
	if(_compass)
		_compass->Update(_mag);
	if(_baro)
		_baro->Update(_pressure);
	if(_ins.Update(_acc,_gyro)){
//		IMU_Filter(); //8深度 滑动窗口滤波
		if(_compass)
			NonlinearSO3AHRSupdate(_gyro.x,_gyro.y,_gyro.z,_acc.x,_acc.y,_acc.z,_mag.x,_mag.y,_mag.z,so3_comp_params_Kp,so3_comp_params_Ki,_ins.Interval());
		else
			NonlinearSO3AHRSupdate(_gyro.x,_gyro.y,_gyro.z,_acc.x,_acc.y,_acc.z,0,0,0,so3_comp_params_Kp,so3_comp_params_Ki,_ins.Interval());
	}
	ToEuler(&_roll,&_pitch,&_yaw);
	return true;
}

bool AHRS_Quater::Ready(){
	return _ins.Ready();
}

void AHRS_Quater::NonlinearSO3AHRSinit(float ax, float ay, float az, float mx, float my, float mz)
{
		float initialRoll, initialPitch;
		float cosRoll, sinRoll, cosPitch, sinPitch;
		float magX, magY;
		float initialHdg, cosHeading, sinHeading;

		q0 =1;
		//计算绕x轴旋转pitch 和绕y轴旋转yaw 
		//计算cos 旋转角  atan2（x，y）复数 x+yi 的辐角。 返回值单位弧度
		initialRoll = atan2(-ay, -az);//计算YOZ平面，∠AOY的角度 绕X轴的角度
		initialPitch = atan2(ax, -az);//计算YOZ平面，∠AOY的角度 绕Y轴的角度
	
		//求弧度值的余弦值
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

		//欧拉角到四元素的转换
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


void AHRS_Quater::NonlinearSO3AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float twoKp, float twoKi, float dt)
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
			//【ax,ay,az】与[halfvx,halfvy,halfvz]都表示垂直向下的向量 都是机体坐标系上的，那么误差也是在机体坐标系，
			//做向量积得到误差[halfex,halfey,halfez] 利用误差来修正bcn矩阵
			//ax,ay,az是机体坐标参考系上，加速度测出来的重力向量
			ax *= recipNorm;
			ay *= recipNorm;
			az *= recipNorm;

			// 估计重力和磁场的方向  上一次四元素在机体坐标系下换算出来的重力的单位向量
			// halfvx,halfvy,halfvz,是陀螺仪积分后的姿态推算出来的重力向量
			//带误差的加速度计向量转到与重量向量重合，需要绕什么轴，转多少度
			//这个叉积在机体三轴上的投影，就是加速度计和重力之间的角度误差，
			//也就是，如果陀螺仪按照这个叉积误差的轴，转动叉积误差的角度（转动三轴投影的角度），
			//就能把加速度计向量和重力向量的误差除掉
			//如果完全按照叉积误差转，那就是完全信任加速度计，如果一点也不转，那就是完全信任陀螺仪
			//那么吧这个叉积的三轴乘以，加到陀螺仪的积分角度上，就是x互补系数的互补算法
			halfvx = q1q3 - q0q2;
			halfvy = q0q1 + q2q3;
			halfvz = q0q0 - 0.5f + q3q3;
		
			// Error is sum of cross product between estimated direction and measured direction of field vectors
			//上面两个向量的叉积就是陀螺仪积分后的姿态和加速度计测出来的姿态之间的误差，
			//大小正好和陀螺仪积分误差成正比，用来修正陀螺仪
			halfex += ay * halfvz - az * halfvy;
			halfey += az * halfvx - ax * halfvz;
			halfez += ax * halfvy - ay * halfvx;
		}

		// Apply feedback only when valid data has been gathered from the accelerometer or magnetometer
		if(halfex != 0.0f && halfey != 0.0f && halfez != 0.0f) {
			// Compute and apply integral feedback if enabled
			if(twoKi > 0.0f) {
				//用叉积误差来做PI修正陀螺零偏 dt为测量周期，
				//积分求误差,关于当前姿态分离出的重力分量,与当前加速度计测得的重力分量的差值进行积分消除误差
				//Kp 加速度权重，越大则向加速度测量值收敛越快
				//Ki 误差积分增益
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
			//用叉积误差来做PI修正陀螺零偏
			gx += twoKp * halfex;
			gy += twoKp * halfey;
			gz += twoKp * halfez;

		}
		
		// Time derivative of quaternion. q_dot = 0.5*q\otimes omega.
		//! q_k = q_{k-1} + dt*\dot{q}
		//! \dot{q} = 0.5*q \otimes P(\omega)
		//gx,gy,gz 为陀螺仪的角速度
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

Vector3f AHRS_Quater::GetAngle(Vector3<int> acc, Vector3<float> gyro,float deltaT)
{
		NonlinearSO3AHRSupdate(gyro.x,gyro.y,gyro.z,acc.x,acc.y,acc.z,0,0,0,100,0.05,deltaT);//so3_comp_params_Kp so3_comp_params_Ki
		 mAngle.y= asinf(-2 * q1 * q3 + 2 * q0* q2)* RtA; // pitch
		 mAngle.x= atan2f(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* RtA; // roll
		 mAngle.z = atan2f(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3))* RtA;//yaw
		return mAngle;
}


Vector3f AHRS_Quater::GetAngle(Vector3<int> acc, Vector3<float> gyro,Vector3<int> mag,float deltaT)
{
		NonlinearSO3AHRSupdate(gyro.x,gyro.y,gyro.z,acc.x,acc.y,acc.z,mag.x,mag.y,mag.z,100,0.05,deltaT);
		
		 mAngle.y= asin(-2 * q1 * q3 + 2 * q0* q2)* RtA; // pitch
		 mAngle.x= atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* RtA; // roll
		 mAngle.z = atan2(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3))* RtA;//yaw
		return mAngle;
}

#define ACC_FILTER_DELAY 8	//滤波窗口大小
void AHRS_Quater::IMU_Filter(){
  Vector3f acc_filter;
	s32 resultx = 0;
    static s32 s_resulttmpx[ACC_FILTER_DELAY] = {0};
    static u8 s_bufferCounterx = 0;
    static s32 s_totalx = 0;
		
	s32 resulty = 0;
    static s32 s_resulttmpy[ACC_FILTER_DELAY] = {0};
    static u8 s_bufferCountery = 0;
    static s32 s_totaly = 0;
		
	s32 resultz = 0;
    static s32 s_resulttmpz[ACC_FILTER_DELAY] = {0};
    static u8 s_bufferCounterz = 0;
    static s32 s_totalz = 0;

	//加速度计滤波
    s_totalx -= s_resulttmpx[s_bufferCounterx];					//从总和中删除头部元素的至
    s_resulttmpx[s_bufferCounterx] = _ins.AccRaw().x;		//把采样值放到尾部
    s_totalx += _ins.AccRaw().x;                     //更新总和
    resultx = s_totalx / ACC_FILTER_DELAY;		                //计算平均值，并输入一个变量中
    s_bufferCounterx++;		                        			//更新指针
    if (s_bufferCounterx == ACC_FILTER_DELAY)		            //到达队列边界
        s_bufferCounterx = 0;
		acc_filter.x = resultx;
				
    s_totaly -= s_resulttmpy[s_bufferCountery];
    s_resulttmpy[s_bufferCountery] = _ins.AccRaw().y;
    s_totaly += _ins.AccRaw().y;
    resulty = s_totaly / ACC_FILTER_DELAY;
    s_bufferCountery++;
    if (s_bufferCountery == ACC_FILTER_DELAY)
        s_bufferCountery = 0;
		acc_filter.y = resulty;
		
    s_totalz -= s_resulttmpz[s_bufferCounterz];
    s_resulttmpz[s_bufferCounterz] = _ins.AccRaw().z;
    s_totalz += _ins.AccRaw().z;
    resultz = s_totalz / ACC_FILTER_DELAY;
    s_bufferCounterz++;
    if (s_bufferCounterz == ACC_FILTER_DELAY)
        s_bufferCounterz = 0;
		acc_filter.z = resultz;
	
//		// ÍÓÂÝÒÇ±ê¶¨-->¼õÈ¥³õÊ¼Ê±¿ÌµÄÖµ
//		g_MPU6050Data_Filter.gyro_x_c = g_MPU6050Data.gyro_x - g_Gyro_xoffset;	// ¼õÈ¥±ê¶¨»ñµÃµÄÆ«ÒÆ
//		g_MPU6050Data_Filter.gyro_y_c = g_MPU6050Data.gyro_y - g_Gyro_yoffset;
//		g_MPU6050Data_Filter.gyro_z_c = g_MPU6050Data.gyro_z - g_Gyro_zoffset;
//		
//		// ÕâÀï¿ÉÒÔÊÖ¶¯±ê¶¨,¼´ÈÃ·É»úË®Æ½Ðý×ª90¶È,ÕÒµ½Ò»¸ö²ÎÊýÊ¹µÃYaw¸ÕºÃÒ²ÊÇ90¶È
//		// ¸Ã²ÎÊýÒ²¿ÉÒÔÖ±½Ó²é¿´MPU6050µÄÊý¾ÝÊÖ²áÖÐ¸ø¶¨µÄÁ¿³ÌÔöÒæ
//		g_MPU6050Data_Filter.gyro_x_c *= GYRO_CALIBRATION_COFF;
//		g_MPU6050Data_Filter.gyro_y_c *= GYRO_CALIBRATION_COFF;
//		g_MPU6050Data_Filter.gyro_z_c *= GYRO_CALIBRATION_COFF;

}

/**/



/*

#define sampleFreq	512.0f		// sample frequency in Hz
#define betaDef		0.1f		// 2 * proportional gain

//---------------------------------------------------------------------------------------------------
// Variable definitions

volatile float beta = betaDef;								// 2 * proportional gain (Kp)
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;	// quaternion of sensor frame relative to auxiliary frame

void AHRS_Quater::MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz){
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float hx, hy;
	float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
		MadgwickAHRSupdateIMU(gx, gy, gz, ax, ay, az);
		return;
	}

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0mx = 2.0f * q0 * mx;
		_2q0my = 2.0f * q0 * my;
		_2q0mz = 2.0f * q0 * mz;
		_2q1mx = 2.0f * q1 * mx;
		_2q0 = 2.0f * q0;
		_2q1 = 2.0f * q1;
		_2q2 = 2.0f * q2;
		_2q3 = 2.0f * q3;
		_2q0q2 = 2.0f * q0 * q2;
		_2q2q3 = 2.0f * q2 * q3;
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

		// Reference direction of Earth's magnetic field
		hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
		hy = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
		_2bx = sqrt(hx * hx + hy * hy);
		_2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
		_4bx = 2.0f * _2bx;
		_4bz = 2.0f * _2bz;

		// Gradient decent algorithm corrective step
		s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q0 += qDot1 * (1.0f / sampleFreq);
	q1 += qDot2 * (1.0f / sampleFreq);
	q2 += qDot3 * (1.0f / sampleFreq);
	q3 += qDot4 * (1.0f / sampleFreq);

	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

void AHRS_Quater::MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az){
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * q0;
		_2q1 = 2.0f * q1;
		_2q2 = 2.0f * q2;
		_2q3 = 2.0f * q3;
		_4q0 = 4.0f * q0;
		_4q1 = 4.0f * q1;
		_4q2 = 4.0f * q2;
		_8q1 = 8.0f * q1;
		_8q2 = 8.0f * q2;
		q0q0 = q0 * q0;
		q1q1 = q1 * q1;
		q2q2 = q2 * q2;
		q3q3 = q3 * q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q0 += qDot1 * (1.0f / sampleFreq);
	q1 += qDot2 * (1.0f / sampleFreq);
	q2 += qDot3 * (1.0f / sampleFreq);
	q3 += qDot4 * (1.0f / sampleFreq);

	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

*/
