#include "communication.h"

Communication::Communication(NRF24L01 &nrf) :nrf(nrf){
	mRcvTargetYaw = 0;
	mRcvTargetRoll = 0;
	mRcvTargetThr = 0;
	mRcvTargetPitch = 0;
	mRcvTargetHight = 0;
	mClockState = true; //1为锁定，0为解锁

	mAcc_Calibrate = false;
	mGyro_Calibrate = false;
	mMag_Calibrate = false;
	mPidUpdata = false;
	mGetPid = false;
}

bool Communication::Calibration(u8 *data, int lenth, u8 check){
	u8 num = 0;
	for (int i = 0; i < lenth; i++){
		num += data[i];
	}
	if (num == check)
		return true;
	else
		return false;
}

bool Communication::DataListening(){
	u8 ch = 0;
	u8 data[32] = { 0 };
	u8 check = 0;
	u8 num = nrf.ReceiveBufferSize();
	if (num > 0){
		nrf.GetReceivedData(&ch, 1);			//取第一个字节
		if (ch == 0xAA){
			nrf.GetReceivedData(&ch, 1);		//取第二个字节
			data[0] = 0xAA;
			if (ch == 0xAA){//帧头判断完成
				data[1] = 0xAA;
				nrf.GetReceivedData(&ch, 1);	//取第三个字节
				if(ch == 0x00){//console
					data[2] = 0x00;
					while (nrf.ReceiveBufferSize() < 3);//等待数据
					nrf.GetReceivedData(data + 3, 3);//读取剩余所有数据
					check = data[5];
					if(Calibration(data, data[3] + 4, check)){
						if (data[4] == 0xa0){//飞机锁定
							mClockState = true;
							reply(data[2], check);
							return true;
						}
						else if (data[4] == 0xa1){//飞机解锁
							mClockState = false;
							reply(data[2], check);
							return true;
						}
					}
				}
				else if (ch == 0x01){//命令集1
					data[2] = 0x01;
					while (nrf.ReceiveBufferSize() < 3);//等待数据
					nrf.GetReceivedData(data + 3, 3);
					check = data[5];
					if (Calibration(data, data[3] + 4, check)) {//如果校验正确
						if (data[4] == 0x01){ 		//ACC校准
							mAcc_Calibrate = true;
							return true;
						}
						else if (data[4] == 0x02){//GYRO校准
							mGyro_Calibrate = true;
							return true;
						}
						else if (data[4] == 0x04){//MAG校准
							mMag_Calibrate = true;
							return true;
						}
						else{											//未知命令
							nrf.ClearReceiveBuffer();
							return false;
						}
					}
					else
						return false; //校准错误
				}
				/*else if (ch == 0x02){//命令集2
					data[2] = ch;
					while (nrf.ReceiveBufferSize() < 3);//等待数据
					nrf.GetReceivedData(data + 3, 3);
					check = data[5];
					if (Calibration(data, data[3] + 4, check)){//如果校验正确
						if (data[4] == 0x01) { //PID请求
							mGetPid = true;
							return true;
						}
						else { //未知命令
							return false;
						}
					}
					else
						return false;
				}*/
				else if (ch == 0x03){//控制信息 commander
					data[2] = ch;
					while (nrf.ReceiveBufferSize() < 12);//等待数据
					nrf.GetReceivedData(data + 3, 12);
					check = data[14];			//和校验值
					if (Calibration(data, data[3] + 4, check)) {//如果校验正确
						mRcvTargetRoll = (u16)data[4] * 256 + data[5];
						mRcvTargetPitch = (u16)data[6] * 256 + data[7];
						mRcvTargetYaw = (u16)data[8] * 256 + data[9];
						mRcvTargetHight = (u16)data[10] * 256 + data[11];
						mRcvTargetThr = (u16)data[12] * 256 + data[13];
						nrf.ClearReceiveBuffer();
						return true;
					}
					return false;
				}
				/*else if (ch == 0x10) {//PID更新
					data[2] = ch;
					while (nrf.ReceiveBufferSize() < 20);//等待数据
					nrf.GetReceivedData(data + 3, 20);
					check = data[22];
					if (Calibration(data, data[3] + 4, check)) {//如果校验正确
						PID[0] = ((u16)data[4] * 256 + data[5]) / 1000.0;
						PID[1] = ((u16)data[6] * 256 + data[7]) / 1000.0;
						PID[2] = ((u16)data[8] * 256 + data[9]) / 1000.0;
						PID[3] = ((u16)data[10] * 256 + data[11]) / 1000.0;
						PID[4] = ((u16)data[12] * 256 + data[13]) / 1000.0;
						PID[5] = ((u16)data[14] * 256 + data[15]) / 1000.0;
						PID[6] = ((u16)data[16] * 256 + data[17]) / 1000.0;
						PID[7] = ((u16)data[18] * 256 + data[19]) / 1000.0;
						PID[8] = ((u16)data[20] * 256 + data[21]) / 1000.0;
						reply(ch, check);
						mPidUpdata = true;
						return true;
					}
					else
						return false;
				}*/
				else {//未知功能字
					nrf.ClearReceiveBuffer();
					return false;
				}
			}
			else {
				nrf.ClearReceiveBuffer();
				return false;		//不是帧头	
			}
		}
		else{
			nrf.ClearReceiveBuffer();
			return false;		//不是帧头	
		}
	}
	else{
		nrf.ClearReceiveBuffer();
		return false;//没接收到数据
	}
	return false;//没接收到数据
}

bool Communication::SendCopterState(float angle_rol, float angle_pit, float angle_yaw, s32 Hight, u8 fly_model, u8 armed){
	u8 _cnt = 0;
	vs16 _temp;
	vs32 _temp2 = Hight;
	u8 data_to_send[30];

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x01;
	data_to_send[_cnt++] = 0;

	_temp = (int)(angle_rol * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = (int)(angle_pit * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = (int)(angle_yaw * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	data_to_send[_cnt++] = BYTE3(_temp2);
	data_to_send[_cnt++] = BYTE2(_temp2);
	data_to_send[_cnt++] = BYTE1(_temp2);
	data_to_send[_cnt++] = BYTE0(_temp2);

	data_to_send[_cnt++] = fly_model;

	data_to_send[_cnt++] = armed;

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;

	nrf.SendData(data_to_send, _cnt);
	return true;
}

bool Communication::SendSensorOriginalData(Vector3<int> acc, Vector3<int> gyro, Vector3<int> mag){
	u8 _cnt = 0;
	vs16 _temp;
	u8 data_to_send[30];


	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x02;
	data_to_send[_cnt++] = 0;

	_temp = acc.x;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = acc.y;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = acc.z;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = gyro.x;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = gyro.y;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = gyro.z;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);


	_temp = mag.x;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = mag.y;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = mag.z;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	data_to_send[3] = _cnt - 4;
	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;

	nrf.SendData(data_to_send, _cnt);
	return true;
}

bool Communication::SendRcvControlQuantity(){
	u8 _cnt = 0;
	u8 data_to_send[30];

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x03;
	data_to_send[_cnt++] = 0;

	data_to_send[_cnt++] = BYTE1(mRcvTargetThr);
	data_to_send[_cnt++] = BYTE0(mRcvTargetThr);

	data_to_send[_cnt++] = BYTE1(mRcvTargetYaw);
	data_to_send[_cnt++] = BYTE0(mRcvTargetYaw);

	data_to_send[_cnt++] = BYTE1(mRcvTargetRoll);
	data_to_send[_cnt++] = BYTE0(mRcvTargetRoll);

	data_to_send[_cnt++] = BYTE1(mRcvTargetPitch);
	data_to_send[_cnt++] = BYTE0(mRcvTargetPitch);

	u8 i = 12;
	while (i--)
		data_to_send[_cnt++] = 0;


	data_to_send[3] = _cnt - 4;
	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;

	nrf.SendData(data_to_send, _cnt);
	return true;
}

bool Communication::reply(u8 difference, u8 sum){
	u8 data_to_send[10];

	data_to_send[0] = 0xAA;
	data_to_send[1] = 0xAA;
	data_to_send[2] = 0xEF;
	data_to_send[3] = 2;
	data_to_send[4] = difference;
	data_to_send[5] = sum;

	u8 _sum = 0;
	for (u8 i = 0; i < 6; i++)
		_sum += data_to_send[i];
	data_to_send[6] = _sum;

	nrf.SendData(data_to_send, 7);
	return true;
}
bool Communication::SendPID(float p1_p, float p1_i, float p1_d, float p2_p, float p2_i, float p2_d, float p3_p, float p3_i, float p3_d){
	u8 data_to_send[30];
	u8 _cnt = 0;
	vs16 _temp;

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x10;
	data_to_send[_cnt++] = 0;

	_temp = p1_p * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p1_i * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p1_d * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p2_p * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p2_i * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p2_d * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p3_p * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p3_i * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p3_d * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	data_to_send[3] = _cnt - 4;
	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];

	data_to_send[_cnt++] = sum;
	nrf.SendData(data_to_send, _cnt);
	return true;
}

void Communication::SendMotoMsg(u16 m_1, u16 m_2, u16 m_3, u16 m_4, u16 m_5, u16 m_6, u16 m_7, u16 m_8){
	u8 data_to_send[30];
	u8 _cnt = 0;
	vs16 _temp;


	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x06;
	data_to_send[_cnt++] = 0;

	data_to_send[_cnt++] = BYTE1(m_1);
	data_to_send[_cnt++] = BYTE0(m_1);
	data_to_send[_cnt++] = BYTE1(m_2);
	data_to_send[_cnt++] = BYTE0(m_2);
	data_to_send[_cnt++] = BYTE1(m_3);
	data_to_send[_cnt++] = BYTE0(m_3);
	data_to_send[_cnt++] = BYTE1(m_4);
	data_to_send[_cnt++] = BYTE0(m_4);
	data_to_send[_cnt++] = BYTE1(m_5);
	data_to_send[_cnt++] = BYTE0(m_5);
	data_to_send[_cnt++] = BYTE1(m_6);
	data_to_send[_cnt++] = BYTE0(m_6);
	data_to_send[_cnt++] = BYTE1(m_7);
	data_to_send[_cnt++] = BYTE0(m_7);
	data_to_send[_cnt++] = BYTE1(m_8);
	data_to_send[_cnt++] = BYTE0(m_8);

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];

	data_to_send[_cnt++] = sum;

	nrf.SendData(data_to_send, _cnt);
}

void Communication::test(float a, float b, float c, float d, float e, float f, float g, float h, float i){
	u8 _cnt = 0;
	vs16 _temp;
	u8 data_to_send[30];


	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x02;
	data_to_send[_cnt++] = 0;


	_temp = (int)(a * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = (int)(b * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = (int)(c * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = (int)(d * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = (int)(e * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = (int)(f * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);


	_temp = (int)(g * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = (int)(h * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = (int)(i * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	data_to_send[3] = _cnt - 4;
	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;

	nrf.SendData(data_to_send, _cnt);

}
