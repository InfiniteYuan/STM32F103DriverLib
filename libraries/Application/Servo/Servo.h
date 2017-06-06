#ifndef _SERVO_H_
#define _SERVO_H_

#include "stm32f10x.h"
#include "ESCMotor.h"
#include "mathtool.h"


//   3   back  4
//    O       O
//     \     /
//      \   /
//       \ /
//        X
//       / \
//      /   \
//     /     \
//    O       O
//   2  front  1

class Servo
{
private:
	ESCMotor *_motor[4];
	float _speed[4];
	CraftVector _factor[4];
public:
	Servo(ESCMotor *m1,ESCMotor *m2,ESCMotor *m3,ESCMotor *m4);
	void SetDutyRange(float maxDuty,float minDuty);
	void FullSpeed();
	void Stop();

	void Update(const CraftVector &velCtrlValue);
};







#endif

