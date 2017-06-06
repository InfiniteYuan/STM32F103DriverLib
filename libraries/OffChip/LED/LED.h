#ifndef _LED_H_
#define _LED_H_

#include "stm32f10x.h"

class LED
{
	private:
		static const u16 mPin[16];    //Pin0 ~ Pin15
		GPIO_TypeDef*    mPort;       //GPIOx
		u16			         mSelectPin;  //Pinx
	  bool             mLowOn;      //true = led on when pin level low; false = led on when pin high
	  bool             mLedOn;      //true = led on;  false = led off
	  double           mNewTime;    //new time
	  double           mOldTime;    //old time
	public:
		LED(GPIO_TypeDef* GPIOx, u16 Pinx, bool lowOn=true);
	  void On();
	  void Off();
	  void Hz(u16 frq);
};



#endif


