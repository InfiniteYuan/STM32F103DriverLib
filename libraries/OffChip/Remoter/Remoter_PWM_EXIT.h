#ifndef _REMOTER_PWM_EXIT_H_
#define _REMOTER_PWM_EXIT_H_

#include "stm32f10x.h"
#include "Remoter.h"
#include "InputCapture_EXIT.h"


class Remoter_PWM_EXIT: public Remoter
{
private:
	InputCapture_EXIT *mICP[10];
public:
	Remoter_PWM_EXIT(InputCapture_EXIT *icpRoll,          //roll ICP
                   InputCapture_EXIT *icpPitch,         //pitch ICP 
                   InputCapture_EXIT *icpYaw = 0,       //yaw ICP 
                   InputCapture_EXIT *icpThrottle = 0,  //throttle 
                   InputCapture_EXIT *icpMode = 0,      //Mode ICP 
                   InputCapture_EXIT *icpAux1 = 0,      //Auxiliary 1 ICP 
                   InputCapture_EXIT *icpAux2 = 0,      //Auxiliary 2 ICP 
                   InputCapture_EXIT *icpAux3 = 0,      //Auxiliary 3 ICP 
                   InputCapture_EXIT *icpAux4 = 0,      //Auxiliary 4 ICP 
                   InputCapture_EXIT *icpAux5 = 0       //Auxiliary 5 ICP 
              );
	virtual float operator[](u8 chNum); //return the percentage of channel x  (0.0 ~ 100.0)
  virtual float Channel(u8 chNum);    //return the percentage of channel x  (0.0 ~ 100.0), the same as operator[]
	virtual float ChannelRaw(u8 chNum); //return the raw value of channel x (ms)
	virtual u8 Update();                //update rc data, return rc status, check lock / unlock
};

#endif


