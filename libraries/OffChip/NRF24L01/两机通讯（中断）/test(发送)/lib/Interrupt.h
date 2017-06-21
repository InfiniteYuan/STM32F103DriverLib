/**
  *@file Interrupt.h
  *@date 2015-10-27
  *@author 
  *@breif  interrupt management file
  *
  */

#ifndef __INTERRUPT_H
#define __INTERRUPT_H

/***************************************configuration********************************************/
#define USE_USART
#define USE_I2C
#define USE_SPI
#define USE_NRF_IRQ
/************************************configuration end********************************************/

#ifdef USE_NRF_IRQ
#include "NRF24L01.h"
extern NRF24L01 *pNRF;
extern "C"{
	void EXTI2_IRQHandler(void);
}
#endif

#ifdef USE_USART
#include "USART.h"
extern USART *pUSART1;
extern USART *pUSART2;
extern USART *pUSART3;
extern "C"{
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
	
}
#endif

#ifdef USE_I2C
#include "I2C.h"
extern I2C *pI2C1;
extern I2C *pI2C2;
extern "C"{
void I2C1_EV_IRQHandler(void);
void I2C2_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C2_ER_IRQHandler(void);	
}
#endif

#ifdef USE_SPI
#include "SPI.h"
extern SPI *pSPI1;
extern SPI *pSPI2;
extern "C"{
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
}
#endif

extern "C"{
//////////////////////////////////
///USART3 send channel 
//////////////////////////////////
void DMA1_Channel2_IRQHandler();
/////////////////////////////////
///USART1 DMA send channel
///I2C2   DMA send channel
/////////////////////////////////
void DMA1_Channel4_IRQHandler();
///////////////////////////////
///I2C2 DMA receive channel
//////////////////////////////
void DMA1_Channel5_IRQHandler();

///////////////////////////////
///I2C1   DMA send channel
//////////////////////////////
void DMA1_Channel6_IRQHandler();

/////////////////////////////////
///USART2 DMA send channel
///I2C1   DMA receive channel
////////////////////////////////
void DMA1_Channel7_IRQHandler();
}

#endif
