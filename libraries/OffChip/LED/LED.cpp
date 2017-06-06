#include "LED.h"
#include "TaskManager.h"

const  u16  LED::mPin[16]={GPIO_Pin_0,  GPIO_Pin_1,  GPIO_Pin_2,  GPIO_Pin_3,  
	                         GPIO_Pin_4,  GPIO_Pin_5,  GPIO_Pin_6,  GPIO_Pin_7,
								           GPIO_Pin_8,  GPIO_Pin_9,  GPIO_Pin_10, GPIO_Pin_11, 
                           GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15};

LED::LED(GPIO_TypeDef* GPIOx, u16 Pinx, bool lowOn)
{
	mPort = GPIOx;
	mSelectPin = Pinx;
	mLowOn = lowOn;
	mLedOn = false;
	mNewTime = 0;
	mOldTime = 0;
	
	if(mPort==GPIOA)	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	if(mPort==GPIOB)	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	if(mPort==GPIOC)	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	if(mPort==GPIOD)	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	if(mPort==GPIOE)	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	if(mPort==GPIOF)	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin      = mPin[mSelectPin];
	GPIO_InitStructure.GPIO_Speed    = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode     = GPIO_Mode_Out_PP;
	GPIO_Init(mPort,&GPIO_InitStructure);
}


void LED::On()
{
	if(mLowOn)
		GPIO_ResetBits(mPort,mPin[mSelectPin]);
	else
		GPIO_SetBits(mPort,mPin[mSelectPin]);
}

void LED::Off()
{
	if(mLowOn)
		GPIO_SetBits(mPort,mPin[mSelectPin]);
	else
		GPIO_ResetBits(mPort,mPin[mSelectPin]);
}

void LED::Hz(u16 frq)
{
	mNewTime = tskmgr.Time();
	if(mNewTime-mOldTime > 1.0/double(frq))
	{
		mLedOn = !mLedOn;
		mOldTime = mNewTime;
		if(mLedOn)  On();
		else        Off();
	}
}


