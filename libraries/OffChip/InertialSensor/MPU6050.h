#ifndef _MPU6050_H_
#define _MPU6050_H_
#include "stm32f10x.h"
#include "I2C.h"
#include "MathTool.h"
#include "InertialSensor.h"

//MPU6050 IIC Address
#define MPU6050_ADDRESS_AD0_LOW     0x68 //address pin low (GND)
#define MPU6050_ADDRESS_AD0_HIGH    0x69 //address pin high (VCC)
#define MPU6050_DEFAULT_ADDRESS     0xD0 //MPU6050_ADDRESS_AD0_LOW  ((MPU6050_ADDRESS_AD0_LOW<<1)&0xFE) or  ((MPU6050_ADDRESS_AD0_HIGH<<1)&0xFE)
#define MPU6050_ADDRESS   MPU6050_DEFAULT_ADDRESS

//MPU6050 Register Address
#define	SMPLRT_DIV					0x19	//sample rate of gyro. typically: 0x07(125Hz)
#define	CONFIG							0x1A	//low pass filter, typically: 0x06(5Hz)
#define	GYRO_CONFIG		 			0x1B	//gyro range and self test set, typically: 0x18(no selftest,2000deg/s)
#define	ACCEL_CONFIG	 			0x1C	//acc range, self test, high pass filter. typically: 0x01(no self test,2G,5Hz)
#define	ACCEL_XOUT_H	 			0x3B  //acc x hight byte, register address
#define	ACCEL_XOUT_L 				0x3C  //acc x low   byte, register address
#define	ACCEL_YOUT_H				0x3D  //acc y hight byte, register address
#define	ACCEL_YOUT_L				0x3E  //acc y low   byte, register address
#define	ACCEL_ZOUT_H				0x3F  //acc z hight byte, register address
#define	ACCEL_ZOUT_L	 			0x40  //acc z low   byte, register address
#define	TEMP_OUT_H					0x41  //temperature hight byte, register address
#define	TEMP_OUT_L					0x42  //temperature low   byte, register address
#define	GYRO_XOUT_H		 			0x43  //gyro x hight byte, register address
#define	GYRO_XOUT_L		 			0x44	//gyro x low byte,   register address
#define	GYRO_YOUT_H		 			0x45  //gyro y hight byte, register address
#define	GYRO_YOUT_L		 			0x46  //gyro y low   byte, register address
#define	GYRO_ZOUT_H		 			0x47  //gyro z hight byte, register address
#define	GYRO_ZOUT_L					0x48  //gyro z low   byte, register address
#define	PWR_MGMT_1		 			0x6B	//power management, typically: 0x00(work normally)
#define	WHO_AM_I			 			0x75	//self identify, typically: (0x68, read only)
#define I2C_MST_CTRL        0x24  
#define I2C_SLV0_ADDR       0x25  //slave0 device address
#define I2C_SLV0_REG        0x26 	//slave0 register address 
#define I2C_SLV0_CTRL       0x27  //
#define I2C_SLV0_DO         0x63  //data write to slave0
#define USER_CTRL           0x6A  //enable settings: FIFO, I2C master mode, I2C interface
#define INT_PIN_CFG         0x37  
#define EXT_SENS_DATA_00    0x49
#define EXT_SENS_DATA_01    0x4A
#define EXT_SENS_DATA_02    0x4B
#define EXT_SENS_DATA_03    0x4C
#define EXT_SENS_DATA_04    0x4D
#define EXT_SENS_DATA_05    0x4E
#define EXT_SENS_DATA_06    0x4F
#define EXT_SENS_DATA_07    0x50
#define EXT_SENS_DATA_08    0x51
#define EXT_SENS_DATA_09    0x52
#define EXT_SENS_DATA_10    0x53
#define EXT_SENS_DATA_11    0x54

#define AVG_FILTER_LEN  5     //the length of average filter


class MPU6050:public InertialSensor
{
protected:
	I2C &_i2c;           //i2c bus device
	Vector3f _acc_filter[AVG_FILTER_LEN];   //sum of acc data
	Vector3f _gyro_filter[AVG_FILTER_LEN];  //sum of gyro data
	u16 _filter_idx;       //arry index for acc/gyro filter
  u8 _raw_data[15];      //raw data of MPU6050
	bool _health;


public:
	MPU6050(I2C &i2c);
	bool getHealth();
  virtual bool Initialize(void);
	virtual bool Update(Vector3f &acc, Vector3f &gyro);
};


#endif



