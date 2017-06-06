#include "MS561101.h"
#include "TaskManager.h"

MS561101::MS561101(I2C &i2c):mI2C(i2c)
{
	Initialize();
}


bool MS561101::Initialize()
{
	mConvertPressure = false;                          //Convert temperature fisrt
	mHealthy = false;                                  //set MS561101 not healthy
	if(!mI2C.WaitFree(50))                             //wait for i2c work and free
		if(!mI2C.Initialize()) return false;             //if time out, reset and initialize i2c
	
	u8 cmd = MS561101BA_RESET;
  mI2C.AddCommand(MS561101BA_ADDR, &cmd,   1,      0,      0,   this, true); //reset ms561101
	if(!mI2C.Start())      return false;  //start to rum i2c command
	if(!mI2C.WaitFree(50)) return false;	
	for(volatile u32 i=0; i<500000;i++);
	
	for(u8 i=0; i<6; i++)    //read rom data of ms561101
  {
		bool isTaskTail = ((i==5) ? true : false);
		cmd = MS561101BA_PROM_BASE_ADDR + i*2;
		//Get Rom data:    i2c addr    txdata txNum rxdata[]  rxNum
		mI2C.AddCommand(MS561101BA_ADDR, &cmd, 1, mRomData+i*2, 2,   this, isTaskTail);
	}
	if(!mI2C.Start())      return false;               //start to rum i2c command	
	if(!mI2C.WaitFree(50)) return false;               //wait MS5611 initialize complete, if time out, keep nuhealthy state
	mHealthy = true;                                   //initialize success
	for(u8 i=0;i<6;i++) 
		mC[i] = u16((mRomData[i*2]<<8) | mRomData[i*2+1]); //Calculate C1,C2,C3,C4,C5,C6
	return true;
}

bool MS561101::Update(float &pressure)
{
	static double curTime;
	curTime = tskmgr.Time();
	if(!mI2C.IsHealthy())   //if i2c not work correctly
	{
		mI2C.Initialize();    //initialize i2c
		Initialize();	        //initialize MS561101
		return false;
	}
	if(mIsUpdated==false) 
	{
		if(curTime- mUpdatedTime > 1)
		{
			mI2C.Initialize();    //initialize i2c
			Initialize();	        //initialize MS561101
			mIsUpdated = true;
		}
		return false;
	}
	if(curTime-mUpdatedTime<0.01) return false; //minimal interval > 10ms = 0.01s
	
	mIsUpdated = false;
	
	static u8 cmd[3] = {0,MS561101BA_D1+MS561101BA_OSR_4096,MS561101BA_D2+MS561101BA_OSR_4096};
	if(mConvertPressure)
	{
		mI2C.AddCommand(MS561101BA_ADDR, cmd,   1, mRawT, 3,   this, false);  //read temperature data
		mI2C.AddCommand(MS561101BA_ADDR, cmd+1, 1, 0,     0,   this, true);  //read temperature data
	}
	else
	{
		mI2C.AddCommand(MS561101BA_ADDR, cmd,   1, mRawP,    3,   this, false);  //read pressure data
		mI2C.AddCommand(MS561101BA_ADDR, cmd+2, 1, 0,        0,   this, true);  //read temperature data
	}
	mConvertPressure = !mConvertPressure;
	
	pressure = Pressure();
	
	return true;
}

float MS561101::Temperature()
{
	static int64_t deltaT;
	static int32_t temperature;
	deltaT = int32_t(((u32)(mRawT[0]<<16))|((u32)(mRawT[1])<<8)|mRawT[2]) - u32(mC[4]<<8);
	temperature = 2000 + ((deltaT*mC[5])>>23);
	if(temperature<2000)
		temperature = temperature - ((deltaT*deltaT)>>31);
	return temperature/100.0f;
}
float MS561101::Pressure()
{
	int64_t OFF2=0;
	int64_t SENS2=0;
	int64_t dT = ((u32)(mRawT[0]<<16)|(u32)(mRawT[1]<<8)|mRawT[2])   -  (mC[4]<<8 );	
	int64_t OFF =(int64_t(mC[1])<<16) + ((int64_t(mC[3]*dT))>>7);
	int64_t SENS=((int64_t)(mC[0])<<15) + ( ((int64_t)(mC[2])*dT) >>8 );
	int32_t TEMP=2000 + ((dT*(mC[5]))>>23);
	
	//SECOND ORDER TEMPERATURE COMPENSATION
	if(TEMP<2000)
	{
		OFF2=(5*(TEMP-2000)*(TEMP-2000))>>1;
		SENS2=(5*(TEMP-2000)*(TEMP-2000))>>2;
		if(TEMP<-1500)
		{
			OFF2 = OFF2 + 7*(TEMP+1500)*(TEMP+1500);
			SENS2 = SENS2 + ((11*(TEMP+1500)*(TEMP+1500))>>1);
		}
	}
	OFF  -= OFF2;
	SENS -= SENS2;
	return (((((mRawP[0]<<16|mRawP[1]<<8|mRawP[2]) *SENS) >>21) - OFF)>>15)/100.0f;
}

