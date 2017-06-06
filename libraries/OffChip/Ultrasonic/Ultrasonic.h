/*
*********************************************************************************************************
*
*                                      Ultrasonic CALSS HEADER FILE                                
*
* Filename      : Ultrasonic.h
* Version       : V1.1
* Test module	: HC-SR04
* Programmer(s) : Jason Xie
*              
*********************************************************************************************************
*/

#ifndef _ULTRASONIC_H
#define _ULTRASONIC_H

#include "stm32f10x.h"

#define MAX_SONAR_INTERVAL 0.25    //max interval time since last detection, unit: second
#define MIN_SONAR_INTERVAL 0.02   //mix interval time since last detection, unit: second

class Ultrasonic
{
protected:
	GPIO_TypeDef *mTrigPort;  //GPIO Port of Trig pin 
	GPIO_TypeDef *mEchoPort;  //GPIO Port of Echo pin
	u16 mTrigPin;             //GPIO pin for trig
	u16 mEchoPin;             //GPIO pin for echo
	double mTrigTime;          //trig time
	double mRiseTime;          //rising edge time
	double mFallTime;          //falling edge time
	float mMaxRange;          //max detection range (cm)
	bool mIsReady;
public:
		Ultrasonic(GPIO_TypeDef *trigPort, u8 trigPin, GPIO_TypeDef *echoPort, u8 echoPin, float maxRange=200,u8 itGroup=3,u8 prePriority=7,u8 subPriority=1);
		void IRQ();        //Interrupt Response Function
		u8 Update(float &dis);     //Start a ultrasonic detection
};

#endif
