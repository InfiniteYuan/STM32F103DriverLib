/**
*@file USART.cpp
*@author Neutree
*@version v1.0
*@brief stm32f10x串口驱动文件，使用时引入 USART.h USART.cpp FIFOBuffer.h三个文件，然后根据需要配置USART.h中开头的配置部分（注意中断函数配置）
* 然后定义对象初始化，既可以使用
*@copyright 
*
*/

#include "USART.h"
#include "Interrupt.h"
#include "Configuration.h"

USART::USART(USART_TypeDef* USARTx,u32 baud,u8 priGroup, u8 prePri, u8 subPri,bool remap,u16 parity, u16 wordLen, u16 stopBits)
{
	mUSARTx   = USARTx;   //USARTx
	mBaudrate = baud;     //baudrate of usart
	mPriGroup = priGroup; //priority group
	mPrePri   = prePri;   //preemption priority
	mSubPri   = subPri;   //sub priority
	mRemap    = remap;    //gpio remap flag
	mParity   = parity;   //parity of usart
  mWordLen  = wordLen;  //world length of usart
  mStopBits = stopBits; //stop bits of usart
	mPrecision = 3;
	mTxOverflow = 0;
	mRxOverflow = 0;
	
	if(mUSARTx==USART1)
	{
		mIRQn      = USART1_IRQn;                                           //USART IRQn
		mUSARTRcc  = RCC_APB2Periph_USART1;	                                //USARTx Clock
		mPort    = (mRemap? GPIOB                : GPIOA               );   //GPIO Port
		mTxPin   = (mRemap? GPIO_Pin_6           : GPIO_Pin_9          );   //Tx Pin
		mRxPin   = (mRemap? GPIO_Pin_7           : GPIO_Pin_10         );   //Rx Pin
		mGPIORcc = (mRemap? RCC_APB2Periph_GPIOB : RCC_APB2Periph_GPIOA);   //GPIO Clock
		
	#ifdef USE_USART1_DMA
		mDMATxCh   = DMA1_Channel4;       //DMA Tx Channel
		mDMAIRQn   = DMA1_Channel4_IRQn;  //DMA IRQn
		mDMATCFlag = DMA1_FLAG_TC4;       //DMA TC Mask
		mDMAGLFlag = DMA1_IT_GL4;         //DMA IT GL mask
		pCOM1 = this;
	#endif
		
	#ifdef USE_USART1
		pCOM1 = this;
	#endif
		
	}
	else if(mUSARTx==USART2)
	{
		mIRQn      = USART2_IRQn;                                           //USART IRQn
		mUSARTRcc  = RCC_APB1Periph_USART2;	                                //USARTx Clock
		mPort    = (mRemap? GPIOD                : GPIOA               );   //GPIO Port
		mTxPin   = (mRemap? GPIO_Pin_5           : GPIO_Pin_2          );   //Tx Pin
		mRxPin   = (mRemap? GPIO_Pin_6           : GPIO_Pin_3         );    //Rx Pin
		mGPIORcc = (mRemap? RCC_APB2Periph_GPIOD : RCC_APB2Periph_GPIOA);   //GPIO Clock
	#ifdef USE_USART2_DMA
		mDMATxCh   = DMA1_Channel7;       //DMA Tx Channel
		mDMAIRQn   = DMA1_Channel7_IRQn;  //DMA IRQn
		mDMATCFlag = DMA1_FLAG_TC7;       //DMA TC Mask
		mDMAGLFlag = DMA1_IT_GL7;         //DMA IT GL mask
		pCOM2 = this;
	#endif
	#ifdef USE_USART2
		pCOM2 = this;
	#endif
	}
	else if(mUSARTx==USART3)
	{
		mIRQn      = USART3_IRQn;                                           //USART IRQn
		mUSARTRcc  = RCC_APB1Periph_USART3;	                                //USARTx Clock
		mPort    = (mRemap? GPIOC                : GPIOB               );   //GPIO Port
		mTxPin   = (mRemap? GPIO_Pin_10          : GPIO_Pin_10         );   //Tx Pin
		mRxPin   = (mRemap? GPIO_Pin_11          : GPIO_Pin_11         );   //Rx Pin
		mGPIORcc = (mRemap? RCC_APB2Periph_GPIOC : RCC_APB2Periph_GPIOB);   //GPIO Clock
	#ifdef USE_USART3_DMA
		mDMATxCh   = DMA1_Channel2;       //DMA Tx Channel
		mDMAIRQn   = DMA1_Channel2_IRQn;  //DMA IRQn
		mDMATCFlag = DMA1_FLAG_TC2;       //DMA TC Mask
		mDMAGLFlag = DMA1_IT_GL2;         //DMA IT GL mask
		pCOM3 = this;
	#endif
	#ifdef USE_USART3
		pCOM3 = this;
	#endif
	}
	
	Initialize();
	
}

void USART::Initialize()
{
	InitGPIO();
	InitNVIC();
	InitUSART();
#ifdef USE_USART_DMA
	InitDMA();
#endif
}

void USART::InitGPIO()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | mGPIORcc, ENABLE);
	if(mUSARTx==USART1)  RCC_APB2PeriphClockCmd(mUSARTRcc, ENABLE);
	else    	           RCC_APB1PeriphClockCmd(mUSARTRcc, ENABLE);
	if(mRemap)
	{
		if(mUSARTx==USART1) GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
		if(mUSARTx==USART2) GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
		if(mUSARTx==USART3) GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);
	}
	GPIO_InitTypeDef GPIO_InitStructure;//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;       //
	GPIO_InitStructure.GPIO_Pin   = mTxPin;                //Tx
	GPIO_Init(mPort, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //in floating
	GPIO_InitStructure.GPIO_Pin   = mRxPin;                //Rx
	GPIO_Init(mPort, &GPIO_InitStructure); 
}

void USART::InitNVIC()
{
	switch(mPriGroup)
	{
		case 0:	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);	break;
		case 1:	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	break;
		case 2:	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	break;
		case 3:	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);	break;
		case 4:	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	break;
		default:NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);	break;
	}
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel                   = mIRQn;   //USART IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = mPrePri; //preemption priority
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = mSubPri; //sub priority
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;  //enable interrup
	NVIC_Init(&NVIC_InitStructure);                                 //initialize irq
#ifdef USE_USART_DMA
	NVIC_InitStructure.NVIC_IRQChannel                   = mDMAIRQn;//DMA IRQ
	NVIC_Init(&NVIC_InitStructure);
//	mDMATxCh->CCR |= DMA_IT_TC;  //Enable DMA TX Channel TCIT 
//	mDMATxCh->CCR |= DMA_IT_TE;  //Enable DMA TX Channel TEIT
#endif
}

void USART::InitUSART()
{
	USART_InitTypeDef USART_InitStructure;                       //
	USART_InitStructure.USART_BaudRate            = mBaudrate;  
	USART_InitStructure.USART_WordLength          = mWordLen;
	USART_InitStructure.USART_StopBits            = mStopBits;
	USART_InitStructure.USART_Parity              = mParity ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(mUSARTx, &USART_InitStructure); 
	
	USART_Cmd(mUSARTx, ENABLE);
	
	USART_ITConfig(mUSARTx, USART_IT_RXNE, ENABLE);
	
#ifndef USE_USART_DMA	
	USART_ITConfig(mUSARTx, USART_IT_TC, DISABLE);
#endif	
	USART_ClearFlag(mUSARTx, USART_FLAG_TC);      //clear TC flag to make sure the first byte can be send correctly
}

bool USART::SendBytes(u8 txData[], u16 size)
{
	if(mTxBuf.ResSize()<size)      //compare the unused bytes and sending bytes
	{
		mTxOverflow += size;         //flash Tx overflow bytes
		return false;
	}
	mTxBuf.Puts(txData,size);                        //add data to Tx buffer, if overflow, return false
	if(isBusySend)                return true;       //USARTx is busy send data, return
	if(mTxBuf.Size() <= 0)        return true;       //have no data to send, return
	isBusySend = true;                               //set busy state, going to send data
#ifdef USE_USART_DMA
	if(mTxBuf.Size() <= USART_DMA_TX_BUFFER_SIZE)  //rest data bytes less than DMA buffer size
	{
		mDMATxCh->CNDTR = (u32)mTxBuf.Size();        //send all data to DMA buffer
		mTxBuf.Gets(mDMATxBuf,mTxBuf.Size());
	}
	else                                           //rest data bytes more than DMA buffer size
	{
		mDMATxCh->CNDTR = USART_DMA_TX_BUFFER_SIZE;  //send max DMA buffer size data to DMA buffer
		mTxBuf.Gets(mDMATxBuf,USART_DMA_TX_BUFFER_SIZE);
	}
	DMA_Cmd(mDMATxCh,ENABLE); 	                   //enable DMA to send data
#else	
	USART_ClearITPendingBit(mUSARTx, USART_IT_TC); //Clear TC, otherwise the first byte may not able to send out
	USART_ClearITPendingBit(mUSARTx, USART_IT_TXE); //Clear TC, otherwise the first byte may not able to send out
	USART_ITConfig(mUSARTx, USART_IT_TC, ENABLE);  //Enable TC, going to send data
	//USART_ITConfig(mUSARTx, USART_IT_TXE, ENABLE);  //Enable TC, going to send data
	USART_GetFlagStatus(mUSARTx, USART_FLAG_TC);   //read SR to clear flag, otherwise the first byte may not able to send out
	//USART_GetFlagStatus(mUSARTx, USART_FLAG_TXE);   //read SR to clear flag, otherwise the first byte may not able to send out
	static u8 data=0;
	mTxBuf.Get(data);                              //get one byte data from tx buffer
	USART_SendData(mUSARTx,data);                  //send one byte data
#endif	
	return true;
}


void USART::SetBaudRate(uint32_t baudRate)
{
//	assert_param(IS_USART_BAUDRATE(baudRate));
	uint32_t tmpreg = 0x00, apbclock = 0x00;
	uint32_t integerdivider = 0x00;
	uint32_t fractionaldivider = 0x00;
	uint32_t usartxbase = 0;
	RCC_ClocksTypeDef RCC_ClocksStatus;

	usartxbase = (uint32_t)mUSARTx;

	/*---------------------------- USART BRR Configuration -----------------------*/
	/* Configure the USART Baud Rate -------------------------------------------*/
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	if (usartxbase == USART1_BASE)
	{
	apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	}
	else
	{
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	}

	/* Determine the integer part */
	if ((mUSARTx->CR1 & ((u16)0x8000)) != 0)//CR1_OVER8_Set=((u16)0x8000)
	{
	/* Integer part computing in case Oversampling mode is 8 Samples */
	integerdivider = ((25 * apbclock) / (2 * (baudRate)));    
	}
	else /* if ((USARTx->CR1 & CR1_OVER8_Set) == 0) */
	{
	/* Integer part computing in case Oversampling mode is 16 Samples */
	integerdivider = ((25 * apbclock) / (4 * (baudRate)));    
	}
	tmpreg = (integerdivider / 100) << 4;

	/* Determine the fractional part */
	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	/* Implement the fractional part in the register */
	if ((mUSARTx->CR1 & ((u16)0x8000)) != 0)
	{
	tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
	}
	else /* if ((USARTx->CR1 & CR1_OVER8_Set) == 0) */
	{
	tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
	}

	/* Write to USART BRR */
	mUSARTx->BRR = (uint16_t)tmpreg;
}

bool USART::SendByte(u8 data)
{
	if(mTxBuf.Put(data))
		return true;
	mTxOverflow++;
	return false;
}
bool USART::GetBytes(u8 data[],u16 num)
{
		return mRxBuf.Gets(data,num);
}
bool USART::GetByte(u8 &data)
{
	return mRxBuf.Get(data);
}


u16 USART::RxSize()
{
	return mRxBuf.Size();
}

u16 USART::TxSize()
{
	return mTxBuf.Size();
}

u16 USART::TxOverflowSize()
{
	return mTxOverflow;
}

u16 USART::RxOverflowSize()
{
	return mRxOverflow;
}

void USART::ClearRxBuf()
{
	mRxBuf.Clear();
}

void USART::ClearTxBuf()
{
	mTxBuf.Clear();
}

USART& USART::operator<<(int val)
{
	u8 sign = 0, len = 0,data[10];
	if(val<0)
	{
		sign = 1;
		val = -val;
	}
	do
	{
		len++;
		data[10-len] = val%10 + '0';
		val = val/10;
	}while(val);
	if(sign==1)
		data[10-(++len)] = '-';
	SendBytes(data+10-len,len);
	return *this;
}
USART& USART::operator<<(double val)
{
	u8 sign = 0, len = 0,data[20];
	if(val<0)
	{
		sign = 1;
		val = -val;
	}
	u8 prec = mPrecision;
	while(prec--)
		val *= 10;
	u32 t = val;
	do
	{
		if(++len==mPrecision+1) data[20-len] = '.';
		else
		{
			data[20-len] = t%10 + '0';
			t = t/10;
		}
	}while(t || len<mPrecision+2);
	//if(len==3) data[20-(++len)] = '.';
	//if(len==4) data[20-(++len)] = '0';
	if(sign==1)
		data[20-(++len)] = '-';
	SendBytes(data+20-len,len);
	return *this;
}
USART& USART::operator<<(const char* pStr)
{
	unsigned int length=0;
	for(int i=0;pStr[i]!='\0';++i)
	{
		++length;
	}
	SendBytes((u8*)pStr,length);
	return *this;
}

bool USART::CheckFrame(DataFrame &df)
{
	
	return mRxBuf.CheckFrame(df);;
}


void USART::IRQ()
{	
	if (USART_GetFlagStatus(mUSARTx, USART_FLAG_ORE) != RESET)
	{
			USART_ReceiveData(mUSARTx);
	}
	if(USART_GetITStatus(mUSARTx,USART_IT_RXNE)!=RESET)  //RxNE
	{
		USART_ClearITPendingBit(mUSARTx,USART_IT_RXNE);    //Clear RxNE
		if(!mRxBuf.Put(USART_ReceiveData(mUSARTx)))            //receive byte
		{
			USART1->DR = mUSARTx->DR;
			mRxOverflow++;
		}
	}
	
#ifndef USE_USART_DMA 
	u8 data = 0;
	if(USART_GetITStatus(mUSARTx,USART_IT_TXE)!=RESET || USART_GetITStatus(mUSARTx,USART_IT_TC)!=RESET)   //TxE and TC
	{
		if(mTxBuf.Size()>0)                                //still left some bytes of data
		{
			mTxBuf.Get(data);                                //get one byte data from buffer
			mUSARTx->DR = (data & (u16)0x01FF);              //send one byte data
		}
	  else                                               //all data send complete
		{
			USART_ITConfig(mUSARTx, USART_IT_TXE, DISABLE);  //disable TxE
			USART_ITConfig(mUSARTx, USART_IT_TC, DISABLE);   //disable TC
			isBusySend = false;                              //set free state
		}
	}
#endif
}


#ifdef USE_USART_DMA
void USART::InitDMA()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);                   //enable DMA clock
	
	DMA_DeInit(mDMATxCh);
	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&mUSARTx->DR;	       //DMA Tx data target address
	DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)mDMATxBuf;           //DMA data source address
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;	   //DMA data direction: from memory to peripheral
	DMA_InitStructure.DMA_BufferSize         = 0;                        //data size to send with DMA
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;//peripheral address not increase
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;	   //memory address increase
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//peripheral data unit: Byte
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;	 //memory data unit: Byte
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal ;         //DMA mode: normal (not loop)
	DMA_InitStructure.DMA_Priority           = DMA_Priority_Low;         //DMA priority: low
	DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;          //disable memory to memory
	
	DMA_Init(mDMATxCh, &DMA_InitStructure);                              //config DMA	   
	
	DMA_ITConfig(mDMATxCh, DMA_IT_TC, ENABLE); 
	DMA_ITConfig(mDMATxCh, DMA_IT_TE, ENABLE); 
	
	USART_DMACmd(mUSARTx, USART_DMAReq_Tx, ENABLE);                      //enable DMA Tx of USART
	DMA_Cmd (mDMATxCh,DISABLE);		                                       //disable DMA			
}


void USART::DMAIRQ()
{
	if(DMA_GetFlagStatus(mDMATCFlag)==SET) //DMA Tx Complete
	{
		DMA_ClearITPendingBit(mDMAGLFlag);   //Clear DMA global interrupt flag
		DMA_ClearFlag(mDMATCFlag);           //Clear DMA Tx complete flag
		DMA_Cmd(mDMATxCh,DISABLE);           //close dma
		
		if(mTxBuf.Size()>0)
		{
			if(mTxBuf.Size() <= USART_DMA_TX_BUFFER_SIZE)  //the rest data bytes less than DMA buffer size
			{
				mDMATxCh->CNDTR = mTxBuf.Size();             //send all rest data to DMA buffer 
				mTxBuf.Gets(mDMATxBuf,mTxBuf.Size());		
			}
			else                                           //the rest data bytes more than DMA buffer size
			{
				mDMATxCh->CNDTR = USART_DMA_TX_BUFFER_SIZE;  //send max DMA buffer size data to DMA buffer
				mTxBuf.Gets(mDMATxBuf,USART_DMA_TX_BUFFER_SIZE);
			}
			DMA_Cmd(mDMATxCh,ENABLE); 	                   //enable DMA
		}
		else
			isBusySend = false;               //将忙标志
	}
}

#endif


