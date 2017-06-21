#ifndef _SPI_H
#define _SPI_H


#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "FIFOBuffer.h"
#include "GPIO.h"


/******************************************************************************************************/
							/****configuration  使用前自行配置****/
		#define SPI_TX_BUFFER_SIZE     64              //SPI BUFFER FIFO SIZE
		#define SPI_RX_BUFFER_SIZE     64              //SPI BUFFER FIFO SIZE
		#define SPI_DMA_TX_BUFFER_SIZE 20               //SPI DMA BUFFER SIZE
		
/*******************************************************************************************************/

typedef enum
{
	SPI_RXNE_IRQ,				//SPI接收缓存区非空中断
	SPI_TXE_IRQ,				//SPI发送缓冲区空中断
	SPI_ERR_IRQ					//SPI错误中断
}SPIIrqType;

typedef enum
{
	DMA_HT_IRQ,					//DMA传输过半中断
	DMA_TC_IRQ,					//DMA传输完成中断
	DMA_TE_IRQ					//DMA错误中断
}DMAIrqType;

class SPI{

private:

	FIFOBuffer<u8,SPI_TX_BUFFER_SIZE>  SPIbufferTx;
	FIFOBuffer<u8,SPI_RX_BUFFER_SIZE>  SPIbufferRx;
	u8 bufferTxDma[SPI_DMA_TX_BUFFER_SIZE];
	DMA_Channel_TypeDef* dmaChannel;				//dma通道
	uint32_t dmaTCFlagChannel;								//dma传输完成中断标志位
	uint32_t dmaGLFlagChannel;								//dma全局中断标志位
	uint32_t dmaTEFlagChannel;								//dma错误中断标志位
	bool isBusySend;
	bool mUseDma;
	unsigned char mPrecision;
/*-------SPI Pin Configuration------*/
	SPI_TypeDef* SPIx;
	GPIO mSCK;
	GPIO mCSN;
	GPIO mMISO;
	GPIO mMOSI;

	void RCC_Configuration(void);

public:
	
	SPI(SPI_TypeDef* SPI, bool useDMA = false, u8 remap = 0, u8 Prioritygroup = 3,uint8_t preemprionPriority = 7,uint8_t subPriority = 1,u8 dmaPriority = 3);
	~SPI();
	
	u8 SPI_RW(u8 dat);
	bool SendData(uint8_t *pbuffer, uint32_t size);
	bool GetReceivedData(u8* buffer, u16 number);

	void SetSpeed(u8 SPI_BaudRatePrescaler);

	u16 ReceiveBufferSize();
	u16 SendBufferSize();

	void ClearReceiveBuffer();
	void ClearSendBuffer();

	GPIO& GetmMISOx();
	GPIO& GetmMOSIx();

	SPI_TypeDef* getSPI();
	bool UseDma();

	SPIIrqType SpiIrq();
	DMAIrqType DmaIrq();
	
	void SetCSNPin(u8 value);

};


#endif

