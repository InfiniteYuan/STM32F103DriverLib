#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "stm32f10x.h"
#include "USART.h"
#include "CRC.h"
#include "Vector3.h"
#include "NRF24L01.h"


#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)		) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )


class Communication{

private:
	NRF24L01 &nrf;
	bool Calibration(u8 *data, int lenth, u8 check);

public:

	//接收
	u16 mRcvTargetYaw;
	u16 mRcvTargetRoll;
	u16 mRcvTargetPitch;
	u16 mRcvTargetThr;
	u16 mRcvTargetHight;

	bool mClockState; //1为锁定，0为解锁

	bool mAcc_Calibrate;
	bool mGyro_Calibrate;
	bool mMag_Calibrate;

	bool mPidUpdata; //为true时表示有新的PID发来需要更新PID
	bool mGetPid;    //为true时表示上位机需要获取当前PID的值

	Communication(NRF24L01 &nrf);
	bool DataListening();//数据接收监听

	float PID[9]; //暂存接收到的PID值

	//	//上锁与解锁
	//	 bool FlightLockControl(bool flag);


	//发送
	//状态数据
	bool SendCopterState(float angle_rol, float angle_pit, float angle_yaw, s32 Hight, u8 fly_model, u8 armed);
	//传感器原始数据
	bool SendSensorOriginalData(Vector3<int> acc, Vector3<int> gyro, Vector3<int> mag);
	//接收到的控制量
	bool SendRcvControlQuantity();
	//发送PID数据
	bool SendPID(float p1_p, float p1_i, float p1_d, float p2_p, float p2_i, float p2_d, float p3_p, float p3_i, float p3_d);
	//接收应答 需要应答的功能字和校验和
	bool reply(u8 difference, u8 sum);
	//发送电机信息
	void SendMotoMsg(u16 m_1, u16 m_2, u16 m_3, u16 m_4, u16 m_5, u16 m_6, u16 m_7, u16 m_8);
	//test 使用原始数据的通信方式来发送测试信息
	void test(float a, float b, float c, float d, float e, float f, float g, float h, float i);

};



#endif
