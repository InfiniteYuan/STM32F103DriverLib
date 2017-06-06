#include "SoftwareI2C.h"
#include "TaskManager.h"

//初始化IIC
void IIC_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	//PB6,PB7 输出高
	IIC_SCL = 1;
	IIC_SDA = 1;
}
//I2C起始信号
void IIC_Start(void)
{
	SDA_OUT();
	IIC_SDA = 1;
	IIC_SCL = 1;
	TaskManager::DelayUs(4);
	IIC_SDA = 0;//START:when CLK is high,DATA change form high to low 
	TaskManager::DelayUs(4);
	IIC_SCL = 0;
	TaskManager::DelayUs(4);
}
//I2C停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda输出
	IIC_SCL = 0;
	IIC_SDA = 0;//STOP:when CLK is high DATA change form low to high
	TaskManager::DelayUs(4);
	IIC_SCL = 1;
	TaskManager::DelayUs(4);
	IIC_SDA = 1;//发送I2C总线结束信号
	TaskManager::DelayUs(4);
}
//等待应答信号到来
//返回值：1 接收应答失败
//				0 接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;
	SDA_IN();      //SDA设置为输入
	IIC_SDA = 1; 
	TaskManager::DelayUs(4);
	IIC_SCL = 1; 
	TaskManager::DelayUs(4);
	while (READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
		TaskManager::DelayUs(1);
	}
	IIC_SCL = 0;//时钟输出0
	return 0;
}
//ACK
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	TaskManager::DelayUs(5);
	IIC_SCL = 1;
	TaskManager::DelayUs(5);
	IIC_SCL = 0;
}
//ACK
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	TaskManager::DelayUs(5);
	IIC_SCL = 1;
	TaskManager::DelayUs(5);
	IIC_SCL = 0;
}
//IIC发送一个字节
//返回从机有无应答
//1,有应答
//0,无应答	  
void IIC_Send_Byte(u8 txd)
{
	u8 t;
	SDA_OUT();
	IIC_SCL = 0;//拉低时钟开始数据传输
	for (t = 0; t < 8; t++)
	{
		IIC_SDA = (txd & 0x80) >> 7;
		txd <<= 1;
		TaskManager::DelayUs(2);
		IIC_SCL = 1;
		TaskManager::DelayUs(2);
		IIC_SCL = 0;
		TaskManager::DelayUs(2);
	}
}
//读一个字节，ack=1时,发送ACK,ack=0,发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SDA_IN();//SDA设置为输入
	for (i = 0; i < 8; i++)
	{
		IIC_SCL = 0;
		TaskManager::DelayUs(50);
		IIC_SCL = 1;
		TaskManager::DelayUs(50);
		receive <<= 1;
		if (READ_SDA)
			receive++;
	}
	if (!ack)
		IIC_NAck();//发送nACK
	else
		IIC_Ack(); //发送ACK   
	return receive;
}
//**************************************
//向I2C设备写入一个数据
//**************************************
void Single_WriteI2C(u8 SlaveAddress, u8 REG_Address, u8 REG_data)
{
	IIC_Start();                  //起始信号
	IIC_Send_Byte(SlaveAddress);  //发送设备地址+写信号
	IIC_Wait_Ack();
	IIC_Send_Byte(REG_Address);   //发送存储单元地址，从0开始
	IIC_Wait_Ack();
	IIC_Send_Byte(REG_data);      //内部寄存器数据
	IIC_Wait_Ack();
	IIC_Stop();                   //停止信号
}
//**************************************
//从I2C设备读取一个字节数据
//**************************************
u8 Single_ReadI2C(u8 SlaveAddress, u8 REG_Address)
{
	u8 REG_data;
	IIC_Start();                    //起始信号
	IIC_Send_Byte(SlaveAddress);    //发送设备地址+写信号
	REG_data = IIC_Wait_Ack();
	IIC_Send_Byte(REG_Address);     //发送存储单元地址，从0开始
	REG_data = IIC_Wait_Ack();
	IIC_Start();                    //起始信号
	IIC_Send_Byte(SlaveAddress + 1);//发送设备地址+读信号
	REG_data = IIC_Wait_Ack();
	REG_data = IIC_Read_Byte(0);   //读出寄存器数据，接收应答信号
	IIC_Stop();                    //停止信号
	return REG_data;
}
