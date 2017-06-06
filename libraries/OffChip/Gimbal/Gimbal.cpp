#include "Gimbal.h"
#include "TaskManager.h"
#include "math.h"
#include "USART.h"
extern USART com;

Gimbal::Gimbal(AHRS &ahrs, StepMotor &motorRoll,StepMotor &motorPitch,StepMotor &motorYaw, u16 ctrlHz):_ahrs(ahrs), _motor_roll(motorRoll),_motor_pitch(motorPitch),_motor_yaw(motorYaw)
{
	_ctrlHz = ctrlHz;
	_DTms = 1000.0/_ctrlHz;
	
	_roll = 0;
	_pitch = 0;
	_yaw = 0;
	_yaw_vel = 0;
	_yaw_pulse = 0;
	
	_targetRoll = 0;
	_targetPitch = 0;
	_targetYaw = 0;
	

	_adc_yaw = 0;
	_remoter = 0;
	
//	_rollPID(1000,4,13);
//	_pitchPID(1000,4,8);
//	_yawPID(-0,-0.001,-0.00);
}

double Gimbal::UpdateIMU()
{
	double enterTime = tskmgr.Time();
	
	_ahrs.Update();
		
	_roll = _ahrs.Roll();
	_pitch = _ahrs.Pitch();
	
	if(_adc_yaw)
	{
		float newYaw = ((*_adc_yaw)[_adc_yaw_ch]-1.35f)*90.0f / 0.70f;
		_yaw = _yaw + (newYaw-_yaw)*0.1;
	}
	Vector3f gyro = _ahrs.GetGyro();
	
	_yaw_vel = cos(_roll)*cos(_pitch)*gyro.z - gyro.y*sin(_roll) - gyro.x*cos(_roll)*sin(_pitch);//+= 0.1*(_ahrs.GetGyro().z - _yaw_vel);
	
	return (tskmgr.Time()-enterTime)*1000;
}
double Gimbal::UpdateRC()
{
	double enterTime = tskmgr.Time();
	
	if(_remoter)
	{	
		_remoter->Update();
		
		if(!_remoter->Ready()) return (tskmgr.Time()-enterTime)*1000;
		
		float pitchSpeed = (*_remoter)[1]-50;
		float yawSpeed   = (*_remoter)[2]-50;
		float rollSpeed  = (*_remoter)[3]-50;
		
		pitchSpeed = (abs(pitchSpeed)>8 ? pitchSpeed:0);
		yawSpeed   = (abs(yawSpeed)  >8 ? yawSpeed  :0);
		rollSpeed  = (abs(rollSpeed) >8 ? rollSpeed :0);
		
		_targetPitch += 0.006f*pitchSpeed*_DTms / 1000.0f; //pitch
		
		_targetRoll  += 0.003f*rollSpeed*_DTms / 1000.0f; //roll
		
		float deltaYaw = 0.8f*yawSpeed*_DTms / 1000.0f; //yaw
		_targetYaw   += deltaYaw;
		_yaw_pulse += 4.2f*deltaYaw;
	}
	
	return (tskmgr.Time()-enterTime)*1000;
}

double Gimbal::UpdateMotor()
{
	double enterTime = tskmgr.Time();
	
	_motor_roll.SetPosition(_rollPID.ComputePID(_targetRoll,_roll));
	_motor_pitch.SetPosition(_pitchPID.ComputePID(_targetPitch,_pitch));
	if(_adc_yaw)
	{
		
		_motor_yaw.SetPosition(-_yawPID.ComputePID(_targetYaw,_yaw)-_yaw_vel*150 + _yaw_pulse);
	}
	return (tskmgr.Time()-enterTime)*1000;
}

void Gimbal::SetYaw(ADC *adc,u16 yawCh)
{
	_adc_yaw = adc;
	_adc_yaw_ch = yawCh;
}
void Gimbal::SetRemoter(Remoter_PWM_EXIT *rc)
{
	_remoter = rc;
}

float Gimbal::Roll()                                   //get roll angle
{
	return _roll*57.29f;
}
float Gimbal::Pitch()                                  //get pitch angle
{
	return _pitch*57.29f;
}

float Gimbal::Yaw()                                    //get yaw angle
{
	return _yaw;
}

float Gimbal::TargetRoll()                             //get target roll angle
{
	return _targetRoll;
}
float Gimbal::TargetPitch()                            //get target pitch angle
{
	return _targetPitch;
}
float Gimbal::TargetYaw()                              //get target yaw angle
{
	return _targetYaw;
}

void Gimbal::SetRollPID(float Kp,float Ki, float Kd)   //set pid parameters of roll
{
//	_rollPID(Kp,Ki,Kd);
}
void Gimbal::SetPitchPID(float Kp,float Ki, float Kd)  //set pid parameters of pitch
{
//	_pitchPID(Kp,Ki,Kd);
}
void Gimbal::SetYawPID(float Kp,float Ki, float Kd)    //set pid parameters of yaw
{
//	_yawPID(Kp,Ki,Kd);
}

void Gimbal::Arm()
{
	_motor_roll.Enable();
	_motor_pitch.Enable();
	_motor_yaw.Enable();
}
void Gimbal::DisArm()
{
		_motor_roll.Disable();
	_motor_pitch.Disable();
	_motor_yaw.Disable();
}
