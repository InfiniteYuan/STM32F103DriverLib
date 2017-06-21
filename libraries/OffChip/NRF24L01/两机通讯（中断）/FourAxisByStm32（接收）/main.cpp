#include "NRF24L01.h"
#include "SPI.h"
#include "USART.h"
#include "LED.h"
#include "F103_PWM.h"
#include "HMC5883L.h"
#include "mpu6050.h"
#include "I2C.h"
#include "communication.h"

void RCC_Configuration(void);

I2C iic(1);
mpu6050 mpu6050_(iic);
//HMC5883L mag(iic);  //暂时不用
PWM mMotor14(TIM3, false, false, false, false, 200, 1);
USART usart1(1, 115200, true);//Can't use DMA?

SPI spi(SPI1);
NRF24L01 nrf(spi);
Communication mCommunication(nrf);

GPIO ledRedGPIO(GPIOB, 10, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);//LED GPIO
GPIO ledYewGPIO(GPIOB, 11, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);//LED GPIO
LED Red(ledRedGPIO, false);
LED GRE(ledYewGPIO, false);

int main() {

//	u8 pbuf[33] = { 0x01, 0x02, 0x04, 0x05, 0x06,
//		0x01, 0x02, 0x04, 0x05, 0x06,
//		0x01, 0x02, 0x04, 0x05, 0x06,
//		0x01, 0x02, 0x04, 0x05, 0x06,
//		0x01, 0x02, 0x04, 0x05, 0x06,
//		0x01, 0x02, 0x04, 0x05, 0x06 };
//	u8 buf[35];
	RCC_Configuration();
	Red.On();
	GRE.On();

	if (nrf.NRF_Check() == 0){
		usart1 << "NRF24L01 Offline\n";
	} else {
		usart1 << "NRF24L01 Online\n";
	}

	if (!mpu6050_.Init()){
		if (!mpu6050_.Init())
			;
	}

	usart1 << "ALL USART Init Complete!\n";

//	nrf.NRF_TX_Mode();//发送模式
//	while (1){
//		Red.Blink3(GRE, 4, 100);

//		if (nrf.NRF_SendPacket(pbuf) == TX_DS){
//			usart1 << "NRF24L01 Send Success!\n";
//			usart1 << nrf.nrf_flag << "\n";
//		}

	nrf.NRF_RX_Mode();//接收模式
	while (1){
		Red.Blink3(GRE, 4, 100);
		if (nrf.nrf_flag&RX_DR){ //2.4G中断 数据更新
			nrf.nrf_flag &= ~RX_DR;
//			usart1.SendData(nrf.TempBuf, 32);
//			usart1<<"\n\t"<<nrf.ReceiveBufferSize()<<"\n";
			mCommunication.DataListening();
			usart1<<mCommunication.mRcvTargetYaw;
//			nrf.GetReceivedData(buf,32);
//			usart1.SendData(buf,32);
		}

		if (MOD_ERROR == mpu6050_.Update()){
			if (MOD_ERROR == mpu6050_.Update())
				usart1 << "error\n\n\n";
		}
		
		TaskManager::DelayMs(20);
		usart1 << "AccRaw:  " << mpu6050_.GetAccRaw().x << "\t GyrRaw:" << mpu6050_.GetGyrRaw().x << "\n";//*(4.0/65536)
		
		if (mpu6050_.GetHealth() == 1){
			usart1 << "Health!\n";
		}
	}
}


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


//	//测试磁力计是否存在
//	if(!mag.TestConnection(false))
//		usart1<<"mag connection error\n";
//	mag.Init();

//		if (!mag.Update()){
//			if (!mag.Update())
//				usart1 << "error_mag\n";
//			if (!mpu6050_.Init(true))//设置mpu6050为bypass模式
//				mpu6050_.Init(true);
//		}
//		else if (mag.Update()){
//			usart1 << "heading:" << mag.GetDataRaw().x << "\t" << mag.GetDataRaw().y << "\t" << mag.GetDataRaw().x << "\n";
//		}

