#include "VelocityLoop.h"


VelocityLoop::VelocityLoop(const CraftVector &curAngVel, const CraftVector &tagAngVel, CraftVector &velCtrlVal)
	:_curAngVel(curAngVel),_tagAngVel(tagAngVel),_velCtrlVal(velCtrlVal)
{
	//       Kp    Ki   Kd    dt     
	pidPitch(5,    0.0,  0.1, 0.002,  15,  10);
	pidRoll(5,    0.0,  0.1, 0.002,  15,  10);
	pidYaw(5,    0.0,  0.1, 0.002,  15,  10);
}

void VelocityLoop::Roll()
{
//	_velCtrlVal.roll = pidRoll.ComputePID(_tagAngVel.roll, _curAngVel.roll);
}

void VelocityLoop::Pitch()
{
	_velCtrlVal.pitch = pidPitch.ComputePID(_tagAngVel.pitch, _curAngVel.pitch);
}

void VelocityLoop::Yaw()
{
//	_velCtrlVal.yaw = pidYaw.ComputePID(_tagAngVel.yaw, _curAngVel.yaw);
}

void VelocityLoop::Throttle()
{
	
}

void VelocityLoop::Set_PIT_PID(float kp, float ki, float kd)
{
	pidPitch.Set_PID(kp, ki, kd);
}

void VelocityLoop::Set_ROL_PID(float kp, float ki, float kd)
{
	pidRoll.Set_PID(kp, ki, kd);
}

void VelocityLoop::Set_YAW_PID(float kp, float ki, float kd)
{
	pidYaw.Set_PID(kp, ki, kd);
}

