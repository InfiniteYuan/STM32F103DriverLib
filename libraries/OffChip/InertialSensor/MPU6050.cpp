#include "MPU6050.h"

//constructor
MPU6050::MPU6050(I2C &i2c):_i2c(i2c)
{
	StartGyroCalibrating();
	Initialize();                      //initialize mpu6050
}

//initialze mpu6050
bool MPU6050::Initialize(void)
{
	mHealthy = false;                                  //set mpu6050 not healthy
	if(!_i2c.WaitFree(50))                             //wait for i2c work and free
		if(!_i2c.Initialize()) return false;             //if time out, reset and initialize i2c
	                      //   7          6           5             4              3              2            1             0
	                      // Reset      Sleep       Cycle          -            temp-dis               clksel[2:0]
	u8 configData[10][2] = {PWR_MGMT_1,     0x03,      //Set Clock to ZGyro, not reset, not sleep, enable temp sensor
		
		                    //INT_LEVEL  INT_OPEN  LATCH_INT_EN  INT_RD_CLEAR  FSYNC_INT_LEVEL  FSYNC_INT_EN  I2C_BYPASS_EN    -
	                       //INT_PIN_CFG,     0x02,      //enable slave i2c bypass mode
		                    
      		              //  XG_ST     YG_ST       ZG_ST            FS_SEL[ 1 : 0 ]              -            -             -
		                     GYRO_CONFIG,     0x18,      //	no selftest, +-250 o/s
		                    
		                    //  XA_ST     YA_ST       ZA_ST            FS_SEL[ 1 : 0 ]              -            -             -
		                     ACCEL_CONFIG,    0x10,      //no selftest, +-2g
		
		                    //    -          -                    EXT_SYNC_SET[ 2 : 0 ]                   DLPF_CFG[ 2 : 0 ]
		                     CONFIG,          0x00,      //no external sync, disable low pass filter = band width 256 hz
		                    
		                    //Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV) 
												SMPLRT_DIV,      0x00,      //sample rate of gyro = 8KHz,  max acc rate = 1KHz
		
		                    //USER_CTRL,       0x07,      //fifo, i2c master, etc.
		
		                    
		
												 I2C_MST_CTRL,    0x0D,      //config the mpu6050  master bus rate as 400kHz
		                     //USER_CTRL,       0x00,      //Disable mpu6050 Master mode
		                     INT_PIN_CFG,     0x02       //Enable bypass mode 
	                      };
	for(u8 i=0; i<7; i++)
  {
		bool isTaskTail = ((i==6) ? true : false);
		//mpu init cmd:    i2c addr      txdata[]       txNum  rxdata[] rxNum
		_i2c.AddCommand(MPU6050_ADDRESS, configData[i],   2,      0,      0,   this, isTaskTail);
	}
	_i2c.Start();  //start to rum i2c command	
	
	if(!_i2c.WaitFree(50)) return false;               //wait mpu6050 initialize complete, if time out, keep nuhealthy state
	mHealthy = true;                                   //initialize success
	
	return true;
}



//update sensor data
bool MPU6050::Update(Vector3f &acc, Vector3f &gyro)
{
	if(!_i2c.IsHealthy())//if i2c not work correctly
	{
		_i2c.Initialize();    //initialize i2c
		Initialize();	        //initialize mpu6050
		_health = false;
		return false;
	}
	if(mIsUpdated==false) //not updated(no new data avalible)
	{
		double curTime = tskmgr.Time();
		if(curTime-mUpdatedTime > 0.05)   //if time out, then reset
		{
			_i2c.Initialize();    //initialize i2c
			Initialize();	        //initialize mpu6050
			mIsUpdated = true;
		}
		_health = false;
		return false;
	}
	mIsUpdated = false;
	u8 reg = ACCEL_XOUT_H;    //form acc x high register, read 14 bytes
	_i2c.AddCommand(MPU6050_ADDRESS, &reg, 1, _raw_data, 14, this, true);
	_i2c.Start();  //start run i2c command
	
	
	if(GyroCalibrating())
	{
		_gyro_calibrate_sum.x += _gyro_raw.x;
		_gyro_calibrate_sum.y += _gyro_raw.y;
		_gyro_calibrate_sum.z += _gyro_raw.z;
		if(++_gyro_calibrate_cnt>=2000)
		{
			StopGyroCalibrating();
			_gyro_offset.x = _gyro_calibrate_sum.x / _gyro_calibrate_cnt;
			_gyro_offset.y = _gyro_calibrate_sum.y / _gyro_calibrate_cnt;
			_gyro_offset.z = _gyro_calibrate_sum.z / _gyro_calibrate_cnt;
		}	
	}	
	//convert sensor data
	
	_acc_raw.x = s16(_raw_data[0]<<8 | _raw_data[1]) - 100;		//OFFSET
	_acc_raw.y = s16(_raw_data[2]<<8 | _raw_data[3]) + 116;
	_acc_raw.z = s16(_raw_data[4]<<8 | _raw_data[5]) + 261;
	_gyro_raw.x = s16(_raw_data[8]<<8 | _raw_data[9]);
	_gyro_raw.y = s16(_raw_data[10]<<8 | _raw_data[11]);
	_gyro_raw.z = s16(_raw_data[12]<<8 | _raw_data[13]);
	
	_gyro_raw -= _gyro_offset;
	
	_acc_g.x   = _acc_raw.x*0.00024414;        //g
	_acc_g.y   = _acc_raw.y*0.00024414;        //g
	_acc_g.z   = _acc_raw.z*0.00024414;        //g
	_gyro_rad.x  = _gyro_raw.x*0.000532632*2;   //rad/s
	_gyro_rad.y  = _gyro_raw.y*0.000532632*2;   //rad/s
	_gyro_rad.z  = _gyro_raw.z*0.000532632*2;   //rad/s
	
	acc(_acc_raw.x, _acc_raw.y, _acc_raw.z);
	gyro = _gyro_rad;
	_health = true;
	return true;
}

bool MPU6050::getHealth(){
	return _health;
}
