
#include "PositionLoop.h"

PositionLoop::PositionLoop(const CraftVector &curAngle, const CraftVector &tagAngle, CraftVector &tagAngVel)
	:_curAngle(curAngle),_tagAngle(tagAngle),_tagAngVel(tagAngVel)
{
	//       Kp    Ki   Kd    dt     
	pidPitch(7,    0.0,  0.0, 0.004,  15,  10);
	pidRoll(5,    0.0,  0.1, 0.002,  15,  10);
	pidYaw(5,    0.0,  0.1, 0.002,  15,  10);
}

void PositionLoop::Roll()
{
//	_tagAngVel.roll = pidRoll.ComputePID(_tagAngle.roll - _curAngle.roll);
}

void PositionLoop::Pitch()
{
	_tagAngVel.pitch = pidPitch.ComputePID(_tagAngle.pitch , _curAngle.pitch);
}

void PositionLoop::Yaw()
{
//	_tagAngVel.yaw = pidYaw.ComputePID(_tagAngle.yaw - _curAngle.yaw);
}

void PositionLoop::Throttle()
{
	
}

void PositionLoop::Set_PIT_PID(float kp, float ki, float kd)
{
	pidPitch.Set_PID(kp, ki, kd);
}

void PositionLoop::Set_ROL_PID(float kp, float ki, float kd)
{
	pidRoll.Set_PID(kp, ki, kd);
}

void PositionLoop::Set_YAW_PID(float kp, float ki, float kd)
{
	pidYaw.Set_PID(kp, ki, kd);
}

