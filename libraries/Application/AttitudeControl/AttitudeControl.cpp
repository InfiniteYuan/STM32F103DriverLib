#include "AttitudeControl.h"



AttitudeControl::AttitudeControl(Remoter &rc,AHRS &ahrs, Servo &servo)
  :_rc(rc),_ahrs(ahrs),_servo(servo), _posLoop(_curAngle, _tagAngle, _tagAngVel) ,_velLoop(_curAngVel,_tagAngVel,_velCtrlVal)
{
	
}


void AttitudeControl::UpdateSensor()
{
	_curAngle.roll  = _ahrs.Roll()*57.29577f;
	_curAngle.pitch = _ahrs.Pitch()*57.29577f;
	_curAngle.yaw   = _ahrs.Yaw()*57.29577f;
	
	_curAngVel.roll  = _ahrs.GetGyro().x*57.29577f;
	_curAngVel.pitch = _ahrs.GetGyro().y*57.29577f;
	_curAngVel.yaw   = _ahrs.GetGyro().z*57.29577f;
	 
//	_tagAngle.roll  =  _rc.Roll()-50;
	_tagAngle.pitch =  (_rc.Pitch()-50.0f)/2.0f;
//	_tagAngle.yaw  += (_rc.Yaw()-50)*0.001;
	
//	_tagAngVel.pitch = _tagAngle.pitch*3.0f;
//	_tagAngVel.roll  = (_rc.Roll()-50.0f)*0.2f;
//	_tagAngVel.pitch = (_rc.Pitch()-50.0f)*2.0f; //+= _tagAngVel.pitch + 0.3f*( (_rc.Pitch()-50.0f) - _tagAngVel.pitch);
//	_tagAngVel.yaw   = (_rc.Yaw()-50.0f)*0.2f;
}

void AttitudeControl::UpdateServo()
{
	_servo.Update(_velCtrlVal);
}

void AttitudeControl::PositionLoop()
{
	_posLoop.Roll();
	_posLoop.Pitch();
	_posLoop.Yaw();
	_posLoop.Throttle();
}

void AttitudeControl::VelocityLoop()
{
	_velLoop.Roll();
	_velLoop.Pitch();
	_velLoop.Yaw();
	_velLoop.Throttle();
}

void AttitudeControl::SET_POS_PID(float kp,float ki,float kd)
{
	_posLoop.Set_PIT_PID((float)kp,(float)ki,(float)kd);
}

void AttitudeControl::SET_RATE_PID(float kp,float ki,float kd)
{
	_velLoop.Set_PIT_PID((float)kp,(float)ki,(float)kd);
}

float AttitudeControl::CurrentVelocityRoll()
{	return _curAngVel.roll; }
float AttitudeControl::CurrentVelocityPitch()
{	return _curAngVel.pitch; }
float AttitudeControl::CurrentVelocityYaw()
{	return _curAngVel.yaw; }
float AttitudeControl::CurrentVelocityThrottle()
{	return _curAngVel.throttle; }
float AttitudeControl::TargetVelocityRoll()
{ return _tagAngVel.roll;}
float AttitudeControl::TargetVelocityPitch()
{ return _tagAngVel.pitch;}	
float AttitudeControl::TargetVelocityYaw()
{ return _tagAngVel.yaw;}
float AttitudeControl::TargetVelocityThrottle()
{ return _tagAngVel.throttle;}

float AttitudeControl::VelCtrlValueRoll()
{ return _velCtrlVal.roll; }
float AttitudeControl::VelCtrlValuePitch()
{ return _velCtrlVal.pitch; }
float AttitudeControl::VelCtrlValueYaw()
{ return _velCtrlVal.yaw; }
float AttitudeControl::VelCtrlValueThrottle()
{ return _velCtrlVal.throttle; }

float AttitudeControl::CurrentAngleRoll()
{ return _curAngle.roll; }
float AttitudeControl::CurrentAnglePitch()
{ return _curAngle.pitch; }
float AttitudeControl::CurrentAngleYaw()
{ return _curAngle.yaw; }
float AttitudeControl::TargetAngleRoll()
{ return _tagAngle.roll; }
float AttitudeControl::TargetAnglePitch()
{ return _tagAngle.pitch; }
float AttitudeControl::TargetAngleYaw()
{ return _tagAngle.yaw; }


