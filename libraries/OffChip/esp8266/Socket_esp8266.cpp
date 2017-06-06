#include "Socket_esp8266.h"

Socket_esp8266::Socket_esp8266(USART& usart)
	:esp8266(usart), mUsart(usart), mIsConnected(false)
{
	mTimeOut = 2;
	mWifiStatus = WIFI_FREE;
	CSQvalue = 80;
}

bool Socket_esp8266::Init()
{
	mWifiStatus = WIFI_FREE;
	mWIFIInitStep = WIFIInit_START;
	mUsart.ClearRxBuf();
	mUsart.ClearTxBuf();
	return true;
}

bool Socket_esp8266::Connect(char* ipAddr, short port, Socket_Type socketType, Socket_Protocol socketProtocol)
{
	mUsart.ClearRxBuf();
	mUsart.ClearTxBuf();
	if (CreateConnectMutipleMode(ipAddr, port, socketType))
		mIsConnected = true;
	else
		mIsConnected = false;
	return mIsConnected;
}

bool Socket_esp8266::GetConnectStatus(void)
{
	return mIsConnected;
}

void Socket_esp8266::GetTCPStatus(void)
{
	if ((mWifiStatus == WIFI_FREE) && (mWIFIInitStep == WIFIInit_FINISH))
		mWifiStatus = WIFI_TCPSTATUS;
}

bool Socket_esp8266::Write(char* data, unsigned int num)
{
	if (!SendMultipleMode(data, num))
	{
		mIsConnected = false;
		Init();
		return false;
	}
	return true;
}

unsigned int Socket_esp8266::Read(char* data)
{
	float starttime = TaskManager::Time();
	if (!mIsConnected)
		return 0;
	unsigned char temp[13];
	unsigned short bufferSize = 0;
	unsigned short dataLength = 0;
	unsigned short count = 0;
	bool flag = false;
	//读出数据长度
	while (TaskManager::Time() - starttime < mTimeOut){
		bufferSize = mUsart.RxSize();
		if (bufferSize > 6)//数据帧格式：+IPD,长度:数据
		{
			for (count = 0; (count < bufferSize) && (count < ESP8266_RECEIVE_BUFFER_SIZE); ++count)
			{
				mUsart.GetBytes(temp, 1);
				if (temp[0] == '+')
				{
					if (!mIsEnableMUX)
						RecvFindAndFilter(":", ",", ":", (char*)temp, mTimeOut);
					else
					{
						if (RecvFind(":", mTimeOut))
						{
							char* index1 = strstr(mReceiveBuffer, ",");
							index1 = strstr(index1, ",");
							char* index2 = strstr(mReceiveBuffer, ":");

							if (index1&&index2)
							{
								index1 += strlen(",");
								*index2 = '\0';
								strcpy((char*)temp, index1);
								return true;
							}
						}
					}
					dataLength = atoi((char*)temp);
					flag = true;
					break;
				}
			}
		}
		if (flag)
			break;
	}
	count = 0;
	//读取数据
	while (TaskManager::Time() - starttime < mTimeOut){
		if (mUsart.RxSize()>0)
		{
			mUsart.GetBytes((unsigned char*)(data + count), 1);
			++count;
			if ((count == dataLength) || (count == ESP8266_RECEIVE_BUFFER_SIZE))
				break;
		}
	}
	return count;
}

unsigned int Socket_esp8266::Read(char* data, unsigned int num)
{
	float starttime = TaskManager::Time();
	if (!mIsConnected)
		return 0;
	unsigned int count = 0;
	while (TaskManager::Time() - starttime < mTimeOut&&count < num){
		if (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes((unsigned char*)(data + count), 1);
			++count;
		}
	}
	return count;
}

bool Socket_esp8266::IsAlive()
{
	if (!mIsConnected)
		return false;
	if (GetStatus() == 3)
		return true;
	return false;
}

bool Socket_esp8266::Close()
{
	if (mIsConnected)
	{
		mIsConnected = false;
		esp8266::Close();
	}
	return true;
}

void Socket_esp8266::SetTimeOut(float timetOut)
{
	mTimeOut = timetOut;
}

bool Socket_esp8266::SetMUX(bool isEnableMUX)
{
	mIsEnableMUX = isEnableMUX;
	return esp8266::SetMUX(isEnableMUX);
}

//****************** 应用于兆欧表
void Socket_esp8266::SetIP(char * mWIFISSID, char * mWIFIKEY, char *ipaddre, char *ipport)
{
	if ((strcmp(mWIFISSID, "") != 0) && (strcmp(mWIFIKEY, "") != 0))
	{
		strcpy(WIFISSID, mWIFISSID);
		strcpy(WIFIKEY, mWIFIKEY);
	}
	if ((strcmp(ipaddre, "") != 0) && (strcmp(ipport, "") != 0))
	{
		strcpy(IPAddress, ipaddre);
		strcpy(IPPort, ipport);
	}
}

void Socket_esp8266::CheckStatus()
{
	u8 temp;
	static double curTime = 0, TxoldTime = 0, RxoldTime = 0, oldTimeout = 0;
	static bool GetCSQTrue = false;
	curTime = tskmgr.Time();
	if ((curTime - oldTimeout > 5) && ((mWifiStatus == WIFI_FREE) && (mWIFIInitStep != WIFIInit_FINISH)))
		mWifiStatus = WIFI_FREE;
	if (((curTime - TxoldTime) > 2) || ((mWIFIInitStep != WIFIInit_FINISH) && (mWifiStatus == WIFI_FREE)))
	{
		TxoldTime = curTime;
		switch (mWifiStatus)
		{
		case WIFI_FREE:
		{
			mUsart.ClearRxBuf();
			mUsart.ClearTxBuf();
			ClearBuffer();
			switch (mWIFIInitStep)
			{
			case WIFIInit_START:
				ClearBuffer();
				oldTimeout = tskmgr.Time();			//初始化计时
				mIsConnected = false;
				mWIFIInitStep = WIFIInit_KICK;
				mWifiStatus = WIFI_CON;
				break;
			case WIFIInit_KICK:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_RESET;
				mWifiStatus = WIFI_RESET;
				break;
			case WIFIInit_RESET:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_ECHO;
				mWifiStatus = WIFI_ECHO;
				break;
			case WIFIInit_ECHO:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_MODE;
				mWifiStatus = WIFI_SETMODE;
				break;
			case WIFIInit_MODE:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_MUX;
				mWifiStatus = WIFI_SETMUX;
				break;
			case WIFIInit_MUX:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_QUITAP;
				mWifiStatus = WIFI_QUITAP;
				break;
			case WIFIInit_QUITAP:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_JOINAP;
				mWifiStatus = WIFI_JOINAP;
				break;
			case WIFIInit_JOINAP:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_TCPCLOSE;
				mWifiStatus = WIFI_TCPCLOSE;
				break;
			case WIFIInit_TCPCLOSE:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_TCPCON;
				mWifiStatus = WIFI_TCPCON;
				break;
			case WIFIInit_TCPCON:
				ClearBuffer();
				mWIFIInitStep = WIFIInit_FINISH;
				mWifiStatus = WIFI_FREE;
				break;
			case WIFIInit_FINISH:
				break;
			default:
				break;
			}
		}
		break;
		case WIFI_CON:
			ClearBuffer();
			mUsart << "AT\r\n";
			break;
		case WIFI_ECHO:
			ClearBuffer();
			mUsart << "ATE" << 0 << "\r\n";
			break;
		case WIFI_SETMODE:
			ClearBuffer();
			mUsart << "AT+CWMODE=" << (int)1 << "\r\n";
			break;
		case WIFI_RESET:
			ClearBuffer();
			mUsart << "AT+RST\r\n";
			break;
		case WIFI_SETMUX:
			ClearBuffer();
			mUsart << "AT+CIPMUX=" << 0 << "\r\n";
			break;
		case WIFI_QUITAP:
			ClearBuffer();
			mUsart << "AT+CWQAP\r\n";
			break;
		case WIFI_JOINAP:
			ClearBuffer();
			mUsart << "AT+CWJAP=" << "\"" << WIFISSID << "\",\"" << WIFIKEY << "\"\r\n";
			break;
		case WIFI_TCPCON:
			ClearBuffer();
			mUsart << "AT+CIPSTART=" << "\"" << "TCP" << "\",\"" << IPAddress << "\"," << IPPort << "\r\n";
			break;
		case WIFI_TCPCLOSE:
			ClearBuffer();
			mUsart << "AT+CIPCLOSE\r\n";
			break;
		case WIFI_TCPSTATUS:
			ClearBuffer();
			mUsart << "AT+CIPSTATUS\r\n";
			break;
		case WIFI_GETCSQ:
			GetCSQTrue = false;
			mUsart << "AT+CWLAP=\"" << WIFISSID << "\"\r\n";
			break;
		default:
			break;
		}
	}
	curTime = tskmgr.Time();
	if ((curTime - RxoldTime) > 0.001)
	{
		RxoldTime = curTime;
		switch (mWifiStatus)
		{
		case WIFI_FREE:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "TCP CLOSED") || strstr(mReceiveBuffer, "CLOSED"))
				{
					mIsConnected = false;
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_CON:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_ECHO:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_SETMODE:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_RESET:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "ready"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_SETMUX:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_QUITAP:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_JOINAP:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK"))
				{
					CSQvalue = 50;
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}  //**********************
				else if (strstr(mReceiveBuffer, "FAIL"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					mWIFIInitStep = WIFIInit_RESET;
					break;
				}
			}
			break;
		case WIFI_TCPCLOSE:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++ % ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK") || strstr(mReceiveBuffer, "ERROR"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_TCPCON:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++ % ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "OK") || strstr(mReceiveBuffer, "ALREADY CONNECTED"))
				{
					mIsConnected = true;
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
				else if (strstr(mReceiveBuffer, "ERROR"))
				{
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					mWIFIInitStep = WIFIInit_RESET;
					break;
				}
			}
			break;
		case WIFI_TCPSTATUS:
			while (mUsart.RxSize() > 0)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
				mReceiveBuffer[mReceiveBufferIndex] = '\0';
				if (strstr(mReceiveBuffer, "STATUS:3") || strstr(mReceiveBuffer, "CONNECTED") || strstr(mReceiveBuffer, "CONNECT"))
				{
					mIsConnected = true;
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
				else if (strstr(mReceiveBuffer, "STATUS:4") || strstr(mReceiveBuffer, "STATUS:5") || strstr(mReceiveBuffer, "CLOSED")) {
					mIsConnected = false;
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
				else if (strstr(mReceiveBuffer, "TCP CLOSED") || strstr(mReceiveBuffer, "CLOSED")) {
					mIsConnected = false;
					mReceiveBufferIndex = 0;
					mWifiStatus = WIFI_FREE;
					break;
				}
			}
			break;
		case WIFI_GETCSQ:
		{
			u8 tempValue = 0;
			while (mUsart.RxSize() > 0)//AT+CWLAP="InfiniteYuan" +CWLAP:(3,"InfiniteYuan",-50,"14:2d:27:fc:d7:5f",11)
			{
				mUsart.GetBytes(&temp, 1);
				if (temp == '\0')
					break;
				else if (temp == '-')
				{
					mReceiveBufferIndex = 0;
					GetCSQTrue = true;
				}
				if (GetCSQTrue)
				{
					mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
					if (temp == ','){
						mReceiveBuffer[mReceiveBufferIndex - 1] = '\0';
						tempValue = atoi((char*)(mReceiveBuffer + 1));
						if ((tempValue > 30) && (tempValue < 80))
							CSQvalue = tempValue;
						mReceiveBufferIndex = 0;
						mWifiStatus = WIFI_FREE;
						break;
					}
				}
				else {
					mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;
					mReceiveBuffer[mReceiveBufferIndex] = '\0';
					if (strstr(mReceiveBuffer, "TCP CLOSED") || strstr(mReceiveBuffer, "CLOSED")) {
						mIsConnected = false;
						mReceiveBufferIndex = 0;
						mWifiStatus = WIFI_FREE;
						break;
					}
					else if (strstr(mReceiveBuffer, "OK")) {
						CSQvalue = 80;
						mIsConnected = false;
						mReceiveBufferIndex = 0;
						mWifiStatus = WIFI_FREE;
						break;
					}
				}
			}
		}
		break;
		default:
			break;
		}
	}
	//重新链接
	if (!GetConnectStatus() && (mWifiStatus == WIFI_FREE) && (mWIFIInitStep == WIFIInit_FINISH))
	{
		mWIFIInitStep = WIFIInit_START;
	}
}

u8 Socket_esp8266::GetCSQ(void)
{
	if ((mWIFIInitStep == WIFIInit_FINISH) && (mWifiStatus == WIFI_FREE))
	{
		mWifiStatus = WIFI_GETCSQ;
	}
	return CSQvalue;
}
//****************** 应用于兆欧表
