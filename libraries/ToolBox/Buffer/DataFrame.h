#ifndef _DATA_FRAME_H_
#define _DATA_FRAME_H_

#include "stm32f10x.h"

#define MAX_FRAME_DATA_LENGTH 50
#define FRAME_HEADER 0xAA

//frame struct:
// | header | fnCode |  dataLength | data1 | ...... | datan | checkSum | 

struct DataFrame
{
public:
	const u8 header;
	u8 fnCode;
	u8 dataLength;
	u8 data[MAX_FRAME_DATA_LENGTH];
	bool isUpdated;
	u8 checkSum;

public:
	DataFrame():header(FRAME_HEADER)                //constructor
	{
		isUpdated = false;
	}
	
	bool VerifyCheckCode()    
	{
		u8 code = header + fnCode + dataLength;
		for(u8 i=0; i<dataLength; i++)
			code += data[i];
		if(code == checkSum)
			return true;
		else
			return false;
	}

	
	
	void CreateCheckCode()
	{
		checkSum = header + fnCode + dataLength;
		for(u8 i=0; i<dataLength; i++)
			checkSum += data[i];
	}
	DataFrame& operator=(const DataFrame &df)
	{
		//header = df.header;
		fnCode = df.fnCode;
		dataLength = df.dataLength;
		isUpdated = df.isUpdated;
		checkSum = df.checkSum;
		for(u8 i=0; i<dataLength; i++)
			data[i] = df.data[i];
		return *this;
	}
};

//function code  user defined name    value        notice
#define 					DIRECTION_SEND 			0
#define 					DIRECTION_RECV  		1
#define          	FC_DEV_STA   				0x00    //function code 00: motor setting velocity
#define          	FC_IMU_RAW   	  		0x01    //function code 01: motor current velocity
#define          	FC_GPS_RAW	  			0x02    //function code 02: get Weighing machine value
#define          	FC_RC_RAW					  0x03    //function code 03: get Voltage value
#define          	FC_CIRCLE_CONTROL		0x04    //function code 04: get Current value
#define          	FC_IAP_UPDATE  			0x05    //function code 05: set pid parameters for pitch
#define          	FC_IAP_INFO  				0x06		//function code 06: set pid parameters for yaw
#define          	FC_SET_PIT_PID  		0x07    //function code 05: set pid parameters for pitch
#define          	FC_SET_YAW_PID  		0x08
#define          	FC_SET_ROL_PID  		0x09		//function code 06: set pid parameters for roll
#define          	FC_SET_RATE_PIT_PID  		0x0A    //function code 05: set pid parameters for pitch
#define          	FC_SET_RATE_YAW_PID  		0x0B		//function code 06: set pid parameters for yaw
#define          	FC_SET_RATE_ROL_PID  		0x0C		//function code 06: set pid parameters for roll
//#define          	FC_GET_ROL_PID  		0x08		//function code 07: get roll pid parameters
//#define          	FC_GET_PIT_PID  		0x09		//function code 08: get pitch pid parameters
//#define 					FC_GET_YAW_PID  		0x0A		//function code 09: get yaw pid parameters
//#define 					FC_BAC_ROL_PID  		0x0B		//function code 10: feed back roll p,i,d 
//#define 					FC_BAC_PIT_PID  		0x0C		//function code 11: feed back pitch p, i, d
//#define 					FC_BAC_YAW_PID  		0x0D		//function code 12: feed back yaw p , i, d
//#define           FC_RC_GET_POS       0x0E	  //function code 13: PC to MCU, get rc position
//#define           FC_RC_BAC_POS       0x0F 	  //function code 14: MCU to PC, return rc position,16 byte,  roll, pitch, yaw, mode
//#define           FC_GET_TAG_ANG      0x10    //function code 15: PC to MCU, get target angle,
//#define           FC_BAC_TAG_ANG      0x11    //function code 16: MCU to PC, back target angle,12 bytes, roll, pitch, yaw
//#define           FC_GET_IMU_ANG      0x12    //function code 17: PC to MCU, get imu angle
#define 	MAX_FN_CODE 	0x0C

const u8 DATA_LENGTH[MAX_FN_CODE+1][2] = 
{
	3,	0,
	36,	0,
	41, 0,
	16,	0,
	41,	1,
	1,  50,
	0,  3,
	0,  12,
	0,  12,
	0,  12,
	0,  12,
	0,  12,
	0,  12
//	4,			//data length of function code 00: motor setting velocity, 4 bytes, 0.0 ~ 100.0
//	4*4,			//data length of function code 01: motor current velocity  4 bytes, rotate / second
//	4,			//data length of function code 02: get Weighing machine value  >8000000
//	4,			//data length of function code 03: motor enable,  0x00-disable,   0xFF-enable
//	4,			//data length of function code 04: set roll pid parameters: p, i, d, power
//	16,			//data length of function code 05: set pitch pid parameters: p, i, d, power
//	16,			//data length of function code 06: set yaw pid parameters: p, i, d, power
//	0,			//data length of function code 07: get roll p, i, d, power
//	0,			//data length of function code 08: get pitch p,i, d, power
//	0,			//data length of function code 09: get yaw p, i, d, power
//	16,			//data length of function code 10: return back roll p, i ,d, power
//	16, 		//data length of function code 11: return back pitch p, i, d, power
//	16, 		//data length of function code 12: return back yaw p, i, d, power
//	0,      //data length of function code 13: PC to MCU, get rc position
//  16,     //data length of function code 14: MCU to PC, return rc position,16 byte, roll, pitch, yaw, mode
//  0,      //data length of function code 15: PC to MCU, get target angle,
//  12,     //data length of function code 16: MCU to PC, back target angle,12 bytes, roll, pitch, yaw
//  0       //data length of function code 17: PC to MCU, get imu angle
};





#endif

