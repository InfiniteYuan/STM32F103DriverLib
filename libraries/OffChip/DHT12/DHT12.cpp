#include "DHT12.h"

DHT12::DHT12()
{
	IIC_Init();
	V_Temper = 0;
	V_Humid = 0;
}

void DHT12::DHT12_ReadByte(void)
{                   
	uint8_t i;
	
	IIC_Start();
	IIC_Send_Byte(0xB8);
	IIC_Wait_Ack( );

	IIC_Send_Byte(0x00);
	IIC_Wait_Ack( );

	IIC_Start( );
	IIC_Send_Byte(0xB9);
	IIC_Wait_Ack( );
	 
	for(i = 0; i < 4; i++)
	{
		readData[i] = IIC_Read_Byte(1);
	}
	readData[i] = IIC_Read_Byte(0);
	
	IIC_Stop();
} 

void DHT12::Update(void)
{
	DHT12_ReadByte();
	bool isChecked = false;
	if(readData[4] == (readData[0] + readData[1] + readData[2] + readData[3]))  
		isChecked = true;   
	else  
		isChecked = false;
	
	if(isChecked) {
		V_Humid = ((readData[0] + readData[1]/10.0));
		if(readData[3]&0x80)
				V_Temper = -(readData[2] + (readData[3]/10.0));
		else  
				V_Temper = (readData[2] + (readData[3]/10.0));
	}
}

uint16_t DHT12::GetHumid(void)
{
	return V_Humid;
}

uint16_t DHT12::GetTemper(void)
{
	return V_Temper;
}
