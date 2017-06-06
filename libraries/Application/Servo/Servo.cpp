#include "Servo.h"



Servo::Servo(ESCMotor *m1,ESCMotor *m2,ESCMotor *m3,ESCMotor *m4)
{
	_motor[0] = m1;
	_motor[1] = m2;
	_motor[2] = m3;
	_motor[3] = m4;
	
	_factor[0].roll = -1;   _factor[0].pitch = 1;   _factor[0].yaw = -1;  _factor[0].throttle = 1;
	_factor[1].roll = 1;    _factor[1].pitch = 1;   _factor[1].yaw = 1;   _factor[1].throttle = 1;
	_factor[2].roll = 1;    _factor[2].pitch = -1;  _factor[2].yaw = -1;  _factor[2].throttle = 1;
	_factor[3].roll = -1;   _factor[3].pitch = -1;  _factor[3].yaw = 1;   _factor[3].throttle = 1;
}


void Servo::SetDutyRange(float maxDuty,float minDuty)
{
	for(int i=0;i<4;i++)
		_motor[i]->SetDutyRange(maxDuty,minDuty);
}
void Servo::FullSpeed()
{
		for(int i=0;i<4;i++)
		_motor[i]->Speed(100);
}
void Servo::Stop()
{
			for(int i=0;i<4;i++)
		_motor[i]->Speed(0);
}

void Servo::Update(const CraftVector &ctrlVal)
{
	for(u8 i=0;i<4;i++)
	{
		_speed[i] = _factor[i].roll*ctrlVal.roll + _factor[i].pitch*ctrlVal.pitch + _factor[i].yaw*ctrlVal.yaw + _factor[i].throttle*ctrlVal.throttle;
		if(_speed[i]<0)
			_speed[i] = 0;
		if(_speed[i]>20)
			_speed[i] = 20;
		_motor[i]->Speed(_speed[i]+15);
	}
}




