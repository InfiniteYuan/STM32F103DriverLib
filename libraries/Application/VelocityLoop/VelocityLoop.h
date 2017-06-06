#ifndef _VELOCITY_LOOP_H_
#define _VELOCITY_LOOP_H_

#include "stm32f10x.h"
#include "mathtool.h"
#include "PIDParameter.h"

class VelocityLoop
{
private:
	const CraftVector &_curAngVel;  //current angular velocity
	const CraftVector &_tagAngVel;  //target angular velocity
	CraftVector &_velCtrlVal; //control value of velocity loop

	PIDParameter pidRoll;
	PIDParameter pidPitch;
	PIDParameter pidYaw;
  PIDParameter pidThrottle;
public:
	VelocityLoop(const CraftVector &curAngVel, const CraftVector &tagAngVel, CraftVector &velCtrlVal);

	void Roll();
	void Pitch();
	void Yaw();
	void Throttle();

	void Set_PIT_PID(float kp=0, float ki=0, float kd=0);
	void Set_ROL_PID(float kp=0, float ki=0, float kd=0);
	void Set_YAW_PID(float kp=0, float ki=0, float kd=0);

	void Set_RATE_PIT_PID(float kp=0, float ki=0, float kd=0);
	void Set_RATE_ROL_PID(float kp=0, float ki=0, float kd=0);
	void Set_RATE_YAW_PID(float kp=0, float ki=0, float kd=0);
};






#endif
