#ifndef _POSITION_LOOP_H_
#define _POSITION_LOOP_H_

#include "stm32f10x.h"
#include "mathtool.h"
#include "PIDParameter.h"

class PositionLoop
{
private:
	const CraftVector &_curAngle;   //current angle
	const CraftVector &_tagAngle;   //target angle
	CraftVector &_tagAngVel;  //target angular velocity

	PIDParameter pidRoll;
	PIDParameter pidPitch;
	PIDParameter pidYaw;
  PIDParameter pidThrottle;
public:
	PositionLoop(const CraftVector &curAngle, const CraftVector &tagAngle, CraftVector &tagAngVel);	

	void Roll();
	void Pitch();
	void Yaw();
	void Throttle();

	void Set_PIT_PID(float kp=0, float ki=0, float kd=0);
	void Set_ROL_PID(float kp=0, float ki=0, float kd=0);
	void Set_YAW_PID(float kp=0, float ki=0, float kd=0);
};










#endif
