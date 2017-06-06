#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_
#include "stm32f10x.h"

class TaskManager
{
private:
	static u16 _lose_tick_cnt;		 //current updated time
	static double _old_time;     //last updated time
public:
	static double _it_time;      //time = SysTick interrupt counter*1.8s
public:		
	TaskManager();               //constructor
	static double Time();    //get current time
	static void Update();
};
extern TaskManager tskmgr;

#define	MOD_ERROR  0x00    //error
#define	MOD_READY  0x01    //ready
#define	MOD_BUSY   0x02    //busy
#define	MOD_LOCK   0x04    //locked
#define	MOD_UNLOCK 0x08    //unlocked
#define	MOD_ADJUST 0x10    //adjust, calibrate

#endif
