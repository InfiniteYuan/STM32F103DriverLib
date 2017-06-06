#include "Remoter_PWM_EXIT.h"
#include "math.h"


Remoter_PWM_EXIT::Remoter_PWM_EXIT(InputCapture_EXIT *icpRoll,
                                   InputCapture_EXIT *icpPitch,     //pitch ICP 
                                   InputCapture_EXIT *icpYaw,       //yaw ICP 
                                   InputCapture_EXIT *icpThrottle,  //throttle 
                                   InputCapture_EXIT *icpMode,      //Mode ICP 
                                   InputCapture_EXIT *icpAux1,      //Auxiliary 1 ICP 
                                   InputCapture_EXIT *icpAux2,      //Auxiliary 2 ICP 
                                   InputCapture_EXIT *icpAux3,      //Auxiliary 3 ICP 
                                   InputCapture_EXIT *icpAux4,      //Auxiliary 4 ICP 
                                   InputCapture_EXIT *icpAux5       //Auxiliary 5 ICP 
                                  )
{
	mICP[0] = icpRoll;
	mICP[1] = icpPitch;
	mICP[2] = icpYaw;   
	mICP[3] = icpThrottle;
	mICP[4] = icpMode;  
	mICP[5] = icpAux1;  
	mICP[6] = icpAux2; 
	mICP[7] = icpAux3;  
	mICP[8] = icpAux4;  
	mICP[9] = icpAux5; 
}

float Remoter_PWM_EXIT::operator[](u8 chNum)
{
	chNum -= 1;
	if(mICP[chNum])
	{
		double chVal = mRawT[chNum];
		chVal = (chVal-mMinT[chNum])/(mMaxT[chNum]-mMinT[chNum]);
		if(chVal>1.0) chVal = 1.0;
		if(chVal<0.0) chVal = 0.0;
		return chVal*100;
	}
	return 0;
}

float Remoter_PWM_EXIT::Channel(u8 chNum)
{
	chNum -= 1;
	if(mICP[chNum])
	{
		float chVal = mRawT[chNum];
		chVal = (chVal-mMinT[chNum])/(mMaxT[chNum]-mMinT[chNum]);
		if(chVal>1.0f) chVal = 1.0f;
		if(chVal<0.0f) chVal = 0.0f;
		return chVal*100;
	}
	return 0;
}

float Remoter_PWM_EXIT::ChannelRaw(u8 chNum)
{
	return mRawT[--chNum];
}


u8 Remoter_PWM_EXIT::Update()
{
	static double newTime = 0, oldTime = 0, interval = 0, lockTime = 0, unlockTime = 0;
	static u8 rcDelayCnt = 0; //when power on, rc is not stable, should delay for a while
	//check if need reset max and min value of all channels
	newTime = tskmgr.Time();    //get current time
	interval = newTime - oldTime;
	if(interval<0.02)
	{
		return MOD_BUSY;
	}
	
	oldTime = newTime;   //update old time
	
	if(rcDelayCnt<100) rcDelayCnt++;
	
	//update all channel values
	for(u8 i=0; i<10; i++)
	{
		if(mICP[i])
		{
			mRawT[i] = mICP[i]->Value();  //get channel value
			
			
			if(mRawT[i]<0.9f || mRawT[i]>2.1f) //wrong channel data
			{
				mRCState = MOD_ERROR;            //error
				return MOD_ERROR;
			}					
			
			if(Ready() && Calibrating())  //curent calibrating
			{
				if(rcDelayCnt>90)
				{
					if(mRawT[i]>mMaxT[i]) mMaxT[i] = mRawT[i];    //update max value
					if(mRawT[i]<mMinT[i]) mMinT[i] = mRawT[i];    //update min value
				}
			}
		}
	}
	
	if(!Ready())		mRCState = (MOD_READY | MOD_LOCK);  //from error to  ready and lock
	
	if(Ready() && Locked()) // current locked, check unlock event
	{
		if(!mCalibrating && abs(Channel(1)-mUnlockVal[0])<1 && abs(Channel(2)-mUnlockVal[1])<1 && abs(Channel(3)-mUnlockVal[2])<1 && abs(Channel(4)-mUnlockVal[3])<1)
			unlockTime += interval;
		else
			unlockTime = 0;
		if(unlockTime>5)
			mRCState = (MOD_READY | MOD_UNLOCK);
		lockTime = 0;
	}
	else if(Ready() && UnLocked()) //current unlocked, check lock event
	{
		if(abs(Channel(4)-mLockVal[3])<1)
			lockTime += interval;
		else
			lockTime = 0;
		if(lockTime>4)
			mRCState = (MOD_READY | MOD_UNLOCK);
		unlockTime = 0;
	}
	return mRCState;
}

