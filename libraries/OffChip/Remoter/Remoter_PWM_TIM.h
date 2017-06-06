#ifndef _REMOTER_PWM_H_
#define _REMOTER_PWM_H_

#include "stm32f10x.h"
#include "Remoter.h"
#include "InputCapture_TIM.h"


class Remoter_PWM_TIM: public Remoter
{
private:
	InputCapture_TIM *mICP[10];
	u8 mChNum[10];
	bool _health;
public:
	Remoter_PWM_TIM(InputCapture_TIM *icpRoll,         u8 chRoll,          //roll ICP and channel number in ICP
              InputCapture_TIM *icpPitch,        u8 chPitch,         //pitch ICP and channel number in ICP
              InputCapture_TIM *icpYaw = 0,      u8 chYaw = 0,       //yaw ICP and channel number in ICP
              InputCapture_TIM *icpThrottle = 0, u8 chThrottle = 0,  //throttle ICP and channel number in ICP
              InputCapture_TIM *icpMode = 0,     u8 chMode = 0,      //Mode ICP and channel number in ICP
              InputCapture_TIM *icpAux1 = 0,     u8 chAux1 = 0,      //Auxiliary 1 ICP and channel number in ICP
              InputCapture_TIM *icpAux2 = 0,     u8 chAux2 = 0,      //Auxiliary 2 ICP and channel number in ICP
              InputCapture_TIM *icpAux3 = 0,     u8 chAux3 = 0,      //Auxiliary 3 ICP and channel number in ICP
              InputCapture_TIM *icpAux4 = 0,     u8 chAux4 = 0,      //Auxiliary 4 ICP and channel number in ICP
              InputCapture_TIM *icpAux5 = 0,     u8 chAux5 = 0       //Auxiliary 5 ICP and channel number in ICP
              );
	virtual float operator[](u8 chNum); //return the percentage of channel x  (0.0 ~ 100.0)
  virtual float Channel(u8 chNum);    //return the percentage of channel x  (0.0 ~ 100.0), the same as operator[]
	virtual float ChannelRaw(u8 chNum); //return the raw value of channel x (ms)
	virtual u8 Update();                //update rc data, return rc status, check lock / unlock
	bool getHealth();
};

#endif


