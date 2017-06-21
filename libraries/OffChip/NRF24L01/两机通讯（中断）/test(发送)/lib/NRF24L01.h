#ifndef __NRF24L01_H
#define __NRF24L01_H

#include "SPI.h"
#include "FIFOBuffer.h"
#include "stm32f10x.h"

/*******************************************************************************************************/
							/****configuration  使用前自行配置****/
			#define NRF_TX_BUFFER_SIZE 64
			#define NRF_RX_BUFFER_SIZE 64
			
/*******************************************************************************************************/

//----------NRF24L01全局设置--------------
#define TX_ADR_WIDTH 		5  	 //发射地址宽度
#define TX_PLOAD_WIDTH  32   //发射数据通道有效数据宽度0~32Byte
#define RX_ADR_WIDTH    5    //接收地址宽度
#define RX_PLOAD_WIDTH  32   //接收数据通道有效数据宽度0~32Byte
#define CHANAL 					40	 //频道选择

//----------NRF24L01命令------------------
#define NRF_READ_REG    0x00  // Define read command to register
#define NRF_WRITE_REG   0x20  // Define write command to register
#define RD_RX_PLOAD 0x61  // Define RX payload register address
#define WR_TX_PLOAD 0xA0  // Define TX payload register address
#define FLUSH_TX    0xE1  // Define flush TX register command
#define FLUSH_RX    0xE2  // Define flush RX register command
#define REUSE_TX_PL 0xE3  // Define reuse TX payload register command
#define NOP         0xFF  // Define No Operation, might be used to read status register

//----------NRF24L01寄存器地址------------
#define CONFIG      0x00  // 'Config' register address
#define EN_AA       0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR   0x02  // 'Enabled RX addresses' register address
#define SETUP_AW    0x03  // 'Setup address width' register address
#define SETUP_RETR  0x04  // 'Setup Auto. Retrans' register address
#define RF_CH       0x05  // 'RF channel' register address
#define RF_SETUP    0x06  // 'RF setup' register address
#define STATUS      0x07  // 'Status' register address
#define OBSERVE_TX  0x08  // 'Observe TX' register address
#define CD          0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0  0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1  0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2  0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3  0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4  0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5  0x0F  // 'RX address pipe5' register address
#define TX_ADDR     0x10  // 'TX address' register address
#define RX_PW_P0    0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1    0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2    0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3    0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4    0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5    0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS 0x17  // 'FIFO Status Register' register address

//-------NRF24L01中断标志------------
#define MAX_RT  0x10 //达到最大重发次数中断标志位
#define TX_DS		0x20 //发送完成中断标志位	  // 
#define RX_DR		0x40 //接收到数据中断标志位

//------发送状态标志----------
enum { OK = 1, FALSE = 0 };

//-------NRF24L01端口状态------------
//#define NRF_CSN_HIGH()   GPIO_SetBits(GPIOA, GPIO_Pin_4)					//CSN拉高
//#define NRF_CSN_LOW()    GPIO_ResetBits(GPIOA, GPIO_Pin_4)		 	  //CSN置低
//#define NRF_CE_HIGH()	   GPIO_SetBits(GPIOA,GPIO_Pin_1)						//CE拉高
//#define NRF_CE_LOW()	   GPIO_ResetBits(GPIOA,GPIO_Pin_1)			  	//CE置低
//#define NRF_Read_IRQ()	 GPIO_ReadInputDataBit (GPIOA, GPIO_Pin_2)//中断引脚


//-------NRF24L01函数声明------------
class NRF24L01{

private:

	FIFOBuffer<u8,NRF_TX_BUFFER_SIZE>  NRFbufferTx;
	FIFOBuffer<u8,NRF_RX_BUFFER_SIZE>  NRFbufferRx;
	SPI &mSPI;
	GPIO CEPin;
	GPIO IRQPin;
	bool isSending;

public:

	u8 nrf_flag;
	u8 TempBuf[33];
	NRF24L01(SPI &SPIx);

	u8 SPI_NRF_RW(u8 dat);							//读写1字节	
	bool NRF_SendData(uint8_t *pbuffer, uint32_t size);			//发送一串数据
	bool NRF_ReceiverData(u8 *buffer, u16 number);					//接收一串数据

	u8 NRF_ReadReg(u8 reg);								//从特定寄存器读出1字节
	u8 NRF_WriteReg(u8 reg, u8 dat);			//向特定寄存器写入1字节

	u8 NRF_WriteBuf(u8 reg, u8 *pBuf, u8 bytes);	//向特定寄存器写入指定字节数据
	u8 NRF_ReadBuf(u8 reg, u8 *pBuf, u8 bytes);		//从特定寄存器读出指定字节数据

	u8 NRF_RecievePacket(u8 *RxBuf);								//接收一帧数据
	u8 NRF_SendPacket(u8 *TxBuf);										//发送一帧数据

	void NRF_TX_Mode(void);							//进入发送模式
	void NRF_RX_Mode(void);							//进入接收模式

	void NRF_Reset(void);								//NRF软件复位，清除标志，清除缓冲区
	u8 NRF_Check(void);									//检查是否在线
	
	void NRF_CSN_HIGH(){ mSPI.SetCSNPin(SET);}				//CSN拉高
	void NRF_CSN_LOW(){ mSPI.SetCSNPin(RESET);} 	 	  //CSN置低
	
	void SetCEPin(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);//设置CEPin 
	void NRF_CE_HIGH(){CEPin.SetLevel(SET);}	 			//CE拉高
	void NRF_CE_LOW(){CEPin.SetLevel(RESET);}		  	//CE置低
	
	void SetIRQPin(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);//设置IRQPin
	uint8_t NRF_Read_IRQ(void ){
		return IRQPin.GetLevel();
	}		  //读取IRQPin Bit
	void IRQIrq(void);
	
	u8 SendData(uint8_t *pbuffer, uint32_t size);
	bool GetReceivedData(u8* buffer, u16 number);
	
	u16 ReceiveBufferSize();
	u16 SendBufferSize();

	void ClearReceiveBuffer();
	void ClearSendBuffer();
	
};

#endif //__NRF24L01_H




