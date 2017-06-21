#include "NRF24L01.h"
#include "SPI.h"
#include "USART.h"
#include "LED.h"
#include "F103_PWM.h"
#include "HMC5883L.h"
#include "mpu6050.h"
#include "I2C.h"

void RCC_Configuration(void){

	SystemInit();
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);

	if (RCC_WaitForHSEStartUp() == SUCCESS) {

		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLK2Config(RCC_HCLK_Div1);
		RCC_PCLK1Config(RCC_HCLK_Div2);

		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		RCC_PLLCmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while (RCC_GetSYSCLKSource() != 0x08);
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//打开GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		//打开AFIO时钟
}

I2C iic(1);
mpu6050 mpu6050_(iic);
//HMC5883L mag(iic);

SPI spi(SPI1);
NRF24L01 nrf(spi);
USART usart1(1, 115200, false);//Can't use DMA?

//	PWM mMotor14(TIM3, true, true, true, true, 200, 1);

GPIO ledRedGPIO(GPIOB, 10, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);//LED GPIO
GPIO ledYewGPIO(GPIOB, 11, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);//LED GPIO
LED Red(ledRedGPIO, false);
LED GRE(ledYewGPIO, false);

int main(void){

	u8 pbuf[33] = { 0xAA, 0xAA, 0x03, 0x0A, 0x01,
									0x04, 0x01, 0x0A, 0x01, 0x15,
									0x01, 0x14, 0x01, 0x04, 0xA1,
									0x01, 0x02, 0x04, 0x05, 0x06,
									0x01, 0x02, 0x04, 0x05, 0x06,
									0x01, 0x02, 0x04, 0x05, 0x06 };
	RCC_Configuration();
	Red.On();
	GRE.On();

	if (nrf.NRF_Check() == 0){
		if (nrf.NRF_Check() == 0)
			usart1 << "NRF24L01 Offline\n";
	} else {
		usart1 << "NRF24L01 Online\n";
	}

	if (!mpu6050_.Init()){
		if (!mpu6050_.Init())
			;
	}

	usart1 << "ALL USART Init Complete!\n";
//		nrf.NRF_RX_Mode();//接收模式
//	while (1){
//		Red.Blink3(GRE, 4, 100);
//		if (nrf.nrf_flag&RX_DR){
//			nrf.nrf_flag&=~RX_DR;
//			usart1.SendData(nrf.TempBuf,33);
//		}
	
		nrf.NRF_TX_Mode();//发送模式
		while (1){
		Red.Blink3(GRE, 4, 100);

		if (nrf.SendData(pbuf,32) == TX_DS){
			usart1 << "NRF24L01 Send Success!\n";
			usart1 << nrf.nrf_flag << "\n";
		}

//		if (MOD_ERROR == mpu6050_.Update())
//		{
//			if (MOD_ERROR == mpu6050_.Update())
//				usart1 << "error\n\n\n";
//		}

//			if(!mag.Update()){
//			usart1<<"error_mag\n";
//			if(!mpu6050_.Init(true))//??mpu6050?bypass??
//				mpu6050_.Init(true);
//		} else if(mag.Update()){
//			usart1<<"heading:"<<mag.GetDataRaw().x<<"\t"<<mag.GetDataRaw().y<<"\t"<<mag.GetDataRaw().x<<"\n";
//		}
//		
//		usart1<<"Test:"<<mag.TestConnection(false)<<"\n";

//		usart1 << "AccRaw:  " << mpu6050_.GetAccRaw().z << "\t GyrRaw:" << mpu6050_.GetGyrRaw().z << "\n";//*(4.0/65536)

//		if (mpu6050_.GetHealth() == 1){
//			usart1 << "Health!\n";
//		}

	}
}

void HardFault_Handler(void)
{

	if (CoreDebug->DHCSR & 1) {  //check C_DEBUGEN == 1 -> Debugger Connected  
		__breakpoint(0);  // halt program execution here         
	}
	while (1)
	{
	}
}




