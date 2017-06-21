#include "NRF24L01.h"
#include "USART.h"
#include "Interrupt.h"

u8 TX_ADDRESS[TX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };  // 定义一个静态发送地址
u8 RX_ADDRESS[RX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };

void Delay(__IO u32 nCount)
{
	for (; nCount != 0; nCount--);
}

u8 NRF24L01::SendData(uint8_t *pbuffer, uint32_t size){//不能使用

//	u8 state;
	u8 dataToSend[32];
//	EXTI->IMR &= ~(1 << 2);			//发送开始时，屏蔽中断线
	mSPI.SetSpeed(SPI_BaudRatePrescaler_8);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）  
	NRFbufferTx.Puts(pbuffer, size);
//	state = NRF_ReadReg(STATUS);
	NRF_CE_LOW();
	if ((size < TX_PLOAD_WIDTH) && (size>0)){//发送数据
		NRFbufferTx.Gets(dataToSend, size);
		NRF_WriteBuf(WR_TX_PLOAD, dataToSend, TX_PLOAD_WIDTH);	 //写数据到TXBUF 32个字节
		NRF_CE_HIGH();  					//CE为高，发送数据
//		EXTI->PR = 1 << 2;					//清除line的中断	
//		EXTI->IMR |= 1 << 2;				//发送结束，不屏蔽中断线
		return TX_DS;
	}
	else if (size >= TX_PLOAD_WIDTH){
		NRFbufferTx.Gets(dataToSend, TX_PLOAD_WIDTH);
		NRF_WriteBuf(WR_TX_PLOAD, dataToSend, TX_PLOAD_WIDTH);	 //写数据到TXBUF 32个字节
		NRF_CE_HIGH();  	//CE为高，发送数据
//		while (NRF_Read_IRQ() != 0);//等待发送完成
//		state =NRF_ReadReg(STATUS);
//		NRF_WriteReg(NRF_WRITE_REG + STATUS, state);				//清除NRF24L01中断标志	
//		EXTI->PR = 1 << 2;					//清除line的中断	
//		EXTI->IMR |= 1 << 2;				//发送结束，不屏蔽中断线
//		if (state&MAX_RT){//最大重发次数
//			NRF_WriteReg(FLUSH_TX, NOP); //清除TX FIFO缓冲区
//			return MAX_RT;
//		}if (state&TX_DS){//发送完成
//			return TX_DS;
//		}
		return nrf_flag;
	}
	return FALSE;
}

NRF24L01::NRF24L01(SPI &SPIx) :mSPI(SPIx), isSending(false){
	pNRF = this;
	SetCEPin(GPIOA, 1);
	SetIRQPin(GPIOA, 2);
	NRF_Reset();
	NRF_CE_LOW();
	NRF_CSN_HIGH();
//	NRF_TX_Mode();//自行配置
}

//NRF24L01读/写一字节数据
u8 NRF24L01::SPI_NRF_RW(u8 dat){
//	u8 retry = 0;
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
//	{
//		retry++;
//		if (retry > 200)return 0;
//	}
//	SPI_I2S_SendData(SPI1, dat); //通过外设SPIx发送一个数据
//	retry = 0;

//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
//	{
//		retry++;
//		if (retry > 200)return 0;
//	}
//	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据
	return mSPI.SPI_RW(dat);
}

bool NRF24L01::NRF_SendData(uint8_t *pbuffer, uint32_t size){			//发送一串数据
	return mSPI.SendData(pbuffer, size);
}

bool NRF24L01::NRF_ReceiverData(u8 *buffer, u16 number){					//接收一串数据
	return mSPI.GetReceivedData(buffer, number);
}

//向NRF24L01特定寄存器写入1个字节数据
u8 NRF24L01::NRF_WriteReg(u8 reg, u8 dat){

	u8 status;
//	NRF_CE_LOW();   //如果有只能接收一次数据，那就是在接收数据后没有拉高CE
	NRF_CSN_LOW();           //置低CSN，使能SPI传输
	status = SPI_NRF_RW(reg);//发送寄存器号	
	SPI_NRF_RW(dat);         //向寄存器写入数据	   
	NRF_CSN_HIGH();					 //CSN拉高，完成		
	return(status);          //返回状态寄存器的值
}

//从NRF24L01特定寄存器读出1个字节数据
u8 NRF24L01::NRF_ReadReg(u8 reg){

	u8 reg_val;
//	NRF_CE_LOW();   //如果有只能接收一次数据，那就是在接收数据后没有拉高CE
	NRF_CSN_LOW();							//置低CSN，使能SPI传输	 
	SPI_NRF_RW(reg); 						//发送寄存器号 
	reg_val = SPI_NRF_RW(NOP);	//读取寄存器的值  	
	NRF_CSN_HIGH();							//CSN拉高，完成  	
	return reg_val;
}

//从特定寄存器读出指定字节数据
u8 NRF24L01::NRF_ReadBuf(u8 reg, u8 *pBuf, u8 bytes){

	u8 status;
	u8 i;

//	NRF_CE_LOW();   //如果有只能接收一次数据，那就是在接收数据后没有拉高CE
	NRF_CSN_LOW();										//置低CSN，使能SPI传输
	status = SPI_NRF_RW(reg); 				//访问特定寄存器

	for (i = 0; i < bytes; i++)							//读取数据 
		pBuf[i] = SPI_NRF_RW(NOP);
	//	NRF_ReceiverData(pBuf,bytes);

	NRF_CSN_HIGH();	//CSN拉高，完成	
	return status;	//返回寄存器状态值
}

//向特定寄存器写入指定字节数据
u8 NRF24L01::NRF_WriteBuf(u8 reg, u8 *pBuf, u8 bytes){

	u8 status;
//	u8 i;

//	NRF_CE_LOW();   //如果有只能接收一次数据，那就是在接收数据后没有拉高CE
	NRF_CSN_LOW();			               //置低CSN，使能SPI传输	
	status = SPI_NRF_RW(reg); //访问发送寄存器

	NRF_SendData(pBuf, bytes);
	//	for (i = 0; i < bytes; i++)		 //向缓冲区写入数据
	//		SPI_NRF_RW(pBuf[i]);

	NRF_CSN_HIGH();		//CSN拉高，完成 
	return (status);	//返回NRF24L01的状态 
}

//从NRF24L01读出一帧数据
//返回OK=1 接收完成，FALSE=0 错误
u8 NRF24L01::NRF_RecievePacket(u8 *RxBuf){

	u8 state;
	mSPI.SetSpeed(SPI_BaudRatePrescaler_8);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）  
	state = NRF_ReadReg(STATUS);
	NRF_WriteReg(NRF_WRITE_REG + STATUS, state);				//清除NRF24L01中断标志	
	if (state&RX_DR){//接收到数据
		NRF_ReadBuf(RD_RX_PLOAD, RxBuf, RX_PLOAD_WIDTH);//读数据	
		NRF_WriteReg(FLUSH_RX, NOP);								   		//清除RX FIFO缓冲区
		return OK;
	}
	return FALSE;
}

//向NRF24L01发送一帧数据
//返回OK=1 接收完成，FALSE=0 错误 MAX_RT 最大重发次数
u8 NRF24L01::NRF_SendPacket(u8 *TxBuf){

	u8 sta;
	EXTI->IMR &= ~(1 << 2);			//发送开始时，屏蔽中断线
	mSPI.SetSpeed(SPI_BaudRatePrescaler_8);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	NRF_CE_LOW();			//进入待机模式
	NRF_WriteBuf(WR_TX_PLOAD, TxBuf, TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
	NRF_CE_HIGH();		//CE拉高 启动发送 
	while (NRF_Read_IRQ() != RESET);//等待发送完成
	sta = NRF_ReadReg(STATUS);  //读取状态寄存器的值	   
	NRF_WriteReg(NRF_WRITE_REG + STATUS, sta); //清除TX_DS或MAX_RT中断标志
	EXTI->PR = 1 << 2;					//清除line的中断	
	EXTI->IMR |= 1 << 2;				//发送结束，不屏蔽中断线
	if (sta&MAX_RT)//达到最大重发次数
	{
		NRF_WriteReg(FLUSH_TX, 0xff);//清除TX FIFO寄存器 
		return MAX_RT;
	}
	if (sta&TX_DS)//发送完成
	{
		return TX_DS;
	}
	return 0xff;//其他原因发送失败
}

//配置并进入接收模式
void NRF24L01::NRF_RX_Mode(void){

	NRF_CE_LOW();
	NRF_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, (u8*)RX_ADDRESS, RX_ADR_WIDTH);//写RX节点地址

	NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);    //使能通道0的自动应答    
	NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);//使能通道0的接收地址  	 
	NRF_WriteReg(NRF_WRITE_REG + RF_CH, CHANAL);	     //设置RF通信频率		  
	NRF_WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
	NRF_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF_WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式
	mSPI.SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz） 
	NRF_WriteReg(FLUSH_RX, 0xff);//清除RX FIFO寄存器 
	NRF_WriteReg(NRF_WRITE_REG + STATUS, 0xff);	//***一定要清空状态寄存器，否则会出错。**
	NRF_CE_HIGH();		//CE拉高，进入接收模式
}


//配置并进入发送模式
void NRF24L01::NRF_TX_Mode(void){

	NRF_CE_LOW();
	NRF_WriteBuf(NRF_WRITE_REG + TX_ADDR, (u8*)TX_ADDRESS, TX_ADR_WIDTH);//写TX节点地址 
	NRF_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, (u8*)RX_ADDRESS, RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  

	NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);     //使能通道0的自动应答    
	NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); //使能通道0的接收地址  
	NRF_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
	NRF_WriteReg(NRF_WRITE_REG + RF_CH, CHANAL);       //设置RF通道为40
	NRF_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF_WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	NRF_WriteReg(FLUSH_TX, 0xff);//清除TX FIFO寄存器 
	NRF_WriteReg(FLUSH_RX, 0xff);//清除RX FIFO寄存器 
	NRF_WriteReg(NRF_WRITE_REG + STATUS, 0xff);	//清除状态寄存器	
	NRF_CE_HIGH();//CE拉高，进入发送模式
}


//NRF24L01存在判断
u8 NRF24L01::NRF_Check(void){

	u8 i, buf[5] = { 0xC2, 0xC2, 0xC2, 0xC2, 0xC2 };
	u8 buf1[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
	mSPI.SetSpeed(SPI_BaudRatePrescaler_4);
	NRF_WriteBuf(NRF_WRITE_REG + TX_ADDR, buf, 5);//写入5个字节的地址
	NRF_ReadBuf(TX_ADDR, buf1, 5); 							//读出写入的地址

	for (i = 0; i < 5; i++){//比较
		if (buf1[i] != 0xC2)
			break;
	}
	if (i == 5)
		return SUCCESS;        //MCU与NRF成功连接 
	else
		return ERROR;        //MCU与NRF不正常连接
}

//NRF24L01配置
void NRF24L01::NRF_Reset(void){

	u8 state;
	state = NRF_ReadReg(STATUS);								//读NRF24L01状态标志
	NRF_WriteReg(NRF_WRITE_REG + STATUS, state);	//清除NRF24L01中断标志	
	NRF_WriteReg(FLUSH_TX, NOP); //清除TX FIFO缓冲区
	NRF_WriteReg(FLUSH_RX, NOP); //清除RX FIFO缓冲区

	NRF_CE_LOW();
	NRF_CSN_HIGH();
}

void NRF24L01::SetIRQPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){

	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	IRQPin = GPIO(GPIOx, GPIO_Pin, GPIO_Mode_IPU, GPIO_Speed_50MHz);
	//将GPIO管脚与外部中断线连接
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);

	//嵌套中断向量配置   发送完成或者接受完成后，中断引脚为由高电平变为低电平 IRQ置低
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	//外部中断配置
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearITPendingBit(EXTI_Line2);
}

void NRF24L01::IRQIrq(void){

	u8 istatus;
	static u8 dataToSend[RX_PLOAD_WIDTH];
	if (EXTI_GetITStatus(EXTI_Line2) != RESET){
		if (IRQPin.GetLevel() == 0){
			istatus = NRF_ReadReg(STATUS);
			nrf_flag = istatus;
			if (istatus & 0x40){	//数据接收中断
				NRF_ReadBuf(RD_RX_PLOAD, TempBuf, RX_PLOAD_WIDTH);//读取数据
				NRF_WriteReg(FLUSH_RX, 0xff);//清楚RX FIFO寄存器
				
				NRFbufferRx.Puts(TempBuf,RX_PLOAD_WIDTH);//放入缓冲区
			}
			else if ((istatus & 0x10) > 0){//达到最大发送次数中断
				NRF_WriteReg(FLUSH_TX, 0xff);//清除TX FIFO寄存器
				isSending = false;
			}
			else if ((istatus & 0x20) > 0){//TX发送完成中断
				NRF_WriteReg(FLUSH_TX, 0xff);//清除TX FIFO寄存器
				if ((NRFbufferTx.Size() > 0) && (NRFbufferTx.Size() <= TX_PLOAD_WIDTH)){
					u8 size = NRFbufferTx.Size();
					NRFbufferTx.Gets(dataToSend, size);
					NRF_WriteBuf(WR_TX_PLOAD, dataToSend, size);
				}
				else if (NRFbufferTx.Size() > TX_PLOAD_WIDTH){
					NRFbufferTx.Gets(dataToSend, TX_PLOAD_WIDTH);
					NRF_WriteBuf(WR_TX_PLOAD, dataToSend, TX_PLOAD_WIDTH);
				}
				else if (NRFbufferTx.Size() == 0){
					isSending = false;
				}
			}
			NRF_WriteReg(NRF_WRITE_REG + STATUS, istatus);//清除状态寄存器
		}
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}

void NRF24L01::SetCEPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	CEPin = GPIO(GPIOx, GPIO_Pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
}

bool NRF24L01::GetReceivedData(u8* buffer, u16 number){
	if (NRFbufferRx.Size() < number){
		return false;
	}
	else {
		NRFbufferRx.Gets(buffer, number);
		return true;
	}
}

u16 NRF24L01::ReceiveBufferSize(){
	return NRFbufferRx.Size();
}

u16 NRF24L01::SendBufferSize(){
	return NRFbufferTx.Size();
}

void NRF24L01::ClearReceiveBuffer(){
	NRFbufferRx.Clear();
}

void NRF24L01::ClearSendBuffer(){
	NRFbufferTx.Clear();
}

