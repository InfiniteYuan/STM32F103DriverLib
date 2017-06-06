#ifndef _HMC5883_H_
#define _HMC5883_H_

#include "stm32f10x.h"
#include "Compass.h"
#include "I2C.h"
#include "MathTool.h"

typedef struct
{
	unsigned char config_RA;        //Configuration register A
	unsigned char config_RB;        //Configuration register B
	unsigned char mode;             //Register Mode
	unsigned char mag_XH;           //Raw x axis data high byte
	unsigned char mag_XL;           //Raw x axis data low byte
	unsigned char mag_ZH;           //Raw z axis data high byte
	unsigned char mag_ZL;           //Raw z axis data high byte
	unsigned char mag_YH;           //Raw y axis data high byte
	unsigned char mag_YL;           //Raw y axis data high byte
	unsigned char mag_Status;       //Register status 
	unsigned char identification_A; //Register identification a  fixed value:0x48 if read correctly
	unsigned char identification_B; //Register identification b  fixed value:0x34 if read correctly
	unsigned char identification_C; //Register identification c  fixed balue:0x33 if read correctly
}HMC5883DataTypeDef;


class HMC5883: public Compass
{
	protected:
		I2C &mI2C;
		HMC5883DataTypeDef mData;
		u16 mMaxUpdateFrequency;
		bool _health;
	public:
		
		bool getHealth();
		
		HMC5883(I2C &i2c, u16 maxUpdateFrequency=75);	
		virtual bool Initialize();
		virtual bool Update(Vector3f &mag);
			
		///////////////////////
		///Get status that if the data register is locked 
		///////////////////////
		bool IsLocked();


		Vector3<int> GetNoCalibrateDataRaw();
		/////////////////////////////
		///Set the max interval of update(Hz)
		///@param maxUpdateFrequency
		//////////////////////////
		void SetMaxUpdateFrequency(u16 maxUpdateFrequency);
		
		/////////////////////////////
		///Get the max interval of update(Hz)
		///@retval maxUpdateFrequency
		//////////////////////////
		u16 GetMaxUpdateFrequency();

		int xMaxMinusMin;
		int yMaxMinusMin;
		int zMaxMinusMin;
};

//---------------HMC5883L Register Address ---------------------------
#define HMC5883_ADDRESS            0x3C  //       HMC     7-bit address:  0x1E    ADRESS+WRITE->0X3C  ADRESS+READ->0X3D
#define HMC5883_Config_RA          0x00  //Configuration Register A           R/W
#define HMC5883_Config_RB          0x01  //Configuration register B           R/W
#define HMC5883_Mode               0x02  //Configuration register Mode        R/W
#define HMC5883_XOUT_M             0x03  //Data Output X MSB Register   Only Read
#define HMC5883_XOUT_L             0x04  //Data Output X LSB Register   Only Read
#define HMC5883_ZOUT_M             0x05  //Data Output Z MSB Register   Only Read
#define HMC5883_ZOUT_L             0x06  //Data Output Z LSB Register   Only Read
#define HMC5883_YOUT_M             0x07  //Data Output Y MSB Register   Only Read
#define HMC5883_YOUT_L             0x08  //Data Output Y LSB Register   Only Read
#define HMC5883_Status             0x09  //Register Status              Only Read
#define HMC5883_Identification_A   0x0A  //Identification Register A    Only Read   fixed value:0x48
#define HMC5883_Identification_B   0x0B  //Identification Register A    Only Read   fixed value:0x34
#define HMC5883_Identification_C   0x0C  //Identification Register A    Only Read   fixed value:0x33

//*****************configuration Register A**************************//
//Configuration Register A bit7
	//Fixed value: 0. This bit must be cleared for correct operation.
//Configuration Register A bit6~bit5
	//Select number of samples averaged (1 to 8) per measurement output.
#define HMC5883L_AVERAGING_MASK     0x60
#define HMC5883L_AVERAGING_1        0x00
#define HMC5883L_AVERAGING_2        0x20
#define HMC5883L_AVERAGING_4        0x40
#define HMC5883L_AVERAGING_8        0x60

//Configuration Register A bit4~bit2
	//Data Output Rate(Hz) Bits. These bits set the rate at which data
	//is written to all three data output registers. 
#define HMC5883L_RATE_MASK          0x1C
#define HMC5883L_RATE_0P75          0x00
#define HMC5883L_RATE_1P5           0x04
#define HMC5883L_RATE_3             0x08
#define HMC5883L_RATE_7P5           0x0c
#define HMC5883L_RATE_15            0x10 //(default value when power on)
#define HMC5883L_RATE_30            0x14
#define HMC5883L_RATE_75            0x18
#define HMC5883L_RATE_NOT_USED      0x1c

//Configuration Register A bit1~bit0
	//Measurement Configuration Bits. These bits define the
	//measurement flow of the device, specifically whether or not
	//to incorporate an applied bias into the measurement.
#define HMC5883L_BIAS_MASK          0x03
#define HMC5883L_BIAS_NORMAL        0x00
#define HMC5883L_BIAS_POSITIVE      0x01
#define HMC5883L_BIAS_NEGATIVE      0x02


//*****************configuration Register B**************************//
//Configuration Register B bit7~bit5
	//Gain Configuration Bits. These bits configure the gain for
	//the device. The gain configuration is common for all
	//channels.
#define HMC5883L_GAIN_MASK          0xe0
#define HMC5883L_GAIN_1370          0x00
#define HMC5883L_GAIN_1090          0x20 //(default value when power on)
#define HMC5883L_GAIN_820           0x40
#define HMC5883L_GAIN_660           0x60
#define HMC5883L_GAIN_440           0x80
#define HMC5883L_GAIN_390           0xA0
#define HMC5883L_GAIN_330           0xc0
#define HMC5883L_GAIN_230           0xe0
//Configuration Register B bit4~bit0
	//These bits must be cleared for correct operation.
	
//*****************configuration Register Mode**************************//	
//Configuration Register Mode bit7~bit2
	//These bits must be cleared for correct operation.
	//! Bit MR7 bit is set internally after each single-measurement operation.
	
//Configuration Register Mode bit1~bit0
#define HMC5883L_MODE_MASK          0x03
#define HMC5883L_MODE_CONTINUOUS    0x00
#define HMC5883L_MODE_SINGLE        0x01
#define HMC5883L_MODE_IDLE          0x02

//*****************configuration Register Status**************************//
//Configuration Register Status bit7~bit2
	//These bits are reserved. SR4 may be 0 or 1 depending on
	//internal activity. Disregard activity on this bit
//Configuration Register Status bit1
#define HMC5883L_STATUS_LOCK_MASK   0x02
#define HMC5883L_STATUS_LOCK_BIT    0x02
//Configuration Register Status bit0
#define HMC5883L_STATUS_READY_MASK  0x01
#define HMC5883L_STATUS_READY_BIT   0x00

#endif



