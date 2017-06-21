/**
  *@file Delay.cpp
  *@author Neutree
  *@breif Delay Class
  *@copyright CQUT IOT Lib all rights reserved
  */

#include "Delay.h"



void Delay::Ms(uint16_t nms)
{
	u32 temp;
	if (nms == 0)
		return;
	SysTick->LOAD = 9000 * nms;
	SysTick->VAL = 0X00;//清空计数器
	SysTick->CTRL = 0X01;//使能，减到零是无动作，采用外部时钟源
	do
	{
		temp = SysTick->CTRL;//读取当前倒计数值
	} while ((temp & 0x01) && (!(temp&(1 << 16))));//等待时间到达
	SysTick->CTRL = 0x00; //关闭计数器
	SysTick->VAL = 0X00; //清空计数器
}


void Delay::Us(uint32_t nus)
{
	u32 temp;
	if (nus == 0)
		return;
	SysTick->LOAD = 9 * nus;
	SysTick->VAL = 0X00;//清空计数器
	SysTick->CTRL = 0X01;//使能，减到零是无动作，采用外部时钟源
	do
	{
		temp = SysTick->CTRL;//读取当前倒计数值
	} while ((temp & 0x01) && (!(temp&(1 << 16))));//等待时间到达

	SysTick->CTRL = 0x00; //关闭计数器
	SysTick->VAL = 0X00; //清空计数器
}



void Delay::S(uint32_t ns)
{
	u32 i;
	u32 temp;
	if (ns == 0)
		return;
	SysTick->LOAD = 9000000;//1S
	SysTick->VAL = 0X00;//清空计数器
	SysTick->CTRL = 0X01;//使能，减到零是无动作，采用外部时钟源
	for (i = 0; i < ns; ++i)
	{
		do
		{
			temp = SysTick->CTRL;//读取当前倒计数值
		} while ((temp & 0x01) && (!(temp&(1 << 16))));//等待时间到达
		SysTick->LOAD = 9000000;//1S
	}
	SysTick->CTRL = 0x00; //关闭计数器
	SysTick->VAL = 0X00; //清空计数器
}

