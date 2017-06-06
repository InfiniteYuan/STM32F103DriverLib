#ifndef _REMOTER_H_
#define _REMOTER_H_

#include "stm32f10x.h"
#include "TaskManager.h"

class Remoter
{
protected:
	float mRawT[10];
	float mMaxT[10];  //the max duration(ms) of channel x
  float mMinT[10];  //the min duration(ms) of channel x
  bool mLocked;     //true: remoter is locked, false: unlocked
  bool mCalibrating;//true: currently calibrate rc, false: not calibrate rc
	float mLockVal[4];
  float mUnlockVal[4];
	u8   mRCState;
public:
	Remoter()        //Constructor
	{
		for(u8 i=0;i<10;i++)
		{
			mMaxT[i] = 1.9f;
			mMinT[i] = 1.1f;
		}
		mLocked = true;
		mCalibrating = false;
		mRCState = MOD_ERROR;
		mLockVal[0] = 0;
		mLockVal[1] = 0;
		mLockVal[2] = 0;
		mLockVal[3] = 0;
		mUnlockVal[0] = 100;
		mUnlockVal[1] = 0;
		mUnlockVal[2] = 0;
		mUnlockVal[3] = 0;
	}
	virtual float operator[](u8 chNum) = 0; //return the percentage of channel x  (0.0 ~ 100.0), the same as Channel
	virtual float Channel(u8 chNum) = 0;    //return the percentage of channel x  (0.0 ~ 100.0), the same as operator[]
	virtual float ChannelRaw(u8 chNum) = 0; //return the raw value of channel x (ms)
	virtual u8 Update() = 0;
	bool IsLocked() { return mLocked;}      //return the locked status of remoter
	void SetLockState(float roll, float pitch, float yaw, float throttle)
	{
		mLockVal[0] = roll;
		mLockVal[1] = pitch;
		mLockVal[2] = yaw;
		mLockVal[3] = throttle;
	}
	void SetUnlockState(float roll, float pitch, float yaw, float throttle)
	{
		mUnlockVal[0] = roll;
		mUnlockVal[1] = pitch;
		mUnlockVal[2] = yaw;
		mUnlockVal[3] = throttle;
	}
	void StartCalibrate()
	{
		if(Ready() && Locked()) 
		{
			mRCState = (MOD_READY | MOD_ADJUST);
			for(u8 i=0;i<10;i++)
			{
				mMaxT[i] = 1.5f;
				mMinT[i] = 1.5f;
			}
		}
	}
	void StopCalibrate()
	{
		if(Ready() && Calibrating())
		{
			mRCState = (MOD_READY | MOD_LOCK);
		}
	}
	bool Ready()
	{
		if(mRCState & MOD_READY) 
			return true;
		else
			return false;
	}
	bool UnLocked()
	{
		if(mRCState & MOD_UNLOCK)
			return true;
		else 
			return false;
	}
	bool Locked()
	{
		if(mRCState & MOD_LOCK)
			return true;
		else 
			return false;
	}
	bool Calibrating()
	{
		if(mRCState & MOD_ADJUST)
			return true;
		else 
			return false;
	}
	float Roll()     { return Channel(1); }
	float Pitch()    { return Channel(2); }
	float Yaw()      { return Channel(3); }
	float Throttle() { return Channel(4); }
	float Mode()     { return Channel(5); }
};


#endif



