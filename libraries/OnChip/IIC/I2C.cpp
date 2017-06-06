#include "I2C.h"
#include "TaskManager.h"
#include "Configuration.h"
#include "Interrupt.h"

#include "USART.h"
extern USART com;

I2C::I2C(I2C_TypeDef* i2c, u32 speed,u8 remap,u8 priGroup,u8 preEvt,u8 subEvt,u8 preErr,u8 subErr)
{
  //record gpio, irq, rcc, etc.....
	mI2C    = i2c;
	mSpeed  = speed;          //i2c bus speed    
  mRemap  = remap;          //i2c gpio remap flag	
	mEvtPre = preEvt;         //event preemption priority
	mEvtSub = subEvt;         //event sub priority
	mErrPre = preErr;         //error preemption priority
	mErrSub = subErr;         //error sub priority
	mState  = I2C_STATE_NULL; //I2C not initialized yet
	mErrorCnt = -1;           //no i2c error
	if(mI2C==I2C1)                    //I2C1
	{
		if(mRemap)                      //Remap IO
		{	
			mSCLPin = GPIO_Pin_8;		      //SCL Pin			=======================================================================
			mSDAPin = GPIO_Pin_9;		      //SDA Pin			|  I2C | remap=false |  remap=true |       IRQ   |      RCC           |
		}                               //						|------|-------------|-------------|-------------|--------------------|
		else                            //Not Remap		|      |SCL | PB6    | SCL | PB8   |I2C1_EV_IRQn |                    |
		{		                            //		        | I2C1 |-------------|-------------|             |RCC_APB1Periph_I2C1 |
			mSCLPin = GPIO_Pin_6;		      //SCL Pin			|      |SDA | PB7    | SDA | PB9   |I2C1_ER_IRQn |                    |
			mSDAPin = GPIO_Pin_7;		      //SDA Pin 		|------|-------------|-------------|-------------|--------------------|
		}                               //						|      |SCL | PB10   |     xxx     |I2C2_EV_IRQn |                    |
		mEvtIRQ = I2C1_EV_IRQn;         //Event IRQ 	| I2C2 |-------------|-------------|             |RCC_APB1Periph_I2C1 |
		mErrIRQ = I2C1_ER_IRQn;         //Error IRQ 	|      |SDA | PB11   |     xxx     |I2C2_ER_IRQn |                    |
		mI2CRcc = RCC_APB1Periph_I2C1;  //I2C Clock  	=======================================================================
	#ifdef USE_I2C1
		pI2C1 = this;
	#endif
	}
	else if(mI2C==I2C2)               //I2C2
	{
		mSCLPin = GPIO_Pin_10;	        //SCL Pin	
	  mSDAPin = GPIO_Pin_11;          //SDA Pin 	
		mEvtIRQ = I2C2_EV_IRQn;         //Event IRQ
		mErrIRQ = I2C2_ER_IRQn;         //Error IRQ
		mI2CRcc = RCC_APB1Periph_I2C2;  //I2C Clock
	#ifdef USE_I2C2
		pI2C2 = this;
	#endif
	}	
		//set prioirty group                              // =============================================================
	                                                    // | NVIC_PriorityGroup   | PreemptionPriority |   SubPriority  | 
	switch(priGroup)                                    // |------------------------------------------------------------|
	{                                                   // | NVIC_PriorityGroup_0 | 0 bit   |   0      |  4 bit | 0~15  |
		case 0: mPriGroup = NVIC_PriorityGroup_0; break;  // |----------------------|---------|----------|----------------|
		case 1: mPriGroup = NVIC_PriorityGroup_1; break;  // | NVIC_PriorityGroup_1 | 1 bit   |   0~1    |  3 bit | 0~7   |
		case 2: mPriGroup = NVIC_PriorityGroup_2; break;  // |----------------------|---------|----------|--------|-------|
		case 3: mPriGroup = NVIC_PriorityGroup_3; break;  // | NVIC_PriorityGroup_2 | 2 bit   |   0~3    |  2 bit | 0~3   |
		case 4: mPriGroup = NVIC_PriorityGroup_4; break;  // |------------------------------------------------------------|
	  default:mPriGroup = NVIC_PriorityGroup_3; break;  // | NVIC_PriorityGroup_3 | 3 bit   |   0~7    |  1 bit | 0~1   |
	}                                                   // |----------------------|---------|----------|----------------|    
	                                                    // | NVIC_PriorityGroup_4 | 4 bit   |   0~15   |  0 bit | 0     |                                                              
	//Initialize I2C, NVIC, etc.....                    // ==============================================================
	Initialize();
}
bool I2C::Reset()
{
	mI2C->CR1 |= I2C_CR1_SWRST;   //Start reset I2C 
	mI2C->CR1 &= ~I2C_CR1_SWRST;  //Stop reset I2C
	mI2C->CR1 &= ~I2C_CR1_PE; 		//disable I2C		
	RCC->APB1RSTR |= mI2CRcc;     //start reset i2c clock
	RCC->APB1RSTR &= ~mI2CRcc;    //end reset i2c clock
	RCC->APB1ENR |= mI2CRcc;      //enable i2c clock
	
	//Check and Fix I2C Bus Busy problem
	u8 timeoutCnt = 0;
	while(I2C_GetFlagStatus(mI2C, I2C_FLAG_BUSY) && timeoutCnt<20)
	{
		RCC_APB1PeriphClockCmd(mI2CRcc,DISABLE);  //Close I2C Device
		InitGPIO(GPIO_Mode_Out_PP);               //Initialize GPIO for Software I2C
		GPIO_ResetBits(GPIOB, mSCLPin);           //
		GPIO_ResetBits(GPIOB, mSDAPin);           //To Generate Stop Signal by Pin simulation
		for(volatile u16 i=0;i<100;i++);          //         _______
		GPIO_SetBits(GPIOB, mSCLPin);             //SCL: ___|
		for(volatile u16 i=0;i<100;i++);          //             ___
		GPIO_SetBits(GPIOB, mSDAPin);             //SDA: _______|
		for(volatile u16 i=0;i<100;i++);	        //
		InitGPIO(GPIO_Mode_IN_FLOATING);          //Initialize GPIO for I2C Busy detect
		RCC->APB1RSTR |= mI2CRcc;                 //start reset i2c clock
		RCC->APB1RSTR &= ~mI2CRcc;                //end reset i2c clock
		RCC->APB1ENR |= mI2CRcc;                  //enable i2c clock
		++timeoutCnt;
	}
	if(timeoutCnt>=20) return false;
	return true;
}
void I2C::InitGPIO(GPIOMode_TypeDef gpioMode)
{
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);     //GPIO Clock
	if(mI2C==I2C1 && mRemap && gpioMode==GPIO_Mode_AF_OD)
	{
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_AFIO),ENABLE);  //Enable GPIO Alternative Functions
		GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);          //Enable I2C1 pin Remap
	}
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         //Set GPIO frequency to 50MHz
	GPIO_InitStructure.GPIO_Mode = gpioMode;                  //I2C: GPIO_Mode_AF_OD; Analog: GPIO_Mode_IN_FLOATING; Detect: GPIO_Mode_Out_PP
	GPIO_InitStructure.GPIO_Pin = mSCLPin | mSDAPin;          //Initialize I2Cx SCL Pin
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void I2C::InitI2C()
{
	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_ClockSpeed          = mSpeed;                        //I2C_ClockSpeed
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;                  //I2C_Mode
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;               //I2C_DutyCycle
	I2C_InitStructure.I2C_OwnAddress1         = 0;                             //I2C_OwnAddress1
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;                //I2C_Ack
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;  //I2C_AcknowledgedAddress
	I2C_Init(mI2C,&I2C_InitStructure);//iic
}
void I2C::InitNVIC()
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //Enable the IRQ channel
	NVIC_InitStructure.NVIC_IRQChannel = mEvtIRQ;                   //I2Cx EVT Interrupt 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = mEvtPre; //Event Preemption Priority
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = mEvtSub;        //Event Sub Priority
	NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = mErrIRQ;                   //I2Cx ERR Interrupt 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = mErrPre; //Error Preemption Priority
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = mErrSub;        //Error Sub Priority 
  NVIC_Init(&NVIC_InitStructure);
	mI2C->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_IT_BUF);  //Enable Event, Error, Buffer Interrupt
	NVIC_PriorityGroupConfig(mPriGroup);                            //Set Priority Group
}
bool I2C::Initialize()
{
	mErrorCnt++;
	if(!Reset()) 
	{
		mState = I2C_STATE_ERROR;
		return false; //Reset RCC,I2C, release I2C Bus
	}
	InitGPIO(GPIO_Mode_AF_OD); //GPIO
	InitI2C();                 //I2C
	InitNVIC();                //NVIC
	mCmdBank.Clear();          //Clear Command
	mState = I2C_STATE_FREE;   //I2C bus ready to use
	return true;
}
bool I2C::AddCommand(u8 slaveAddr,u8 txData[], u8 txNum, u8 rxData[], u8 rxNum,Sensor *pDevice, bool isTaskTail)
{
	if(mCmdBank.Size()>=I2C_QUEUE_SIZE) 
		return false;
	I2C_Command i2cCmd;
	i2cCmd.slaveAddr  = slaveAddr;
	i2cCmd.outDataLen = txNum;
	i2cCmd.inDataLen  = rxNum;
	i2cCmd.pDataIn    = rxData;
	i2cCmd.pDevice    = pDevice;
	i2cCmd.isTaskTail = isTaskTail;
	for(u8 i=0;i<txNum;i++)
		i2cCmd.dataOut[i] = txData[i];
	mCmdBank.Put(i2cCmd);
	return true;
}

bool I2C::StartNextCmd()
{
	if(mState==I2C_STATE_FREE || mState==I2C_STATE_STOP)
	{
		if(mCmdBank.Get(mCurCmd))
		{
			mState = I2C_STATE_START;
			mDataIdx = 0;
			if(mCurCmd.outDataLen>0) mI2CDirection = I2C_Direction_Transmitter;
			else                     mI2CDirection = I2C_Direction_Receiver;
			I2C_AcknowledgeConfig(mI2C,ENABLE);
			I2C_GenerateSTART(mI2C,ENABLE);
			return true;
		}
		mState = I2C_STATE_FREE;
	}
	return false;
}
bool I2C::Start()
{
	if(mState==I2C_STATE_FREE)
		return StartNextCmd();
	return false;
}
bool I2C::IsHealthy()
{
	if(mState==I2C_STATE_ERROR) 
		return false;
	return true;
}
bool I2C::IsFree()
{
	if(mState==I2C_STATE_FREE) 
		return true;
	return false;
}
bool I2C::WaitFree(u16 ms)
{
//	static volatile double newTime = 0, oldTime = 0;
//	oldTime = newTime = TaskManager::Time();
	volatile u32 cnt = 0;
	while(mState!=I2C_STATE_FREE && ++cnt<10000/*(newTime-oldTime)*1000<ms*/)	
		/*newTime=TaskManager::Time()*/;
	
	//com<<newTime<<oldTime<<"\n";
	
	if(mState==I2C_STATE_FREE) return true;
	return false;		
}
void I2C::EventIRQ()
{
	static u8 resetCount = 0;
	
	if(++resetCount>100)
	{
		mState = I2C_STATE_ERROR;
		mI2C->CR1 |= I2C_CR1_SWRST; 
		mI2C->CR1 &= ~I2C_CR1_SWRST;
		resetCount = 0;
	}
	
	switch(I2C_GetLastEvent(mI2C))
	{
		case I2C_EVENT_MASTER_MODE_SELECT:               //EV5:SB + BUSY + MSL
			mState = I2C_STATE_ADDR;
			mDataIdx = 0;
			I2C_Send7bitAddress(mI2C, mCurCmd.slaveAddr, mI2CDirection);
			break;
		case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED: //EV6: after I2C_Send7bitAddress(W), should I2C_SendData()
			mState = I2C_STATE_TXMOD;
			I2C_SendData(mI2C, mCurCmd.dataOut[mDataIdx++]);
			break;
		case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:    //EV6:	after I2C_Send7bitAddress(R)	
			mState = I2C_STATE_RXMOD;
			if(mCurCmd.inDataLen<=1)
			{
				I2C_AcknowledgeConfig(mI2C,DISABLE);
				I2C_GenerateSTOP(mI2C,ENABLE);
			}
			break;
		case I2C_EVENT_MASTER_BYTE_RECEIVED:             //EV7:  data arrived, should receive data
			mState = I2C_STATE_RX;
			mCurCmd.pDataIn[mDataIdx++] = I2C_ReceiveData(mI2C);
			if(mCurCmd.inDataLen - mDataIdx == 1)
			{
				I2C_AcknowledgeConfig(mI2C,DISABLE);
				I2C_GenerateSTOP(mI2C,ENABLE);
			}
			else if(mCurCmd.inDataLen == mDataIdx)
			{
				mState = I2C_STATE_STOP;
				resetCount = 0;
				if(mCurCmd.isTaskTail) mCurCmd.pDevice->Updated();
				StartNextCmd();
			}
			break;
		case I2C_EVENT_MASTER_BYTE_TRANSMITTED:	         //EV8_2:after I2C_SendData()	
			if(mDataIdx < mCurCmd.outDataLen)
			{
				mState = I2C_STATE_TX;
				I2C_SendData(mI2C,mCurCmd.dataOut[mDataIdx++]);
			}
			else
			{
				if(mCurCmd.inDataLen>0)
				{
					mState = I2C_STATE_START;
					mDataIdx = 0;
					mI2CDirection = I2C_Direction_Receiver;
					I2C_GenerateSTART(mI2C,ENABLE);
				}
				else
				{
					mState = I2C_STATE_STOP;
					I2C_GenerateSTOP(mI2C,ENABLE);
					resetCount = 0;
					if(mCurCmd.isTaskTail) mCurCmd.pDevice->Updated();
					StartNextCmd();
				}
			}
			break;
	}
}
void I2C::ErrorIRQ()
{
	mI2C->CR1 |= I2C_CR1_SWRST; 
	mI2C->CR1 &= ~I2C_CR1_SWRST;
	mState = I2C_STATE_ERROR;
}




