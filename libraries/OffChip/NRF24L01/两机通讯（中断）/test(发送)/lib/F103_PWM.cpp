#include "F103_PWM.h"


/**
 *@brief 用于计算出Psc的值和Arr的值，分两种情况，第一种，72M能被frequency整除，第二种 72M不能被frequency整除
 *@param quo  第一次计算之后的商
 *@param quo2 第二次计算的商，由于在第一次计算里面已经定义了，就直接传过来了；
 *@param rem2 理由通quo2
 *@param flagBest 标志是否找到最佳的Psc和Arr的值
 */

void PWM::Calcute2(u32 quo, u32 quo2, u32 rem2, bool flagBest)
{
	u16 i;
	for (i = 1; i <= 1200; i++)
	{
		if (quo%i == 0 && quo / i < 65535){ mPsc = i; mArr = quo / i; flagBest = true; break; }

	}
	if (flagBest == false)//没有找到最佳值
	{
		for (i = 1; i <= 1200; i++)
		{
			quo2 = quo / i;
			rem2 = quo % i;
			if (quo2 < 65535 && rem2 < 32758){ mPsc = i; mArr = quo2; flagBest = true; break; }
		}
	}
}

/**
 *@brief 用于第一次计算，判断是否期望的频率能整除72M
 *@param frequency 期望的频率
 */
void PWM::Calcute(u16 frequency)
{

	bool flagBest = false;		   //标志是否找到最佳值
	u32  quo = 72000000 / frequency;//获得商数
	u32 rem = 72000000 % frequency;//获得余数 

	u32  quo2 = 0;
	u32 rem2 = 0;
	if (rem == 0)//1·F能整除72M;将除数quo再重1开始除，在能整除的情况下，取
	{
		Calcute2(quo, quo2, rem2, flagBest);
	}
	else
	{
		if (rem > 32768)quo++;
		Calcute2(quo, quo2, rem2, flagBest);
	}
}

/**
 *@brief 构造函数
 *@param timer 选择的时钟
 *@param enCh1 通道1是否打开
 *@param enCh2 通道2是否打开
 *@param enCh3 通道3是否打开
 *@param enCh4 通道4是否打开
 *@param frequency 期望频率
 *@param remap 引脚是否映射
 */
void PWM::PWMInit(TIM_TypeDef* timer, bool enCh1 = 0, bool enCh2 = 0, bool enCh3 = 0, bool enCh4 = 0, u16 frequency = 0, u8 remap)//传入选择的定时器,若没有传入参数，默认TIM2
{
	/*
		根据频率设置PSC和ARR的值，以后再说，现在能够满足要求；

		//		mPsc = 71;
		//		mArr = (u16)((72000000)/((mPsc+1)*frequency))-1;
		*/


	mTimer = timer;																			//get the TIM of chosen
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//open the PinA Timer
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//open the PinB Timer
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//管脚复用打开

	//因为swtich不支持TIM_TypeDef类型,强制转换之后会变成null，所以采用传统的if else来设置GPIO

	if (timer == TIM2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_0 & enCh1) |
			(GPIO_Pin_1 & enCh2 << 1) |
			(GPIO_Pin_2 & enCh3 << 2) |
			(GPIO_Pin_3 & enCh4 << 3);
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStruct);

	}
	else
	{
		if (timer == TIM3)
		{
			if (remap == 0){
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
				GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_6 & enCh1 << 6) |
					(GPIO_Pin_7 & enCh2 << 7);
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_Init(GPIOA, &GPIO_InitStruct);

				GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_0 & enCh3) |
					(GPIO_Pin_1 & enCh4 << 1);
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
			}
			else if (remap == 1){
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
				GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
				GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_4 & enCh1 << 4) |
					(GPIO_Pin_5 & enCh2 << 5) |
					(GPIO_Pin_0 & enCh3) |
					(GPIO_Pin_1 & enCh4 << 1);
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
			}
		}
		else{
			if (timer == TIM4)
			{
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
				GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_6 & enCh1 << 6) |
					(GPIO_Pin_7 & enCh2 << 7) |
					(GPIO_Pin_8 & enCh3 << 8) |
					(GPIO_Pin_9 & enCh4 << 9);
				GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
				GPIO_Init(GPIOB, &GPIO_InitStruct);
			}
		}
	}


	//开始设置TIM
	TIM_TimeBaseStruct.TIM_Period = mArr - 1;//设置ARR寄存器，就是计数多少次；PSC = 7199的话就以10KHZ计数5000次，就是5000×0.1ms = 500ms 2HZ
	TIM_TimeBaseStruct.TIM_Prescaler = mPsc - 1;//预分频寄存器输入7200，计数时钟为10kHZ。72M/7200 = 10K
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;//0;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;//中间对齐,形成中间对齐的PWM波 TIM_CounterMode_Up;向上//TIM_CounterMode_CenterAligned1//中间对齐
	TIM_TimeBaseInit(timer, &TIM_TimeBaseStruct);//初始化TIM


	//pwm设置，即初始化
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//选择定时器模式为pwm1模式
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//设置的输出极性，表现为占空比数值是高电平直接占还是低电平
	if (enCh1 == 1)
	{
		//通道1---
		TIM_OCInitStructure.TIM_Pulse = mArr / 2;//占空比时间
		TIM_OC1Init(timer, &TIM_OCInitStructure);//根据TIM_OCInitStructure结构体初始化TIM通道1
		TIM_OC1PreloadConfig(timer, TIM_OCPreload_Enable);//使能预装载器
	}
	if (enCh2 == 1)
	{
		//通道2---
		TIM_OCInitStructure.TIM_Pulse = mArr / 2;//占空比时间
		TIM_OC2Init(timer, &TIM_OCInitStructure);//根据TIM_OCInitStructure结构体初始化TIM通道2
		TIM_OC2PreloadConfig(timer, TIM_OCPreload_Enable);//使能预装载器
	}
	if (enCh3 == 1)
	{
		//通道3---
		TIM_OCInitStructure.TIM_Pulse = mArr / 2;//占空比时间
		TIM_OC3Init(timer, &TIM_OCInitStructure);//根据TIM_OCInitStructure结构体初始化TIM通道3
		TIM_OC3PreloadConfig(timer, TIM_OCPreload_Enable);//使能预装载器
	}
	if (enCh4 == 1)
	{
		//通道4---
		TIM_OCInitStructure.TIM_Pulse = mArr / 2;//占空比时间
		TIM_OC4Init(timer, &TIM_OCInitStructure);//根据TIM_OCInitStructure结构体初始化TIM通道4
		TIM_OC4PreloadConfig(timer, TIM_OCPreload_Enable);//使能预装载器
	}
	TIM_Cmd(timer, ENABLE);

}
/**
 *@brief 改变一个通道的占空比
 *@param channel 选择通道
 *@param duty    设置占空比
 *@param isSetPositiveDuty 设置高电平的占空比还是设置低电平的占空比
 */

void PWM::SetDuty(u8 channel, float duty, bool isSetPositiveDuty)
{
	if (isSetPositiveDuty)
		duty = 100 - duty;
	u16 arrTemp = (duty / 100)*mArr;
	switch (channel)
	{
	case 1:mTimer->CCR1 = arrTemp; break;

	case 2:mTimer->CCR2 = arrTemp; break;

	case 3:mTimer->CCR3 = arrTemp; break;

	case 4:mTimer->CCR4 = arrTemp; break;

	}
}
/**
 *@brief 改变一个通道的占空比
 *@param dutyCh1 设置通道1的占空比
 *@param dutyCh2 设置通道2的占空比
 *@param dutyCh3 设置通道3的占空比
 *@param dutyCh4 设置通道4的占空比
 *@param isSetPositiveDuty 设置高电平的占空比还是设置低电平的占空比
 */
void PWM::SetDuty(float dutyCh1, float dutyCh2, float dutyCh3, float dutyCh4, bool isSetPositiveDuty)
{
	if (isSetPositiveDuty)
	{
		dutyCh1 = 100 - dutyCh1;
		dutyCh2 = 100 - dutyCh2;
		dutyCh3 = 100 - dutyCh3;
		dutyCh4 = 100 - dutyCh4;
	}
	u16 ccr1Temp = (dutyCh1 / 100)*mArr;
	u16 ccr2Temp = (dutyCh2 / 100)*mArr;
	u16 ccr3Temp = (dutyCh3 / 100)*mArr;
	u16 ccr4Temp = (dutyCh4 / 100)*mArr;

	mTimer->CCR1 = ccr1Temp;
	mTimer->CCR2 = ccr2Temp;
	mTimer->CCR3 = ccr3Temp;
	mTimer->CCR4 = ccr4Temp;
}

u16 PWM::GetmFrequency()
{
	return mFrequency;
}

