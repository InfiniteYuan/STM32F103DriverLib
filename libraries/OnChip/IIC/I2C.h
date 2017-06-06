#ifndef _I2C_H_
#define _I2C_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "FIFOBuffer.h"
#include "Sensor.h"

#define I2C_QUEUE_SIZE  50


struct I2C_Command
{
	u8 slaveAddr;		      //slave address
	u8 dataOut[5];	      //data send to slave
	u8 outDataLen;	      //number of bytes send to slave
	u8* pDataIn;		      //ponter to receive data from slave
	u8 inDataLen;         //number of bytes to recieve from slave
	u8 isTaskTail;        //
	Sensor *pDevice;   //
};


class I2C
{
	private:
		I2C_TypeDef* mI2C;        //i2c device name
		u32          mSpeed;      //i2c bus speed
		bool         mRemap;      //whether gpio pin remap	
		u16          mSDAPin;     //gpio pin of sda
	  u16          mSCLPin;     //gpio pin of scl	
		u32          mI2CRcc;     //i2c clock
	  u32          mPriGroup;   //Priority Group
		u8           mEvtIRQ;     //i2c event irqn
	  u8           mErrIRQ;     //i2c error irqn
		u8           mEvtPre;     //event preemption priority
	  u8           mEvtSub;     //event sub priority
	  u8           mErrPre;     //error preemption priority
	  u8           mErrSub;     //error sub priority
		u8           mState;      //i2c bus state flag
		FIFOBuffer<I2C_Command,I2C_QUEUE_SIZE> mCmdBank;
	
		u8           mDataIdx;    //index of Tx / Rx data
		u8           mI2CDirection; //I2C Direction (Tx / Rx)
		I2C_Command  mCurCmd;     //Current I2C Command
		int          mErrorCnt;   //I2C Bus error counter
	private:
		bool Reset();
		void InitGPIO(GPIOMode_TypeDef gpioMode);
		void InitI2C();
		void InitNVIC();
		bool StartNextCmd();
	public:
		I2C(I2C_TypeDef* i2c, u32 speed=400000,u8 remap=0,u8 priGroup=3,u8 preEvt=0,u8 subEvt=0,u8 preErr=0,u8 subErr=0);
		bool Initialize();
		bool AddCommand(u8 slaveAddr,u8 txData[], u8 txNum, u8 rxData[], u8 rxNum, Sensor *pDevice, bool isTaskTail);
		bool Start();
		bool IsHealthy();
		bool IsFree();
		bool WaitFree(u16 ms=2);
		void EventIRQ();
		void ErrorIRQ();
		int I2CErrors() { return mErrorCnt; }
};

#define I2C_STATE_NULL  0x00
#define I2C_STATE_START 0x01
#define I2C_STATE_ADDR  0x02
#define I2C_STATE_TXMOD 0x03
#define I2C_STATE_RXMOD 0x04
#define I2C_STATE_TX    0x05
#define I2C_STATE_RX    0x06
#define I2C_STATE_STOP  0x07
#define I2C_STATE_FREE  0x08
#define I2C_STATE_ERROR 0x09

#endif


