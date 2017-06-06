#include "ESCMotor.h"


ESCMotor::ESCMotor(PWM &tim, u8 ch):_tim(tim),_ch(ch)
{
	
}
void ESCMotor::SetDutyRange(float maxDuty, float minDuty)
{
	_maxDuty = maxDuty;
	_minDuty = minDuty;
}

void ESCMotor::Speed(float VelocityRate)
{
	_tim.SetDuty(_ch, _minDuty + VelocityRate*(_maxDuty - _minDuty)/100.0f);
}


