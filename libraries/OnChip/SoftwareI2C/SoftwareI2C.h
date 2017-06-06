#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h" 

#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)8<<28;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)3<<28;}

#define IIC_SCL    PBout(6) //SCL
#define IIC_SDA    PBout(7) //SDA	 
#define READ_SDA   PBin(7)  //输入SDA 

void IIC_Init(void);        			 	//初始化IIC的IO口			 
void IIC_Start(void);							 	//发送IIC开始信号
void IIC_Stop(void);	  					 	//发送IIC停止信号
void IIC_Send_Byte(u8 txd);					//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 							//IIC等待ACK信号
void IIC_Ack(void);									//IIC发送ACK信号
void IIC_NAck(void);								//IIC不发送ACK信号

u8 Single_ReadI2C(u8 SlaveAddress, u8 REG_Address);
void Single_WriteI2C(u8 SlaveAddress, u8 REG_Address, u8 REG_data);

#endif

