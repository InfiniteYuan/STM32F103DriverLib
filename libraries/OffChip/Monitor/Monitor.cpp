#include "Monitor.h"

const char Monitor::FrameTail[4] = { 0xFF, 0xFC, 0xFF, 0xFF };

Monitor::Monitor(USART &Com) :mCom(Com)
{
	count = 0;
}

bool Monitor::ReadCommand()
{
	u8 bufferSize = 0;
	u8 i = 0;
	if (mCom.RxSize() > 0)
	{
		bufferSize = mCom.RxSize();
		for (i = 0; i < bufferSize; i++)
		{
			mCom.GetBytes((unsigned char *)(temp + count), 1);
			count++;
			if ((count > 5) && (temp[count - 4] == 0xFF) && (temp[count - 3] == 0xFC) && (temp[count - 2] == 0xFF) && (temp[count - 1] == 0xFF))
			{
				temp[count] = '\0';
				count = 0;
				return true;
			}
			if (temp[count - 1] == 0xEE)
			{
				count = 0;
				temp[count] = 0xEE;
				count++;
			}
		}
	}
	return false;
}

CommandData Monitor::DecodeCommand()
{
	char * index;
	CommandData mCommandData;
	//	static u8 temp[50];
	if (temp[0] == 0xEE)//帧头
	{
		if (temp[1] == 0xF7)//读取时间
		{
			if ((temp[9] == 0xFF) && (temp[10] == 0xFC) && (temp[11] == 0xFF) && (temp[12] == 0xFF))
			{
				mCommandData.mBtnType = READDATETIME;
				mCommandData.data[0] = temp[2];
				mCommandData.data[1] = temp[3];
				sprintf(mCommandData.data, "20%d%d-%d%d-%d%d,%d%d:%d%d:%d%d", temp[2] / 0x10, temp[2] % 0x10, temp[3] / 0x10, temp[3] % 0x10, temp[5] / 0x10, temp[5] % 0x10, temp[6] / 0x10, temp[6] % 0x10, temp[7] / 0x10, temp[7] % 0x10, temp[8] / 0x10, temp[8] % 0x10);
				return mCommandData;
			}
		}
		if ((temp[1] == 0xB1) && (temp[2] == 0x10))//设置按钮按下和弹起状态、更新文本控件数值
		{
			if ((temp[3] == 0x00) && (temp[4] == 0x03))//SCREEN_ID
			{
				if ((temp[5] == 0x00) && (temp[6] == 0x07) && (temp[7] == 0x01) && (temp[8] == 0x00))//TRAINSELECT CH1 CONTROL_ID
				{
					mCommandData.mBtnType = CH_TRAIN1;
					return mCommandData;
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x09) && (temp[7] == 0x01) && (temp[8] == 0x00))//TRAINSELECT CH2 CONTROL_ID
				{
					mCommandData.mBtnType = CH_TRAIN2;
					return mCommandData;
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0A) && (temp[7] == 0x01) && (temp[8] == 0x00))//TRAINSELECT CH3 CONTROL_ID
				{
					mCommandData.mBtnType = CH_TRAIN3;
					return mCommandData;
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0B) && (temp[7] == 0x01) && (temp[8] == 0x00))//TRAINSELECT CH4 CONTROL_ID
				{
					mCommandData.mBtnType = CH_TRAIN4;
					return mCommandData;
				}
			}
		}
		if ((temp[1] == 0xB1) && (temp[2] == 0x11))//读取按钮控件的状态、读取文本控件数值
		{
			/************** SCREEN_ID 0x00 ***************/
			if ((temp[3] == 0x00) && (temp[4] == 0x00))//SCREEN_ID
			{
				if ((temp[5] == 0x00) && (temp[6] == 0x01))//CHD600P CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x01) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_D600PBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x02))//CHD600N CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x01) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_D600NBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x03))//CHD110P CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x01) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_D110PBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x04))//CH_D110NBTN CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x01) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_D110NBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x05))//CH_DPNBTN CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x01) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_DPNBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x06))//CLEARBTN CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CLEARBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x07))//PRINTBTN CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = PRINTBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x08))//MEASURE CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = MEASUREBTN;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x09))//TRAINSELECT CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = TRAINSELECT; //车次选择
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0A))//ONE PEOPLE TEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = PEOBTN1;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0B))//TWO PEOPLE TEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = PEOBTN2;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0C))//THREE PEOPLE TEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = PEOBTN3;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x10))//CONFIGBTN CONTROL_ID
				{
					if (temp[7] == 0x10)
					{
						mCommandData.mBtnType = CONFIGBTN;
						return mCommandData;
					}
				}
			}
			/************** SCREEN_ID 0x01 ***************/
			if ((temp[3] == 0x00) && (temp[4] == 0x01))//SCREEN_ID
			{
				if ((temp[5] == 0x00) && (temp[6] == 0x0B))//PRINTSURE CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = PRINTSUREBTN;
						return mCommandData;
					}
				}
			}
			/************** SCREEN_ID 0x03 TRAIN SELECT UI ***************/
			if ((temp[3] == 0x00) && (temp[4] == 0x03))//SCREEN_ID
			{
				if ((temp[5] == 0x00) && (temp[6] == 0x0A))//CH_TRAIN1 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN1;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0B))//CH_TRAIN2 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN2;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0C))//CH_TRAIN3 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN3;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0D))//CH_TRAIN4 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN4;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0E))//CH_TRAIN5 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN5;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x0F))//CH_TRAIN6 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN6;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x10))//CH_TRAIN7 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN7;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x11))//CH_TRAIN8 CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = CH_TRAIN8;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x14))//LASTTRAINPAGE CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = LASTTRAINPAGE;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x15))//NEXTTRAINPAGE CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01) && (temp[9] == 0x00) && (temp[10] == 0xFF) && (temp[11] == 0xFC) && (temp[12] == 0xFF) && (temp[13] == 0xFF))
					{
						mCommandData.mBtnType = NEXTTRAINPAGE;
						return mCommandData;
					}
				}
			}
			/************** SCREEN_ID 0x05 WIFI SET UI ***************/
			if ((temp[3] == 0x00) && (temp[4] == 0x05))//SCREEN_ID
			{
				if ((temp[5] == 0x00) && (temp[6] == 0x01))//WIFISET CONTROL_ID
				{
					if (temp[7] == 0x10 && (temp[8] == 0x01))
					{
						mCommandData.mBtnType = WIFISET;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x02))//GPRSSET CONTROL_ID
				{
					if (temp[7] == 0x10 && (temp[8] == 0x00) && (temp[9] == 0x00))
					{
						mCommandData.mBtnType = GPRSSET;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x05))//WIFISSIDTEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = WIFISSIDTEXT;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x06))//WIFIKEYTEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = WIFIKEYTEXT;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x07))//WIFIIPADDRTEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = WIFIIPADDRTEXT;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x08))//WIFIIPPORTTEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = WIFIIPPORTTEXT;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
			}
			/************** SCREEN_ID 0x07 GPRS SET UI ***************/
			if ((temp[3] == 0x00) && (temp[4] == 0x07))//SCREEN_ID
			{
				if ((temp[5] == 0x00) && (temp[6] == 0x01))//GPRSIPADDRTEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = GPRSIPADDRTEXT;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x02))//GPRSIPPORTTEXT CONTROL_ID
				{
					if (temp[7] == 0x11)
					{
						index = strstr((char *)temp, (char *)FrameTail);
						index[0] = '\0';
						mCommandData.mBtnType = GPRSIPPORTTEXT;
						strcpy(mCommandData.data, (char *)&temp[8]);
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x04))//WIFISET CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x00) && (temp[9] == 0x00))
					{
						mCommandData.mBtnType = WIFISET;
						return mCommandData;
					}
				}
				if ((temp[5] == 0x00) && (temp[6] == 0x05))//GPRSSET CONTROL_ID
				{
					if ((temp[7] == 0x10) && (temp[8] == 0x01))
					{
						mCommandData.mBtnType = GPRSSET;
						return mCommandData;
					}
				}
			}
		}
		if ((temp[1] == 0xB1) && (temp[2] == 0x00))//切换画面
		{

		}
		if ((temp[1] == 0xB1) && (temp[2] == 0x01))//读取画面
		{

		}
	}
	mCommandData.mBtnType = NULLBTN;
	mCommandData.data[0] = '\0';
	return mCommandData;
}

bool Monitor::RedirectTrainSelect()
{
	u8 command[9] = { 0xEE, 0xB1, 0x00, 0x00, 0x03, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 00 00 03 FF FC FF FF
	mCom.SendBytes(command, 9);
	return true;
}


bool Monitor::RedirectWIFI(void)
{
	u8 command[9] = { 0xEE, 0xB1, 0x00, 0x00, 0x05, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 00 00 05 FF FC FF FF
	mCom.SendBytes(command, 9);
	return true;
}

bool Monitor::RedirectGPRS(void)
{
	u8 command[9] = { 0xEE, 0xB1, 0x00, 0x00, 0x07, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 00 00 07 FF FC FF FF
	mCom.SendBytes(command, 9);
	return true;
}

bool Monitor::ReadDateTime()
{
	u8 command[6] = { 0xEE, 0x82, 0xFF, 0xFC, 0xFF, 0xFF };//EE 81 59 06 14 21 01 03 17 FF FC FF FF
	mCom.SendBytes(command, 6);
	return true;
}

bool Monitor::SetTemperature(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x0D };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetHumidity(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x0E };//EE B1 10 00 00 00 0E + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetWiFiSSID(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x05, 0x00, 0x05 };//EE B1 10 00 05 00 05 +??+FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetWiFiKEY(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x05, 0x00, 0x06 };//EE B1 10 00 05 00 06 +??+FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetWiFiIP(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x05, 0x00, 0x07 };//EE B1 10 00 05 00 07 +??+ FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetWiFiIPPort(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x05, 0x00, 0x08 };//EE B1 10 00 05 00 08 +??+FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetGPRSIP(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x07, 0x00, 0x01 };//EE B1 10 00 00 00 0E + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetGPRSIPPort(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x07, 0x00, 0x02 };//EE B1 10 00 00 00 0E + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetWifiSignal(u8 degree)
{
	u8 command[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x1A, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 23 00 00 00 1A 00 FF FC FF FF
	command[7] = degree;
	mCom.SendBytes(command, 12);
	return true;
}

bool Monitor::SetWifiDataLogo(bool isData)
{
	static bool isWifiData = false;
	u8 command[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x19, 0x01, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 23 00 00 00 1A 00 FF FC FF FF
	u8 command2[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x19, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 23 00 00 00 1A 00 FF FC FF FF
	if (isData)
	{
		isWifiData = !isWifiData;
		isWifiData ? command[7] = 0x01 : command[7] = 0x02;
		mCom.SendBytes(command, 12);
	}
	else{
		mCom.SendBytes(command2, 12);
	}
	return true;
}

bool Monitor::SetGprsSignal(u8 degree)
{
	u8 command[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x1A, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 23 00 00 00 18 00 FF FC FF FF
	command[7] = degree + 5;
	mCom.SendBytes(command, 12);
	return true;
}

bool Monitor::SetGPRSDataLogo(bool isData)
{
	static bool isGPRSData = false;
	u8 command[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x19, 0x01, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 23 00 00 00 1A 00 FF FC FF FF
	u8 command2[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x19, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 23 00 00 00 1A 00 FF FC FF FF
	if (isData)
	{
		isGPRSData = !isGPRSData;
		isGPRSData ? command[7] = 0x01 : command[7] = 0x02;
		mCom.SendBytes(command, 12);
	}
	else{
		mCom.SendBytes(command2, 12);
	}
	return true;
}

bool Monitor::SetUSBLogo(bool isCome)
{
	u8 command[12] = { 0xEE, 0xB1, 0x03, 0x00, 0x00, 0x00, 0x1C, 0x01, 0xFF, 0xFC, 0xFF, 0xFF };
	command[7] = isCome ? 0x01 : 0x00;
	mCom.SendBytes(command, 12);
	return true;
}

bool Monitor::SetBattery(u8 value)
{
	u8 command[15] = { 0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };
	command[10] = value;
	mCom.SendBytes(command, 15);
	return true;
}

bool Monitor::RedirectMain()
{
	u8 command[9] = { 0xEE, 0xB1, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 00 00 00 FF FC FF FF
	mCom.SendBytes(command, 9);
	return true;
}

bool Monitor::RedirectInit()
{
	u8 command[9] = { 0xEE, 0xB1, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 00 00 00 FF FC FF FF
	mCom.SendBytes(command, 9);
	return true;
}

bool Monitor::RedirectMeasure()
{
	u8 command[9] = { 0xEE, 0xB1, 0x00, 0x00, 0x02, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 00 00 02 FF FC FF FF
	mCom.SendBytes(command, 9);
	u8 command2[11] = { 0xEE, 0xB1, 0x20, 0x00, 0x02, 0x00, 0x02, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 20 00 02 00 02 FF FC FF FF
	mCom.SendBytes(command2, 11);
	return true;
}

bool Monitor::StopAnimation(void)
{
	u8 command2[11] = { 0xEE, 0xB1, 0x21, 0x00, 0x02, 0x00, 0x02, 0xFF, 0xFC, 0xFF, 0xFF };	//EE B1 21 00 02 00 02 FF FC FF FF
	mCom.SendBytes(command2, 11);
	return true;
}

bool Monitor::RedirectPrint()
{
	u8 command[9] = { 0xEE, 0xB1, 0x00, 0x00, 0x01, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 00 00 02 FF FC FF FF
	mCom.SendBytes(command, 9);
	return true;
}

bool Monitor::HidePrinting()
{
	u8 command[12] = { 0xEE, 0xB1, 0x03, 0x00, 0x01, 0x00, 0x0C, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 03 00 01 00 0C 00 FF FC FF FF
	mCom.SendBytes(command, 12);
	return true;
}

bool Monitor::ComePrinting()
{
	u8 command[12] = { 0xEE, 0xB1, 0x03, 0x00, 0x01, 0x00, 0x0C, 0x01, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 03 00 01 00 0C 01 FF FC FF FF
	mCom.SendBytes(command, 12);
	return true;
}

bool Monitor::DisableCancelBtn()
{
	u8 command[12] = { 0xEE, 0xB1, 0x04, 0x00, 0x01, 0x00, 0x0A, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 04 00 01 00 0A 00 FF FC FF FF
	mCom.SendBytes(command, 12);
	return true;
}

bool Monitor::EnableCancelBtn()
{
	u8 command[12] = { 0xEE, 0xB1, 0x04, 0x00, 0x01, 0x00, 0x0A, 0x01, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 04 00 01 00 0A 01 FF FC FF FF
	mCom.SendBytes(command, 12);
	return true;
}

bool Monitor::SendResidueTrain(u8 value, u8 pagesum)
{
	char str[5];
	u8 command[12] = { 0xEE, 0xB1, 0x10, 0x00, 0x03, 0x00, 0x09, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 10 00 03 00 05+ VALUE +FF FC FF FF
	command[7] = value;
	sprintf(str, "%d/%d", value, pagesum);
	mCom.SendBytes(command, 7);
	mCom << str;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetMainTrain(char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x17 };//EE B1 10 00 00 00 17 + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetChTrain(u8 channel, char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x03, 0x00, 0x01 };//EE B1 10 00 03 00 01 + VALUE +FF FC FF FF
	command[6] = channel;
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetChPeoPle(u8 channel, char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x0A };//EE B1 10 00 00 00 0A + VALUE +FF FC FF FF
	command[6] = channel + 0x0A;
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetChMeasure(u8 channel, char *data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x11 };//EE B1 10 00 00 00 11+ VALUE + FF FC FF FF
	command[6] = channel + 0x11;
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetSelectChMea1(void)
{
	u8 command[8] = { 0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01 };//EE ?B1 10 Screen_id Control_id Status ?FF FC FF FF
	mCom.SendBytes(command, 8);
	mCom << FrameTail;
	command[7] = 0x00;
	command[6] = 0x02;
	mCom.SendBytes(command, 8);
	mCom << FrameTail;
	command[6] = 0x03;
	mCom.SendBytes(command, 8);
	mCom << FrameTail;
	command[6] = 0x04;
	mCom.SendBytes(command, 8);
	mCom << FrameTail;
	command[6] = 0x05;
	mCom.SendBytes(command, 8);
	mCom << FrameTail;
	return true;
}

//********************* 打印预览 **************
bool Monitor::SetPrintTrain(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x01 };//EE B1 10 00 01 00 01 + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintDate(char * data)
{
	char data1[20];
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x02 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	u8 command1[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x0D };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	strcpy(data1, data);
	data1[4] = '\0';
	mCom.SendBytes(command, 7);
	mCom << data1;
	mCom << FrameTail;
	mCom.SendBytes(command1, 7);
	mCom << &data1[5];
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintHum(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x03 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintD600P(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x04 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintD600N(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x05 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintD110P(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x07 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintD110N(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x08 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintDPN(char * data)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x06 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	mCom << data;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetPrintPeoPle(char * data, char * data1, char * data2)
{
	u8 command[7] = { 0xEE, 0xB1, 0x10, 0x00, 0x01, 0x00, 0x09 };//EE B1 10 00 00 00 0D + VALUE +FF FC FF FF
	mCom.SendBytes(command, 7);
	if ((strcmp(data, "") == 0) && (strcmp(data1, "") == 0) && (strcmp(data2, "") == 0))
		mCom << "";
	else if ((strcmp(data, "") == 0) && (strcmp(data1, "") != 0) && (strcmp(data2, "") == 0))
		mCom << data1;
	else if ((strcmp(data, "") == 0) && (strcmp(data1, "") == 0) && (strcmp(data2, "") != 0))
		mCom << data2;
	else if ((strcmp(data, "") == 0) && (strcmp(data1, "") != 0) && (strcmp(data2, "") != 0))
		mCom << data1 << "、" << data2;
	else if ((strcmp(data, "") != 0) && (strcmp(data1, "") == 0) && (strcmp(data2, "") == 0))
		mCom << data;
	else if ((strcmp(data, "") != 0) && (strcmp(data1, "") != 0) && (strcmp(data2, "") == 0))
		mCom << data << "、" << data1;
	else if ((strcmp(data, "") != 0) && (strcmp(data1, "") == 0) && (strcmp(data2, "") != 0))
		mCom << data << "、" << data2;
	else if ((strcmp(data, "") != 0) && (strcmp(data1, "") != 0) && (strcmp(data2, "") != 0))
		mCom << data << "、" << data1 << "、" << data2;
	mCom << FrameTail;
	return true;
}

bool Monitor::SetChargeIcon(bool showFlag)
{
	if (showFlag)
	{
		u8 command2[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x1D, 0x01, 0xFF, 0xFC, 0xFF, 0xFF };//EE B1 23 00 00 00 1D 01 FF FC FF FF
		mCom.SendBytes(command2, 12);
	}
	else
	{
		u8 command2[12] = { 0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x1D, 0x00, 0xFF, 0xFC, 0xFF, 0xFF };	//EE B1 23 00 00 00 1D 00 FF FC FF FF
		mCom.SendBytes(command2, 12);
	}
	return true;
}
