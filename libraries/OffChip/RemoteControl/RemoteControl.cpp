#include "RemoteControl.h"


RemoteControl::RemoteControl(InputCapture *hunter1,u8 Pit,u8 Thr,u8 Yaw,u8 Rol)
{
	State=REMOTECONTROL_LOCK;
	
	Limit_MAX=RC_DEFAULT_LIMIT_MAX;	
	Limit_MIN=RC_DEFAULT_LIMIT_MIN;
	
	OriginalPitMax=FUTABA_ORIGINAL_MAX;
	OriginalPitMin=FUTABA_ORIGINAL_MIN;

	OriginalThrMax=FUTABA_ORIGINAL_MAX;
	OriginalThrMin=FUTABA_ORIGINAL_MIN;

	OriginalYawMax=FUTABA_ORIGINAL_MAX;
	OriginalYawMin=FUTABA_ORIGINAL_MIN;

	OriginalRolMax=FUTABA_ORIGINAL_MAX;
	OriginalRolMin=FUTABA_ORIGINAL_MIN;
	
	mHunter1 = hunter1;
	mPitchCh1=Pit;
	mThrottleCh2=Thr;
	mYawCh3=Yaw;
	mRollCh4=Rol;
	
	mHunter2=0;
	
	//The default state to choose(默认状态选择)
	LockPosture = LOWERRIGHT_LOWERLEFT;			//内八字 
	UnlockPosture = LOWERLEFT_LOWERRIGHT;		//外八字
	LostPosture = LOWERLEFT_LOWERLEFT;        	//左下左下
	CalibrationPosture = LOWERLEFT_UPPERROGHT; //左下+右上
}
//More than four channels(超过四个通道的构造函数)
RemoteControl::RemoteControl(InputCapture *hunter1,u8 Pit,u8 Thr,u8 Yaw,u8 Rol,InputCapture *hunter2,u8 Ch5,u8 Ch6,u8 Ch7,u8 Ch8)
{
	State=REMOTECONTROL_LOCK;
	
	Limit_MAX=RC_DEFAULT_LIMIT_MAX;	
	Limit_MIN=RC_DEFAULT_LIMIT_MIN;
	
	OriginalPitMax=FUTABA_ORIGINAL_MAX;
	OriginalPitMin=FUTABA_ORIGINAL_MIN;

	OriginalThrMax=FUTABA_ORIGINAL_MAX;
	OriginalThrMin=FUTABA_ORIGINAL_MIN;

	OriginalYawMax=FUTABA_ORIGINAL_MAX;
	OriginalYawMin=FUTABA_ORIGINAL_MIN;

	OriginalRolMax=FUTABA_ORIGINAL_MAX;
	OriginalRolMin=FUTABA_ORIGINAL_MIN;
	
	OriginalModelMax=FUTABA_ORIGINAL_MAX;
	OriginalModelMin=FUTABA_ORIGINAL_MIN;

	OriginalAdd6Max=FUTABA_ORIGINAL_MAX;
	OriginalAdd6Min=FUTABA_ORIGINAL_MIN;

	OriginalAdd7Max=FUTABA_ORIGINAL_MAX;
	OriginalAdd7Min=FUTABA_ORIGINAL_MIN;

	OriginalAdd8Max=FUTABA_ORIGINAL_MAX;
	OriginalAdd8Min=FUTABA_ORIGINAL_MIN;
	
	mHunter1 = hunter1;
	mPitchCh1=Pit;
	mThrottleCh2=Thr;
	mYawCh3=Yaw;
	mRollCh4=Rol;
	
	mHunter2 = hunter2;
	mModelCh5=Ch5;
	mCh6=Ch6;
	mCh7=Ch7;
	mCh8=Ch8;
	
}

RemoteControl::~RemoteControl()
{}
	
//Scope of transformation(将从硬件捕获得到的值（油门、偏航、横滚、俯仰）转换为上层可以理解的值)
uint16_t RemoteControl::LimitsConversion(uint16_t OriginalData, uint16_t OriginalMax,uint16_t OriginalMin)
{
	return (OriginalData-OriginalMin)*1.0/(OriginalMax-OriginalMin)*(Limit_MAX-Limit_MIN)+Limit_MIN;
}



//Set the output range
void RemoteControl::Set_Limit_MAX_MIN(u16 max,u16 min)
{
	 Limit_MAX=max;
	 Limit_MIN=min;
}


//Data update and state judgment
u8 RemoteControl::Updata(u16 time_ms,u16 Time_Of_Duration_ms)
{
	static u16 Lock_cnt=0,Unlock_cnt=0,Calibration_cnt=0;
	u16 cnt=(u16)(Time_Of_Duration_ms/time_ms);//次数=触发命令需要保持舵量为特定值持续的时长 / 每两次之间进本函数的间隔

	//Save after processing of the original value, in the range 0 to 1000(保存处理后的原始值，范围在0到1000)
	mPITCH = LimitsConversion((*mHunter1)[mPitchCh1]*1000,OriginalPitMax,OriginalPitMin);
	mTHROTTLE = LimitsConversion((*mHunter1)[mThrottleCh2]*1000,OriginalThrMax,OriginalThrMin);
	mYAW = LimitsConversion((*mHunter1)[mYawCh3]*1000,OriginalYawMax,OriginalYawMin);
	mROLL = LimitsConversion((*mHunter1)[mRollCh4]*1000,OriginalRolMax,OriginalRolMin);
	//Prevent the output of the original value is too small and beyond(防止由于输出原始值过小而超出范围)
	if(mPITCH>Limit_MAX)
		mPITCH=Limit_MAX;
	if(mTHROTTLE>Limit_MAX)
		mTHROTTLE=Limit_MAX;
	if(mYAW>Limit_MAX)
		mYAW=Limit_MAX;
	if(mROLL>Limit_MAX)
		mROLL=Limit_MAX;

	if(mHunter2)
	{
		if(mModelCh5)
		mModel=LimitsConversion((*mHunter2)[mModelCh5]*1000,OriginalModelMax,OriginalModelMin);
		if(mCh6)
		mAdditional6=LimitsConversion((*mHunter2)[mCh6]*1000,OriginalAdd6Max,OriginalAdd6Min);
		if(mCh7)
		mAdditional7=LimitsConversion((*mHunter2)[mCh7]*1000,OriginalAdd7Max,OriginalAdd7Min);
		if(mCh8)
		mAdditional8=LimitsConversion((*mHunter2)[mCh8]*1000,OriginalAdd8Max,OriginalAdd8Min);	
	}
	
	
	if(State == REMOTECONTROL_CALIBRATION)
	{
		StartCalibration();//校准
	}
	//state judgment
	if(StateSelection(LockPosture))  // If the judge for the locked state(如果被判断为上锁状态)
	{
		Lock_cnt++;   //累计上锁量	
		Unlock_cnt=0; //清空其他量
		Calibration_cnt=0;
		
		if(cnt==Lock_cnt) //如果达到阀值
		{
			Lock_cnt=0;   //清空上锁量
			if(State!=REMOTECONTROL_CALIBRATION) //只有当状态不处于校准态时
		    State=REMOTECONTROL_LOCK;	  //标识上锁状态
		    return State;  //返回已经上锁
		}
		else
		  return State;

	}
	else if(StateSelection(UnlockPosture))  //UNLOCK
	{
		Lock_cnt=0;
		Unlock_cnt++;
		Calibration_cnt=0;
		
		if(cnt==Unlock_cnt)
		{
			Unlock_cnt=0;
			if(State!=REMOTECONTROL_CALIBRATION)
			 State= REMOTECONTROL_UNLOCK;
			return State;
		}
		else
			return  State;
	}

	else if(StateSelection(CalibrationPosture))	//CALIBRATION
	{
		Lock_cnt=0;
		Unlock_cnt=0;
		Calibration_cnt++;
		if(cnt==Calibration_cnt)
		{
			Calibration_cnt=0;
			if(State == REMOTECONTROL_LOCK)  //Only locked state calibration(只有上锁状态才能校准)
 			State = REMOTECONTROL_CALIBRATION;
			return State;
		}
		else
			return  State;
	}
	else if( StateSelection(LostPosture) )
	{
		Lock_cnt=0;
		Unlock_cnt=0;
		Calibration_cnt=0;
		if(State!=REMOTECONTROL_CALIBRATION)
		State = REMOTECONTROL_LOSE ;	
		return State;
	}
	else 		     //norm
	{		
		Lock_cnt=0;
		Unlock_cnt=0;
		Calibration_cnt=0;
		State = State | 0x10 ;
		return State ;
	}
}

//Remove the calibration mark
void RemoteControl::EndCalibration()
{
	State = State & 0xfd ; 
}

#ifdef 	USE_THE_RANGE_GIVEN_OUTPUT
uint16_t RemoteControl::operator[](u8 chNum)
{
	if(chNum==1)
		return mPITCH;
	else if(chNum==2)
		return mTHROTTLE;
    else if(chNum==3)
		return mYAW;
	else if(chNum==4)
		return mROLL;
	else if(chNum==5)
		return mAdditional5;
	else if(chNum==6)
		return mAdditional6;
	else if(chNum==7)
		return mAdditional7;
	else if(chNum==8)
		return mAdditional8;
	else
		return 0;
}
	#endif

#ifdef 	USE_PERCENTAGE_OUTPUT
 double RemoteControl::operator[](u8 chNum)
 {
	if(chNum==1)
		return (mPITCH-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;
	else if(chNum==2)
		return (mTHROTTLE-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;
	else if(chNum==3)
		return (mYAW-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;
	else if(chNum==4)
		return (mROLL-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;		
	else if(chNum==5)
		return (mModel-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;
	else if(chNum==6)
		return (mAdditional6-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;
	else if(chNum==7)
		return (mAdditional7-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;
	else if(chNum==8)
		return (mAdditional8-Limit_MIN)*1.0/(Limit_MAX-Limit_MIN)*100.0;
	else
		return 0;
 }
#endif

//Enter the calibration mode(进入校准态)
void RemoteControl::StartCalibration()
{
	static u8 cnt=0;
	u16 ChNumValue[8];
	
		ChNumValue[0]=(*mHunter1)[mPitchCh1]*1000; 
		ChNumValue[1]=(*mHunter1)[mThrottleCh2]*1000; 
		ChNumValue[2]=(*mHunter1)[mYawCh3]*1000; 
		ChNumValue[3]=(*mHunter1)[mRollCh4]*1000;
	
	if(mHunter2)
	{
		ChNumValue[4]=(*mHunter2)[mModelCh5]*1000; 
		ChNumValue[5]=(*mHunter2)[mCh6]*1000; 
		ChNumValue[6]=(*mHunter2)[mCh7]*1000; 
		ChNumValue[7]=(*mHunter2)[mCh8]*1000;
	}
	
	
	State=REMOTECONTROL_CALIBRATION;
	
	
	
	if(cnt==0)//第一次进入时，将所有的范围都置为当前通道的值
	{
	OriginalPitMax= ChNumValue[0];
	OriginalPitMin=  ChNumValue[0];
		
	OriginalThrMax= ChNumValue[1];
	OriginalThrMin= ChNumValue[1];
	
	OriginalYawMax= ChNumValue[2];
	OriginalYawMin= ChNumValue[2];

	OriginalRolMax= ChNumValue[3];
	OriginalRolMin= ChNumValue[3];
		
	if(mHunter2)
	{
	OriginalModelMax=  ChNumValue[4];
	OriginalModelMin=  ChNumValue[4];
		
	OriginalAdd6Max= ChNumValue[5];
	OriginalAdd6Min= ChNumValue[5];
	
	OriginalAdd7Max= ChNumValue[6];
	OriginalAdd7Min= ChNumValue[6];

	OriginalAdd8Max= ChNumValue[7];
	OriginalAdd8Min= ChNumValue[7];
	}	
	
	cnt=1;	
	}
	else{
		if(ChNumValue[0]>=OriginalPitMax)
			OriginalPitMax=ChNumValue[0];
		if(ChNumValue[0]<OriginalPitMin)
			OriginalPitMin=ChNumValue[0];
		
		if(ChNumValue[1]>=OriginalThrMax)
			OriginalThrMax=ChNumValue[1];
		if(ChNumValue[1]<OriginalThrMin)
			OriginalThrMin=ChNumValue[1];	
		
		if(ChNumValue[2]>=OriginalYawMax)
			OriginalYawMax=ChNumValue[2];
		if(ChNumValue[2]<OriginalYawMin)
			OriginalYawMin=ChNumValue[2];	
		
		if(ChNumValue[3]>=OriginalRolMax)
			OriginalRolMax=ChNumValue[3];
		if(ChNumValue[3]<OriginalRolMin)
			OriginalRolMin=ChNumValue[3];					
	}
}

//State operating mode(状态操作方式)
u8 RemoteControl::StateSelection(u8 state)
{

	static u8 ThrottleMinimum,ThrottleHighest;
	static u8 YawMinimum,YawHighest;
	static u8 PitchMinimum,PitchHighest;
	static u8 RollMinimum,RollHighest;
	
	
	ThrottleMinimum=(mTHROTTLE<=(Limit_MIN+Limit_MAX/10));
	ThrottleHighest=(mTHROTTLE>=(Limit_MAX-Limit_MAX/10));

	YawMinimum=(mYAW<=(Limit_MIN+Limit_MAX/10));
	YawHighest=(mYAW>=(Limit_MAX-Limit_MAX/10));

	RollMinimum=(mROLL<=(Limit_MIN+Limit_MAX/10));
	RollHighest=(mROLL>=(Limit_MAX-Limit_MAX/10));

	PitchMinimum=(mPITCH<=(Limit_MIN+Limit_MAX/10));
	PitchHighest=(mPITCH>=(Limit_MAX-Limit_MAX/10));
	
	
	switch (state)
	{
		case 1:	
		{		
			if(ThrottleMinimum && YawHighest && PitchMinimum && RollMinimum)   //内八字 右下+左下
				return 1;
			else
				return 0;
		}
		case 2:
		{
			
			if(ThrottleMinimum && YawMinimum && PitchMinimum && RollHighest) 	//外八字 左下+右下
				return 1;
			else 
				return 0;
		}
		case 3:
		{
			if(ThrottleMinimum && YawMinimum && PitchHighest && RollHighest) 	// 左下+右上
				return 1;
			else 
				return 0;
		}
		case 4:
		{
			if(ThrottleMinimum && YawMinimum && RollMinimum && PitchMinimum && 1)	 // 左下+左下
				return 1;
			else
				return 0;
		}
			
		case 5:
		{
			if(ThrottleMinimum && YawHighest && PitchMinimum && RollHighest )	 // 右下+右下
				return 1;
			else
				return 0;
		}
		case 6:
		{
			if(ThrottleMinimum && YawMinimum )		//  左下+随意
				return 1;
			else
				return 0;
		}
		case 7:
		{
			if(ThrottleMinimum && YawHighest )		//  右下+随意
				return 1;
			else
				return 0;
		}
		case 8:
		{
			if(ThrottleMinimum && PitchHighest)     //下+上
				return 1;
			else
				return 0;
		}
			
		default: return 0;
	}
}

//Return to the original value(返回原始值)
uint16_t RemoteControl::GetOriginalValue(u8 ChNum)
{
	if (ChNum==1)
		return (*mHunter1)[mPitchCh1]*1000;
	else if(ChNum==2)
		return (*mHunter1)[mThrottleCh2]*1000;
	else if(ChNum==3)
		return (*mHunter1)[mYawCh3]*1000;
	else if(ChNum==4)
		return (*mHunter1)[mRollCh4]*1000;
	else if(ChNum==5)
		return (*mHunter2)[mModelCh5]*1000;
	else if(ChNum==6)
		return (*mHunter2)[mCh6]*1000;
	else if(ChNum==7)
		return (*mHunter2)[mCh7]*1000;
	else if(ChNum==8)
		return (*mHunter2)[mCh8]*1000;
	else
		return 0;
}



//Modify the state of judgment(修改状态判断的方式)
void RemoteControl::SetLockPosture(u8 Choose)
{
	LockPosture=Choose;
}
void RemoteControl::SetUnlockPosture(u8 Choose)
{
	UnlockPosture=Choose;
}
void RemoteControl::SetLostPosture(u8 Choose)
{
	LostPosture=Choose;
}
void RemoteControl::SetCalibrationPosture(u8 Choose)
{
	CalibrationPosture=Choose;
}
