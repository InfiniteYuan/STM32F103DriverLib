/**
  *@file Interrupt.h
  *@date 2015-10-27
  *@author 
  *@breif  interrupt management file
  *
  */

#include "Interrupt.h"

/********************************************USART INTERRUPT****************************************************/
#ifdef USE_USART
USART *pUSART1 = 0;
USART *pUSART2 = 0;
USART *pUSART3 = 0;
void USART1_IRQHandler(void)
{
	if(pUSART1)
		pUSART1->Irq();
}
void USART2_IRQHandler(void)
{
	if(pUSART2)
		pUSART2->Irq();
}
void USART3_IRQHandler(void)
{
	if(pUSART3)
		pUSART3->Irq();
}
#endif
/********************************************USART INTERRUP END**********************************************/


/*****************************************I2C EVENT AND ERROR INTERRUPT**************************************/
#ifdef USE_I2C
I2C *pI2C1 = 0;
I2C *pI2C2 = 0;

void I2C1_EV_IRQHandler(void)
{
	if(pI2C1)
		pI2C1->EventIRQ();
}
void I2C2_EV_IRQHandler(void)
{
	if(pI2C2)
		pI2C2->EventIRQ();
}
void I2C1_ER_IRQHandler(void)
{
	if(pI2C1)
		pI2C1->ErrorIRQ();
}
void I2C2_ER_IRQHandler(void)
{
	if(pI2C2)
		pI2C2->ErrorIRQ();
}
#endif
/***********************************I2C EVENT AND ERROR INTERRUPT END **************************************/

/*********************************** DMA  INTERRUPT ********************************************************/
//////////////////////////////////
///USART3 send channel 
//////////////////////////////////
void DMA1_Channel2_IRQHandler()
{	
#ifdef USE_USART
	if(pUSART3)
		pUSART3->DmaIrq();
#endif
}


/////////////////////////////////
///USART1 DMA send channel
///I2C2   DMA send channel
/////////////////////////////////
void DMA1_Channel4_IRQHandler()
{	
#ifdef USE_USART
	if(pUSART1)
		pUSART1->DmaIrq();
#endif
	#ifdef USE_I2C
	#ifdef I2C_USE_DMA
		if(pI2C2)
			pI2C2->DmaTxIRQ();
	#endif
	#endif
}




///////////////////////////////
///I2C2 DMA receive channel
//////////////////////////////
void DMA1_Channel5_IRQHandler()
{
	#ifdef USE_I2C
	#ifdef I2C_USE_DMA
		if(pI2C2)
			pI2C2->DmaRxIRQ();
	#endif
	#endif
}



///////////////////////////////
///I2C1   DMA send channel
//////////////////////////////
void DMA1_Channel6_IRQHandler()
{
#ifdef USE_I2C
#ifdef I2C_USE_DMA
	if(pI2C1)
		pI2C1->DmaTxIRQ();
#endif
#endif
}


/////////////////////////////////
///USART2 DMA send channel
///I2C1   DMA receive channel
////////////////////////////////
void DMA1_Channel7_IRQHandler()
{	
#ifdef USE_USART
	if(pUSART2)
		pUSART2->DmaIrq();
#endif
	
#ifdef USE_I2C
#ifdef I2C_USE_DMA
	if(pI2C1)
		pI2C1->DmaRxIRQ();
#endif
#endif
}


/*****************************************DMA  INTERRUPT END**************************************/

/*****************************************SPI INTERRUPT**************************************/
#ifdef USE_SPI
SPI *pSPI1 = 0;
SPI *pSPI2 = 0;

void SPI1_IRQHandler(void){
	if(pSPI1)
		pSPI1->SpiIrq();
}

void SPI2_IRQHandler(void){
	if(pSPI2)
		pSPI2->SpiIrq();
}
#endif
/*****************************************SPI INTERRUPT END**************************************/

/*****************************************EXTI INTERRUPT START****************************/

#ifdef USE_NRF_IRQ
NRF24L01 *pNRF = 0;

void EXTI2_IRQHandler(void){
	pNRF->IRQIrq();
}

#endif

/*****************************************EXTI INTERRUPT END****************************/

