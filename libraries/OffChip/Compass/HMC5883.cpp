#include "HMC5883.h"
#include "TaskManager.h"


HMC5883::HMC5883(I2C &i2c, u16 maxUpdateFrequency):mI2C(i2c),mMaxUpdateFrequency(maxUpdateFrequency)
{
	Initialize();
//	StartMagCalibrating();
	mOffsetRatio(1.285,1.285,1);
	mOffsetBias(236,309,114);
}

bool HMC5883::Initialize()
{
	mHealthy = false;                                  //set HMC5883 not healthy
	if(!mI2C.WaitFree(50))                             //wait for i2c work and free
		if(!mI2C.Initialize()) return false;             //if time out, reset and initialize i2c
	
	u8 configData[3][2] = {HMC5883_Config_RA, HMC5883L_AVERAGING_8|HMC5883L_RATE_75|HMC5883L_BIAS_NORMAL, //Config Register A  :number of samples averaged->8  Data Output rate->30Hz
	                       HMC5883_Config_RB, HMC5883L_GAIN_1090,                                         //Config Register B:  Gain Configuration as : Sensor Field Range->(+-)1.3Ga ; Gain->1090LSB/Gauss; Output Range->0xF800-0x07ff(-2048~2047)
		                     HMC5883_Mode,      HMC5883L_MODE_CONTINUOUS                                    //Config Mode as: Continous Measurement Mode
	                      };
	for(u8 i=0; i<3; i++)
  {
		bool isTaskTail = ((i==2) ? true : false);
		//mpu init cmd:    i2c addr      txdata[]       txNum  rxdata[] rxNum
		mI2C.AddCommand(HMC5883_ADDRESS, configData[i],   2,      0,      0,   this, isTaskTail);
	}
	mI2C.Start();  //start to rum i2c command	
	if(!mI2C.WaitFree(50)) return false;               //wait HMC5883 initialize complete, if time out, keep nuhealthy state
	mHealthy = true;                                   //initialize success
	return true;
}

bool HMC5883::Update(Vector3f &mag)
{
	if(!mI2C.IsHealthy())//if i2c not work correctly
	{
		mI2C.Initialize();    //initialize i2c
		Initialize();	        //initialize mpu6050
		_health = false;
		return false;
	}
	if(mIsUpdated==false) 
	{
		
		if(tskmgr.Time()-mUpdatedTime > 1)
		{
			mI2C.Initialize();    //initialize i2c
			Initialize();	        //initialize HMC5883
			mIsUpdated = true;
		}
		return false;
	}
	mIsUpdated = false;
	u8 reg = HMC5883_XOUT_M;    //form mag x high register, read 6 bytes mag_Status
	mI2C.AddCommand(HMC5883_ADDRESS, &reg, 1, &(mData.mag_XH), 6, this, true);
	reg = HMC5883_Status;    		//form mag HMC5883_Status
	mI2C.AddCommand(HMC5883_ADDRESS, &reg, 1, &(mData.mag_Status), 1, this, true);
	
	if(IsLocked()){
		u8 configData[1][2] = {HMC5883_Mode, HMC5883L_MODE_CONTINUOUS};
		mI2C.AddCommand(HMC5883_ADDRESS, configData[0], 2, 0, 0, this, false);//Config Mode as: Continous Measurement Mode
	}
	
	mI2C.Start();  //start run i2c command
	
	if(MagCalibrating()){
		static int limitValue[6]={0,0,0,0,0,0};//三个轴的极限值（最小最大值）
		static bool firstTime=true;
		static float lastUpdateLimitValueTime = 0;//记录最近更新最大最小值的时间
		int magRaw[3];
		magRaw[0] = GetNoCalibrateDataRaw().x;
		magRaw[1] = GetNoCalibrateDataRaw().y;
		magRaw[2] = GetNoCalibrateDataRaw().z;//原始值
		if(firstTime)//刚开始校准,给初值
		{
			firstTime = false;
			lastUpdateLimitValueTime = TaskManager::Time();
			for(u8 i=0;i<3;++i)
			{
				limitValue[2*i] = magRaw[i];
				limitValue[2*i+1] = magRaw[i];
			}
		}
		else
		{
			for(u8 i=0;i<3;++i)
			{
				if(magRaw[i]<limitValue[2*i])
				{
					limitValue[2*i] = magRaw[i];
					lastUpdateLimitValueTime = TaskManager::Time();
				}
				else if(magRaw[i]>limitValue[2*i+1])
				{
					limitValue[2*i+1] = magRaw[i];
					lastUpdateLimitValueTime = TaskManager::Time();
				}
			}
			//三个轴的区间
			xMaxMinusMin = limitValue[1] -limitValue[0];
			yMaxMinusMin = limitValue[3] -limitValue[2];
			zMaxMinusMin = limitValue[5] -limitValue[4];
			if(TaskManager::Time() - lastUpdateLimitValueTime > 10)//10秒钟保持极限值不变，停止校准
			{
				if(xMaxMinusMin>yMaxMinusMin && xMaxMinusMin>zMaxMinusMin)
				{
					mOffsetRatio.x = 1.0;
					mOffsetRatio.y = 1.0*xMaxMinusMin/yMaxMinusMin;
					mOffsetRatio.z = 1.0*xMaxMinusMin/zMaxMinusMin;
				}
				else if(yMaxMinusMin>xMaxMinusMin && yMaxMinusMin>zMaxMinusMin)
				{
					mOffsetRatio.x = 1.0*yMaxMinusMin/xMaxMinusMin;
					mOffsetRatio.y = 1.0;
					mOffsetRatio.z = 1.0*yMaxMinusMin/zMaxMinusMin;
				}
				else if(zMaxMinusMin>xMaxMinusMin && zMaxMinusMin>yMaxMinusMin)
				{
					mOffsetRatio.x = 1.0*xMaxMinusMin/yMaxMinusMin;
					mOffsetRatio.y = 1.0*xMaxMinusMin/yMaxMinusMin;
					mOffsetRatio.z = 1.0;
				}
				mOffsetBias.x = mOffsetRatio.x*(xMaxMinusMin*0.5f - limitValue[1]);
				mOffsetBias.y = mOffsetRatio.y*(yMaxMinusMin*0.5f - limitValue[3]);
				mOffsetBias.z = mOffsetRatio.z*(zMaxMinusMin*0.5f - limitValue[5]);
				//检查校准值有效性
				{
					//未写
				}
				//标志复位
				firstTime = true;
				StopMagCalibrating();
			}
		}
	}
	
	//convert sensor data				//x,z,y
	mag.x= mOffsetRatio.x *(((signed short int)(mData.mag_XH<<8)) | mData.mag_XL) +mOffsetBias.x;
	mag.y= mOffsetRatio.y *(((signed short int)(mData.mag_YH<<8)) | mData.mag_YL) +mOffsetBias.y;
	mag.z= mOffsetRatio.z *(((signed short int)(mData.mag_ZH<<8)) | mData.mag_ZL) +mOffsetBias.z;

	_health = true;
	return true;
}


Vector3<int> HMC5883::GetNoCalibrateDataRaw(){
	Vector3<int> temp;
	temp.x= (((signed short int)(mData.mag_XH<<8)) | mData.mag_XL) ;
	temp.y= (((signed short int)(mData.mag_YH<<8)) | mData.mag_YL) ;
	temp.z= (((signed short int)(mData.mag_ZH<<8)) | mData.mag_ZL) ;
	return temp;
}

bool HMC5883::IsLocked(){
	return mData.mag_Status&0x02 ? true : false;
}
void HMC5883::SetMaxUpdateFrequency(u16 maxUpdateFrequency)
{
	mMaxUpdateFrequency=maxUpdateFrequency;
}

u16 HMC5883::GetMaxUpdateFrequency()
{
	return mMaxUpdateFrequency;
}

bool HMC5883::getHealth(){
	return _health;
}
