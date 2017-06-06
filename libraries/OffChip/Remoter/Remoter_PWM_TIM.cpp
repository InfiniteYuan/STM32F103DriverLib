#include "Remoter_PWM_TIM.h"
#include "math.h"


Remoter_PWM_TIM::Remoter_PWM_TIM(InputCapture_TIM *icpRoll, u8 chRoll,  InputCapture_TIM *icpPitch,    u8 chPitch,
                         InputCapture_TIM *icpYaw,  u8 chYaw,   InputCapture_TIM *icpThrottle, u8 chThrottle, 
												 InputCapture_TIM *icpMode, u8 chMode,  InputCapture_TIM *icpAux1,     u8 chAux1,
                         InputCapture_TIM *icpAux2, u8 chAux2,  InputCapture_TIM *icpAux3,     u8 chAux3,
                         InputCapture_TIM *icpAux4, u8 chAux4,  InputCapture_TIM *icpAux5,     u8 chAux5      )
{
	mICP[0] = icpRoll;        mChNum[0] = chRoll;
	mICP[1] = icpPitch;       mChNum[1] = chPitch;
	mICP[2] = icpYaw;         mChNum[2] = chYaw;
	mICP[3] = icpThrottle;    mChNum[3] = chThrottle;
	mICP[4] = icpMode;        mChNum[4] = chMode;
	mICP[5] = icpAux1;        mChNum[5] = chAux1;
	mICP[6] = icpAux2;        mChNum[6] = chAux2;
	mICP[7] = icpAux3;        mChNum[7] = chAux3;
	mICP[8] = icpAux4;        mChNum[8] = chAux4;
	mICP[9] = icpAux5;        mChNum[9] = chAux5;
}

float Remoter_PWM_TIM::operator[](u8 chNum)
{
	chNum -= 1;
	if(mICP[chNum] && mChNum[chNum])
	{
		float chVal = mRawT[chNum];
		chVal = (chVal-mMinT[chNum])/(mMaxT[chNum]-mMinT[chNum]);
		if(chVal>1.0f) chVal = 1.0f;
		if(chVal<0.0f) chVal = 0.0f;
		return chVal*100;
	}
	return 0;
}

float Remoter_PWM_TIM::Channel(u8 chNum)
{
	chNum -= 1;
	if(mICP[chNum] && mChNum[chNum])
	{
		float chVal = mRawT[chNum];
		chVal = (chVal-mMinT[chNum])/(mMaxT[chNum]-mMinT[chNum]);
		if(chVal>1.0f) chVal = 1.0f;
		if(chVal<0.0f) chVal = 0.0f;
		return chVal*100;
	}
	return 0;
}

float Remoter_PWM_TIM::ChannelRaw(u8 chNum)
{
	return mRawT[--chNum];
}


u8 Remoter_PWM_TIM::Update()
{
	static double newTime = 0, oldTime = 0, interval = 0, lockTime = 0, unlockTime = 0;
	static u8 rcDelayCnt = 0; //when power on, rc is not stable, should delay for a while
	//check if need reset max and min value of all channels
	newTime = tskmgr.Time();    //get current time
	interval = newTime - oldTime;
	if(interval<0.02) {
		return MOD_BUSY;
	}
	
	oldTime = newTime;   //update old time
	
	if(rcDelayCnt<100) rcDelayCnt++;
	
	//update all channel values
	for(u8 i=0; i<10; i++)
	{
		if(mICP[i] && mChNum[i])
		{
			mRawT[i] = (*mICP[i])[mChNum[i]];  //get channel value
			if(mLocked && mCalibrating)
			{
				if(rcDelayCnt>90)
				{
					if(mRawT[i]>mMaxT[i]) mMaxT[i] = mRawT[i];    //update max value
					if(mRawT[i]<mMinT[i]) mMinT[i] = mRawT[i];    //update min value
				}
			}
		}
	}
	
	_health = true;
	if(mLocked)
	{
		if(!mCalibrating && abs(Channel(1)-mUnlockVal[0])<1 && abs(Channel(2)-mUnlockVal[1])<1 && abs(Channel(3)-mUnlockVal[2])<1 && abs(Channel(4)-mUnlockVal[3])<1)
			unlockTime += interval;
		else
			unlockTime = 0;
		if(unlockTime>5)
			mLocked = false;
		lockTime = 0;
		return MOD_READY | MOD_LOCK;
	}
	else
	{
		if(abs(Channel(4)-mLockVal[3])<1)
			lockTime += interval;
		else
			lockTime = 0;
		if(lockTime>4)
			mLocked = true;
		unlockTime = 0;
		return MOD_READY | MOD_UNLOCK;
	}
}

bool Remoter_PWM_TIM::getHealth(){
	return _health;
}

