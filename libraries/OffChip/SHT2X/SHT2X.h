#ifndef __SHT2X_H__
#define __SHT2X_H__
#include "sys.h"

#define  I2C_ADR_W 0x80  //SHT2X IIC 读地址
#define  I2C_ADR_R 0x81  //SHT2X IIC 写地址

typedef enum {
	TRIG_TEMP_MEASUREMENT_HM = 0xE3,	 // 触发 温度 测量 ，保持主机   命令
	TRIG_HUMI_MEASUREMENT_HM = 0xE5,	 // 触发 湿度 测量 ，保持主机   命令
	TRIG_TEMP_MEASUREMENT_POLL = 0xF3, // 触发 温度 测量 ，非保持主机 命令
	TRIG_HUMI_MEASUREMENT_POLL = 0xF5, // 触发 湿度 测量 ，非保持主机 命令
	USER_REG_W = 0xE6, // 写寄存器命令
	USER_REG_R = 0xE7, // 读寄存器命令
	SOFT_RESET = 0xFE  // 软件复位命令
} SHT2xCommand;

typedef enum {
	SHT2x_RES_12_14BIT = 0x00, //RH=12bit, T=14bit 这是默认的值   我们不用修改
	SHT2x_RES_8_12BIT = 0x01,  //RH= 8bit, T=12bit
	SHT2x_RES_10_13BIT = 0x80, //RH=10bit, T=13bit
	SHT2x_RES_11_11BIT = 0x81, //RH=11bit, T=11bit
	SHT2x_RES_MASK = 0x81		   //Mask for res. bits (7,0) in user reg.
} SHT2xResolution;

typedef enum {
	SHT2x_HEATER_ON = 0x04,		 //heater on
	SHT2x_HEATER_OFF = 0x00,	 //heater off
	SHT2x_HEATER_MASK = 0x04   //Mask for Heater bit(2) in user reg.
} SHT2xHeater;

typedef struct{
	float TEMP_POLL;
	float HUMI_POLL;
} SHT2x_data;

class SHT2X{

public:
	SHT2X();
	SHT2x_data mSHT20data;
	u8 SHT2x_Init(void);

	u8    SHT2x_SoftReset(void);  //SHT20软件复位
	float SHT2x_GetTempPoll(void);//获取SHT20 温度
	float SHT2x_GetHumiPoll(void);//获取SHT20 湿度

	void Update();
};
#endif
