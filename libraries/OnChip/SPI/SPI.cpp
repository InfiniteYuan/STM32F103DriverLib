#include "SPI.h"
#include "stm32f10x.h"
#include "USART.h"
#include "Interrupt.h"

SPI::SPI(SPI_TypeDef* SPI, bool useDMA, u8 mRemap, u8 Prioritygroup, uint8_t preemprionPriority, uint8_t subPriority, u8 dmaPriority) :isBusySend(0){

	SPIx = SPI;
	isBusySend = 0;
	mUseDma = useDMA;
	GPIO_TypeDef* GPIOx;
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	uint8_t spiIrqChannel, dmaTxIrqChannel;//中断通道，SPI TXDMA通道

	//*RCC_Configuration*//
	RCC_Configuration();

	/*GPIO Configuration*/
	if (SPIx == SPI1){
		spiIrqChannel = SPI1_IRQn;
		GPIOx    = (mRemap ? GPIOB 								: GPIOA								);
		mGPIORcc = (mRemap ? RCC_APB2Periph_GPIOB : RCC_APB2Periph_GPIOA);
		mCSNPin  = (mRemap ? GPIO_Pin_15 					: GPIO_Pin_4          );   //CSN Pin
		mSCKPin  = (mRemap ? GPIO_Pin_3 					: GPIO_Pin_5          );   //SCK Pin
		mMISOPin = (mRemap ? GPIO_Pin_4 					: GPIO_Pin_6          );   //MISO Pin
		mMOSIPin = (mRemap ? GPIO_Pin_5 					: GPIO_Pin_7          );   //MOSI Pin
		
#ifdef USE_SPI1
		pSPI1 = this;
#endif

#ifdef USE_SPI1_DMA
		dmaTxChannel = DMA1_Channel3;
		dmaTxIrqChannel = DMA1_Channel3_IRQn;
		dmaGLFlagChannel = DMA1_IT_GL3;
		dmaTCFlagChannel = DMA1_IT_TC3;
		dmaTEFlagChannel = DMA1_IT_TE3;
		pSPI1 = this;
#endif
	}
	else if (SPIx == SPI2){
		spiIrqChannel = SPI1_IRQn;
		GPIOx    = (mRemap ? GPIOB 								: GPIOB								);
		mGPIORcc = (mRemap ? RCC_APB2Periph_GPIOB : RCC_APB2Periph_GPIOB);
		mCSNPin  = (mRemap ? GPIO_Pin_9 					: GPIO_Pin_12          );   //CSN Pin
		mSCKPin  = (mRemap ? GPIO_Pin_10 					: GPIO_Pin_13          );   //SCK Pin
		mMISOPin = (mRemap ? GPIO_Pin_2 					: GPIO_Pin_14          );   //MISO Pin
		mMOSIPin = (mRemap ? GPIO_Pin_3 					: GPIO_Pin_15          );   //MOSI Pin
		
#ifdef USE_SPI2
		pSPI2 = this;
#endif

#ifdef USE_SPI2_DMA
		dmaTxChannel = DMA1_Channel5;
		dmaTxIrqChannel = DMA1_Channel5_IRQn;
		dmaGLFlagChannel = DMA1_IT_GL5;
		dmaTCFlagChannel = DMA1_IT_TC5;
		dmaTEFlagChannel = DMA1_IT_TE5;
		pSPI2 = this;
#endif
	}

	RCC_APB2PeriphClockCmd(mGPIORcc, ENABLE);

	/*************  GPIO Init  SCKPin MOSIPin MISOPin *************/
	GPIO_InitStructure.GPIO_Pin = mSCKPin | mMOSIPin | mMISOPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOx, mSCKPin | mMISOPin | mMOSIPin);  //mSCKPin | mMOSIPin | mMISOPin上拉

	/********************* SPI Configuration ****************************/
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_Init(SPIx, &SPI_InitStructure);
	SPI_Cmd(SPIx, ENABLE);

	if (mUseDma){

		DMA_InitTypeDef DMA_InitStructure;
		SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);										 /*开启DMA时钟*/
		DMA_InitStructure.DMA_BufferSize = 1;													 			   //传输DMA_BufferSize 初始值设置为1
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						 				 //方向：从内存到外设
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;								 //内存地址自增
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)bufferTxDma;/*dmaChannelTx->CMAR =  (u32)bufferTxDma;*///内存地址（要传输的变量指针）
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPIx->DR;				 //设置DMA源，SPI数据寄存器地址
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			 //外设地址不增
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;				 //外设数据单位8bit
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据单位
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;													 //DMA模式：不循环
		switch (dmaPriority){
			case 0:
				DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
				break;
			case 1:
				DMA_InitStructure.DMA_Priority = DMA_Priority_High;
				break;
			case 2:
				DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
				break;
			default:
				DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
				break;
		}

		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//禁止内存到内存的传输
		DMA_Init(dmaTxChannel, &DMA_InitStructure);//配置DMA1的通道 Tx通道
		DMA_Cmd(dmaTxChannel, DISABLE);//使能DMA
		DMA_ITConfig(dmaTxChannel, DMA_IT_TC | DMA_IT_TE, ENABLE);//打开传输完成中断，打开错误中断
	}
	else {
		//使能SPI 接收缓冲区非空，和发送缓冲区空，错误中断
		SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE | SPI_I2S_IT_ERR, ENABLE);
		SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, DISABLE);
		SPI_I2S_ClearFlag(SPIx, SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_RXNE);//清除标志
		SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_RXNE);
		SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE | SPI_I2S_IT_ERR);
	}

	//中断优先级设置
	NVIC_InitTypeDef NVIC_InitStructure;
	switch (Prioritygroup){
	case 0:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		break;
	case 1:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		break;
	case 2:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		break;
	default:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
		break;
	case 4:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
		break;
	}

	/******************SPI 中断注册************************/
	NVIC_InitStructure.NVIC_IRQChannel = spiIrqChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemprionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/******************DMA 中断注册************************/
	if (mUseDma){
		NVIC_InitStructure.NVIC_IRQChannel = dmaTxIrqChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemprionPriority;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority;
		NVIC_Init(&NVIC_InitStructure);
	}
}

void SPI::SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI1,ENABLE); 
} 

void SPI::RCC_Configuration(){

	/**-------RCC Configuration--------**/
	if (SPIx == SPI1){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	}
	else if (SPIx == SPI2){
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	}
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

u8 SPI::SPI_RW(u8 dat){

	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET); //当 SPI发送缓冲器非空时等待 
	SPI_I2S_SendData(SPIx, dat);	                                  //通过 SPIx发送一字节数据	
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);//当SPI接收缓冲器为空时等待
	return SPI_I2S_ReceiveData(SPIx);                               //Return the byte read from the SPI bus
}

bool SPI::SendData(uint8_t *pbuffer, uint32_t size){

	isBusySend = false;
	SPIbufferTx.Puts(pbuffer, size);//将发送的数据放入发送缓冲区队列
	if (mUseDma && !isBusySend){//使用DMA发送数据
		if(SPIbufferTx.Size()>0){
			isBusySend = true;
			if(SPIbufferTx.Size()<=SPI_DMA_TX_BUFFER_SIZE){
				dmaTxChannel->CNDTR = SPIbufferTx.Size(); //设置DMA待传输数据数目
				SPIbufferTx.Gets(bufferTxDma, SPIbufferTx.Size());//将剩下的数据放入DMA缓冲区
			}else{
				dmaTxChannel->CNDTR = SPI_DMA_TX_BUFFER_SIZE;
				SPIbufferTx.Gets(bufferTxDma, SPI_DMA_TX_BUFFER_SIZE);//传输数据
			}
			DMA_Cmd(dmaTxChannel,ENABLE);
		}
	}
	else if (!mUseDma && !isBusySend){//使用中断发送
		if (SPIbufferTx.Size() > 0){
			isBusySend = true;
			static u8 dataToSend = 0;
			SPIbufferTx.Get(dataToSend);
			SPI_I2S_SendData(SPIx, dataToSend);
			SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_TXE);
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, ENABLE);
			SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE);
		}
	}
	else if (isBusySend){//正在发送数据 或者发生错误
		return false;
	}
	return true;
}

bool SPI::GetReceivedData(u8* buffer, u16 number){
	if (SPIbufferRx.Size() < number){
		return false;
	} else {
		SPIbufferRx.Gets(buffer, number);
		return true;
	}
}

SPIIrqType SPI::SpiIrq(){

	SPIIrqType IrqType;
	if (SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_RXNE) == SET){//接收缓冲区非空

		SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_RXNE);//清除中断标志
		SPIbufferRx.Put(SPI_I2S_ReceiveData(SPIx));
		IrqType = SPI_RXNE_IRQ;
	}
	else if (SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_TXE) == SET){//发送缓冲区空
		
		static u8 dataToSend = 0;
		SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_TXE);//清除中断标志
		if (SPIbufferTx.Size() > 0){
			SPIbufferTx.Get(dataToSend);
			SPI_I2S_SendData(SPIx, dataToSend);//发送缓冲区不为空，发送数据
		}
		else if (SPIbufferTx.Size() == 0){
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, DISABLE);//发送缓冲区空，关闭中断
			isBusySend = false;
		}
		IrqType = SPI_TXE_IRQ;
	}
	else if (SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_ERR) == SET){//错误中断

		SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_ERR);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_ERR);//清除中断标志
		IrqType = SPI_ERR_IRQ;
		isBusySend = false;
	}
	return IrqType;
}

DMAIrqType SPI::DmaIrq(){
	
	DMAIrqType IrqType;
	if(DMA_GetITStatus(dmaTCFlagChannel) == SET){//传输完成中断
		
		DMA_ClearITPendingBit(dmaTCFlagChannel);
		DMA_ClearFlag(dmaTCFlagChannel);
		IrqType = DMA_TE_IRQ;
		DMA_Cmd(dmaTxChannel,DISABLE);
		if(SPIbufferTx.Size()>0){
			if(SPIbufferTx.Size()<=SPI_DMA_TX_BUFFER_SIZE){
				dmaTxChannel->CNDTR = SPIbufferTx.Size(); //设置DMA待传输数据数目
				SPIbufferTx.Gets(bufferTxDma, SPIbufferTx.Size());
			}else{
				dmaTxChannel->CNDTR = SPI_DMA_TX_BUFFER_SIZE;
				SPIbufferTx.Gets(bufferTxDma, SPI_DMA_TX_BUFFER_SIZE);//传输数据
			}
		}else{
			isBusySend = false;
		}
	}else if(DMA_GetITStatus(dmaTEFlagChannel) == SET){//错误中断
	
		DMA_ClearITPendingBit(dmaTEFlagChannel);
		DMA_ClearFlag(dmaTEFlagChannel);
		IrqType = DMA_TE_IRQ;
		isBusySend = false;  //暂时清除总线忙标志
	}
	return IrqType;
}

u16 SPI::ReceiveBufferSize(){
	return SPIbufferRx.Size();
}

u16 SPI::SendBufferSize(){
	return SPIbufferTx.Size();
}

void SPI::ClearReceiveBuffer(){
	SPIbufferRx.Clear();
}

void SPI::ClearSendBuffer(){
	SPIbufferTx.Clear();
}

SPI::~SPI(){
	if(SPI1 == SPIx){
#ifdef USE_SPI1
		pSPI1 = this;
#endif
	}else if(SPI2 == SPIx){
#ifdef USE_SPI2
		pSPI2 = this;
#endif
	}
}

