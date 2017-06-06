#ifndef _ESC_MOTOR_H_
#define _ESC_MOTOR_H_

#include "stm32f10x.h"
#include "PWM.h"


class ESCMotor
{
private:
	PWM &_tim;
	u8  _ch;
	float _maxDuty;  //0.0% ~ 100.0%
	float _minDuty;  //0.0% ~ 100.0%
	float _speed;    //0.0% ~ 100.0%
public:
	ESCMotor(PWM &tim,u8 ch);
	void SetDutyRange(float maxDuty, float minDuty);
	void Speed(float rate);
  
};




#endif


