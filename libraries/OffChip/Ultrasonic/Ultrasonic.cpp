#include "Ultrasonic.h"
#include "Interrupt.h"
#include "TaskManager.h"

//Constructor: RCC, GPIO, NVIC, EXIT
Ultrasonic::Ultrasonic(GPIO_TypeDef *trigPort, u8 trigPin, GPIO_TypeDef *echoPort, u8 echoPin, float maxRange, u8 itGroup,u8 prePriority,u8 subPriority)
{
	mTrigPort = trigPort;       //trig gpio port
	mEchoPort = echoPort;       //echo gpio port
	mTrigPin = u16(1<<trigPin); //trig gpio pin
	mEchoPin = u16(1<<echoPin); //echo gpio pin
	mMaxRange = maxRange;
	mIsReady = true;
	
	//Stpe 1: GPIO Initialize: RCC & GPIO Pin
	u8 portSource;
	if(mEchoPort==GPIOA)	     { RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); portSource = 0;}
	else if(mEchoPort==GPIOB)  { RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); portSource = 1;}
	else if(mEchoPort==GPIOC)	 { RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); portSource = 2;}
	else if(mEchoPort==GPIOD)	 { RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE); portSource = 3;}
	else if(mEchoPort==GPIOE)	 { RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE); portSource = 4;}
	else if(mEchoPort==GPIOF)	 { RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE); portSource = 5;}
	
	if(mTrigPort==GPIOA)	     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	else if(mTrigPort==GPIOB)  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	else if(mTrigPort==GPIOC)	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	else if(mTrigPort==GPIOD)	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	else if(mTrigPort==GPIOE)	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	else if(mTrigPort==GPIOF)	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = mTrigPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(mTrigPort,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = mEchoPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(mEchoPort,&GPIO_InitStructure);
	
	GPIO_ResetBits(mTrigPort,mTrigPin);
	GPIO_ResetBits(mEchoPort,mEchoPin);
	
	GPIO_EXTILineConfig(portSource,echoPin);
	
  //Step 2: NVIC Initialize
	u32 priorityGroup[5] = {NVIC_PriorityGroup_0,NVIC_PriorityGroup_1,NVIC_PriorityGroup_2,NVIC_PriorityGroup_3,NVIC_PriorityGroup_4};
	u8 EXTIx_IRQn;			//外部中断号	
	switch (echoPin)
	{
		case 0:
			EXTIx_IRQn = EXTI0_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI0
			pUltExt0 = this;
		#endif
		break;
		case 1:
			EXTIx_IRQn = EXTI1_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI1
			pUltExt1 = this;
		#endif
		break;
		case 2:
			EXTIx_IRQn = EXTI2_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI2
			pUltExt2 = this;
		#endif
		break;
		case 3:
			EXTIx_IRQn = EXTI3_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI3
			pUltExt3 = this;
		#endif
		break;
		case 4:
			EXTIx_IRQn = EXTI4_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI4
			pUltExt4 = this;
		#endif
		break;
		case 5:
		#ifdef USE_ULTRASONIC_EXTI5
			pUltExt5 = this;
		#endif
			EXTIx_IRQn = EXTI9_5_IRQn; 
		break;
		case 6:
		#ifdef USE_ULTRASONIC_EXTI6
			pUltExt6 = this;
		#endif
			EXTIx_IRQn = EXTI9_5_IRQn; 
		break;
		case 7:
		#ifdef USE_ULTRASONIC_EXTI7
			pUltExt7 = this;
		#endif
			EXTIx_IRQn = EXTI9_5_IRQn; 
		break;
		case 8:
			EXTIx_IRQn = EXTI9_5_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI8
			pUltExt8 = this;
		#endif
		case 9:
			EXTIx_IRQn = EXTI9_5_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI9
			pUltExt9 = this;
		#endif
		break;
		case 10:
		#ifdef USE_ULTRASONIC_EXTI10
			pUltExt10 = this;
		#endif	
			EXTIx_IRQn = EXTI15_10_IRQn; 
		break;		
		case 11:
		#ifdef USE_ULTRASONIC_EXTI11
			pUltExt11 = this;
		#endif	
			EXTIx_IRQn = EXTI15_10_IRQn; 
		break;		
		case 12:
		#ifdef USE_ULTRASONIC_EXTI12
			pUltExt12 = this;
		#endif	
			EXTIx_IRQn = EXTI15_10_IRQn; 
		break;		
		case 13:
		#ifdef USE_ULTRASONIC_EXTI13
			pUltExt13 = this;
		#endif	
			EXTIx_IRQn = EXTI15_10_IRQn; 
		break;		
		case 14:
		#ifdef USE_ULTRASONIC_EXTI14
			pUltExt14 = this;
		#endif		
			EXTIx_IRQn = EXTI15_10_IRQn; 
		break;
		case 15:
			EXTIx_IRQn = EXTI15_10_IRQn; 
		#ifdef USE_ULTRASONIC_EXTI15
			pUltExt15 = this;
		#endif
		break;
	}	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(priorityGroup[itGroup]);		               //one project should has only one priority group
	NVIC_InitStructure.NVIC_IRQChannel = EXTIx_IRQn;	                 //IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prePriority;//preemption priority 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority;       //sub priority
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);
	
	//Stpe 3: EXTI Initialize
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_ClearITPendingBit(u32(1<<echoPin));							        //clear exti interrupt flag bit
	EXTI_InitStructure.EXTI_Line = u32(mEchoPin);					 	      //select exti line
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;           //interrupt
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//trigger: both rising and falling edge
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;	
	EXTI_Init(&EXTI_InitStructure);
		
	//变量初始化
	mTrigTime = 0;
	mRiseTime = 0;
	mFallTime = 0;
}


//return last sonar distance and trigger next detection
u8 Ultrasonic::Update(float &dis)
{
	static double newTime = 0, oldTime = 0, interval = 0;
	static u8 sonarState;
	
	newTime = tskmgr.Time();
	interval = newTime-oldTime;
	if((mIsReady==false && interval<MAX_SONAR_INTERVAL) || interval<MIN_SONAR_INTERVAL) //check interval time since last detection
		return MOD_BUSY; //interval time too short, current detection not finish,do nothing
	oldTime = newTime;
	//calculate distance
	if(mIsReady==false || mRiseTime>mFallTime)
	{
		sonarState = MOD_ERROR;
		dis = 5000;
	}
	else
	{
		interval = mFallTime - mTrigTime;
		dis = interval*17000 - 8;
		//if(dis<0 || dis>mMaxRange) dis = 10000; //without max range
		sonarState = MOD_READY;
	}
	//trigge next detection	
	GPIO_ResetBits(mEchoPort,mEchoPin); //echo = 0	
	GPIO_SetBits(mTrigPort,mTrigPin);   //trig = 1
	mTrigTime = tskmgr.Time();          //record trig time
	mIsReady = false;
	for(volatile u16 i=0; i<200;i++);	  //delay at least 10us
	GPIO_ResetBits(mTrigPort,mTrigPin); //trig = 0
	
	return sonarState;
}
//EXTI interrupt response function
void Ultrasonic::IRQ()
{
	if(GPIO_ReadInputDataBit(mEchoPort,mEchoPin))	//rising edge interrupt
		mRiseTime = tskmgr.Time();                  //rising edge time
	else											                    //falling edge interrupt
	{
		mFallTime = tskmgr.Time();                  //falling edge time
		mIsReady = true;
	}
}







