#ifndef _GIMBAL_H_
#define _GIMBAL_H_

#include "stm32f10x.h"
#include "StepMotor.h"
#include "AHRS.h"
#include "PIDParameter.h"
#include "ADC.h"
#include "Remoter_PWM_EXIT.h"

class Gimbal
{
	private:
		AHRS &_ahrs;             //AHRS
		StepMotor &_motor_roll;  //Roll Motor
		StepMotor &_motor_pitch; //Pitch Motor
	  StepMotor &_motor_yaw;   //Pitch Motor
	  ADC *_adc_yaw;           //adc object for yaw	
	  u16  _adc_yaw_ch;        //adc channel for yaw
		Remoter_PWM_EXIT* _remoter; //remoter
	
		float _DTms;             //delta time since last control / update  (unit: ms)
		u16 _ctrlHz;             //control frequence
		float _roll;             //roll angle of imu
    float _pitch;            //pitch angle of imu
    float _yaw;              //yaw angle of imu
	
		float _yaw_vel;          //yaw velocity
	  float _yaw_pulse;
	
    float _targetRoll;       //target angle of roll
    float _targetPitch;      //target angle of pitch
    float _targetYaw;	       //target angle of yaw
	  PIDParameter _rollPID;
	  PIDParameter _pitchPID;
	  PIDParameter _yawPID;
	public:
		Gimbal(AHRS &ahrs, StepMotor &motorRoll, StepMotor &motorPitch,StepMotor &motorYaw, u16 ctrlHz=500);
	  void SetYaw(ADC *adc,u16 yawCh);
	  void SetRemoter(Remoter_PWM_EXIT *rc);
	  double UpdateIMU();                               //update _roll,_pitch,_yaw
	  double UpdateRC();                                //update _targetRoll, _targetPitch, targetYaw
	  double UpdateMotor();                             //update PID control value, and set to motors                                
	  float Roll();                                   //get roll angle
	  float Pitch();                                  //get pitch angle
	  float Yaw();                                    //get yaw angle
	  float TargetRoll();                             //get target roll angle
	  float TargetPitch();                            //get target pitch angle
	  float TargetYaw();                              //get target yaw angle
		void SetRollPID(float Kp,float Ki, float Kd);   //set pid parameters of roll
		void SetPitchPID(float Kp,float Ki, float Kd);  //set pid parameters of pitch
		void SetYawPID(float Kp,float Ki, float Kd);    //set pid parameters of yaw
		void Arm();
	  void DisArm();
};



#endif

