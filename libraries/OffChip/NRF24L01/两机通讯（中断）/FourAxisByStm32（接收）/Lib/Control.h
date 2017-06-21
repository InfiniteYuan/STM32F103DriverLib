#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "stm32f10x.h"
#include "flash.h"
#include "Vector3.h"
#include "TaskManager.h"
#include "F103_PWM.h"
#include "math.h"


//#define DUBUG_PITCH
//#define DUBUG_ROLL
#define NORMAL

// PID结构体
typedef struct
{
	float P;
	float I;
	float D;
	float Desired; //期望
	float Error; //误差
	float LastError; //上一次的误差
	float CumulativeError;//积分累计误差
	float Proportion; //比例项
	float Integral;//积分项
	float Differential;//微分项
	float iLimit;//积分限制
	float Output;//最后输出
	float OLimit;//输出限幅

}PID_Typedef;



class Control{
private:

	PWM &mMoto;
	double TimeInterval;
	double OldTime;
	float FlyThr; //起飞油门量 经验值，用于区分使用PID的时间

	PID_Typedef pitch_angle_PID;	//pitch角度环的PID
	PID_Typedef pitch_rate_PID;		//pitch角速率环的PID

	PID_Typedef roll_angle_PID;   //roll角度环的PID
	PID_Typedef roll_rate_PID;    //roll角速率环的PID

	PID_Typedef yaw_angle_PID;    //yaw角度环的PID 
	PID_Typedef yaw_rate_PID;     //yaw角速率环的PID

	//增量式PID工具
	//参数：需要更新的PID结构体，目标位置，当前位置，时间间隔
	bool TOOL_PID_Postion_Cal(PID_Typedef * PID, float target, float measure, float Thr, double dertT);
public:
	Control(PWM &Moto);
	bool ReadPID(flash info, u16 Page, u16 position);
	bool SavePID(flash info, u16 Page, u16 position);

	//SET------------------------------------------------
	bool SetPID_ROL(float P, float I, float D)
	{
		roll_angle_PID.P = P;
		roll_angle_PID.I = I;
		roll_angle_PID.D = D;
		return true;
	}

	bool SetPID_PIT(float P, float I, float D)
	{
		pitch_angle_PID.P = P;
		pitch_angle_PID.I = I;
		pitch_angle_PID.D = D;
		return true;
	}

	bool SetPID_YAW(float P, float I, float D)
	{
		yaw_angle_PID.P = P;
		yaw_angle_PID.I = I;
		yaw_angle_PID.D = D;
		return true;
	}
	//角速度环
	bool SetPID_ROL_rate(float P, float I, float D)
	{
		roll_rate_PID.P = P;
		roll_rate_PID.I = I;
		roll_rate_PID.D = D;
		return true;
	}

	bool SetPID_PIT_rate(float P, float I, float D)
	{
		pitch_rate_PID.P = P;
		pitch_rate_PID.I = I;
		pitch_rate_PID.D = D;
		return true;
	}

	bool SetPID_YAW_rate(float P, float I, float D)
	{
		yaw_rate_PID.P = P;
		yaw_rate_PID.I = I;
		yaw_rate_PID.D = D;
		return true;
	}

	//GET-----------------------------------------------
	PID_Typedef GetPID_ROL()
	{
		return roll_angle_PID;
	}

	PID_Typedef GetPID_YAW()
	{
		return yaw_angle_PID;
	}

	PID_Typedef GetPID_PIT()
	{
		return pitch_angle_PID;
	}

	PID_Typedef GetPID_ROL_rate()
	{
		return roll_rate_PID;
	}

	PID_Typedef GetPID_YAW_rate()
	{
		return yaw_rate_PID;
	}

	PID_Typedef GetPID_PIT_rate()
	{
		return pitch_rate_PID;
	}

	//传入： 当前角度/陀螺仪的角速度/遥控器量
	bool PIDControl(Vector3f angle, Vector3<float> gyr, u16 RcThr, u16 RcPit, u16 RcRol, u16 PcYaw);
	//串级PID
	bool SeriesPIDComtrol(Vector3f angle, Vector3<float> gyr, u16 RcThr, u16 RcPit, u16 RcRol, u16 PcYaw);


};


#endif
