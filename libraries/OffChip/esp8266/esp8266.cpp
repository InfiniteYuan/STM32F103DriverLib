#include "esp8266.h"

esp8266::esp8266(USART &usart, uint32_t baudRate)
	:mUsart(usart)
{
	mUsart.ClearRxBuf();
	mUsart.ClearTxBuf();
	if (baudRate != 0)
	{
		usart.SetBaudRate(baudRate);
	}
}

bool esp8266::Kick(void)
{
	mUsart.ClearRxBuf();
	mUsart.ClearTxBuf();
	mUsart << "AT\r\n";
	return RecvFind("OK");
}

bool esp8266::Restart(void)
{
	mUsart << "AT+RST\r\n";
	return RecvFind("OK");
}

u8 esp8266::GetCCSQ(char* WiFiSSID)
{
	mUsart << "AT+CWLAP=\"" << WiFiSSID << "\"\r\n";
	float starttime = TaskManager::Time();
	unsigned char temp[13];
	unsigned short bufferSize = 0;
	static unsigned short CSQvalue = 70;
	u8 tempValue = 0;
	unsigned short count = 0;
	bool flag = false;
	while ((TaskManager::Time() - starttime) < ESP8266_DEFAULT_TIMEOUT)
	{
		bufferSize = mUsart.RxSize();
		if (bufferSize > (strlen(WiFiSSID) + 20))//AT+CWLAP="InfiniteYuan" +CWLAP:(3,"InfiniteYuan",-50,"14:2d:27:fc:d7:5f",11)
		{
			for (count = 0; (count < bufferSize) && (count < ESP8266_RECEIVE_BUFFER_SIZE); ++count)
			{
				mUsart.GetBytes(temp, 1);
				if (temp[0] == '+')
				{
					mUsart.GetBytes(temp, 5);
					temp[5] = '\0';
					if (strcmp((char *)temp, (char *)"CWLAP") == 0)
						if (RecvFindAndFilter(",\"", "\",-", ",\"", (char*)temp, ESP8266_DEFAULT_TIMEOUT))
						{
							tempValue = atoi((char*)temp);
							flag = true;
							break;
						}
				}
			}
		}
		if (flag)
			break;
	}
	if ((tempValue > 30) && (tempValue < 80))
		CSQvalue = tempValue;
	return CSQvalue;
}

bool esp8266::GetVersion(char* version)
{
	mUsart << "AT+GMR\r\n";
	return RecvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", version);
}

bool esp8266::SetEcho(bool echo)
{
	mUsart << "ATE" << (echo ? 1 : 0) << "\r\n";
	return RecvFind("OK");
}

bool esp8266::Restore(void)
{
	mUsart << "AT+RESTORE" << "\r\n";
	return RecvFind("OK");
}

bool esp8266::SetUart(uint32_t baudrate, esp8266_pattern pattern)
{
	if (pattern > esp8266_PATTERN_DEF)
		return false;
	if (pattern == esp8266_PATTERN_CUR)
	{
		mUsart << "AT+UART_CUR=";
	}
	else if (pattern == esp8266_PATTERN_DEF)
	{
		mUsart << "AT+UART_DEF=";
	}
	else
	{
		mUsart << "AT+UART=";
	}
	mUsart << (int)baudrate << ",8,1,0,0\r\n";
	if (RecvFind("OK"))
		return true;
	return false;
}

bool esp8266::SetMode(esp8266_MODE mode, esp8266_pattern pattern)
{
	if (pattern > esp8266_PATTERN_DEF)
		return false;

	if (pattern == esp8266_PATTERN_CUR)
	{
		mUsart << "AT+CWMODE_CUR=";
	}
	else if (pattern == esp8266_PATTERN_DEF)
	{
		mUsart << "AT+CWMODE_DEF=";
	}
	else
	{
		mUsart << "AT+CWMODE=";
	}
	mUsart << (int)mode << "\r\n";
	if (!ReceiveAndWait("OK", "ERROR"))
		return false;
	if (strstr(mReceiveBuffer, "OK"))
		return true;
	return false;
}

bool esp8266::SetMUX(bool isEnableMUX)
{
	mUsart << "AT+CIPMUX=" << (isEnableMUX ? 1 : 0) << "\r\n";
	return RecvFind("OK");
}

bool esp8266::SetApParam(char* apSetSsid, char* apSetPasswd, unsigned char channel, esp8266_ENCRYPTION encryption, esp8266_pattern pattern)
{
	if (pattern > esp8266_PATTERN_DEF)
		return false;

	if (pattern == esp8266_PATTERN_CUR)
	{
		mUsart << "AT+CWSAP_CUR=";
	}
	else if (pattern == esp8266_PATTERN_DEF)
	{
		mUsart << "AT+CWSAP_DEF=";
	}
	else
	{
		mUsart << "AT+CWSAP=";
	}
	mUsart << "\"" << apSetSsid << "\",\"" << apSetPasswd << "\"," << channel << "," << (int)encryption << "\r\n";
	if (!ReceiveAndWait("OK", "ERROR", 20))
		return false;
	if (strstr(mReceiveBuffer, "OK"))
		return true;
	return false;
}

bool esp8266::JoinAP(char* apJoinSsid, char* apJoinPasswd, esp8266_pattern pattern)
{
	mUsart.ClearRxBuf();
	mUsart.ClearTxBuf();
	if (pattern > esp8266_PATTERN_DEF)
		return false;

	if (pattern == esp8266_PATTERN_CUR)
	{
		mUsart << "AT+CWJAP_CUR=";
	}
	else if (pattern == esp8266_PATTERN_DEF)
	{
		mUsart << "AT+CWJAP_DEF=";
	}
	else
	{
		mUsart << "AT+CWJAP=";
	}
	mUsart << "\"" << apJoinSsid << "\",\"" << apJoinPasswd << "\"\r\n";
	if (!ReceiveAndWait("OK", "ERROR", 20))
		return false;
	if (strstr(mReceiveBuffer, "OK"))
		return true;
	return false;
}

bool esp8266::QuitAP(void)
{
	mUsart.ClearRxBuf();
	mUsart.ClearTxBuf();
	mUsart << "AT+CWQAP\r\n";
	return RecvFind("OK");
}

bool esp8266::CreateConnectMutipleMode(char* ipAddr, short port, Socket_Type socketType, signed char muxID)
{
	char type[4] = "TCP";
	if (socketType == Socket_Type_Dgram)
	{
		type[0] = 'U';
		type[1] = 'D';
	}
	if (muxID != -1)
		mUsart << "AT+CIPSTART=" << muxID << ",\"" << type << "\",\"" << ipAddr << "\"," << port << "\r\n";
	else
		mUsart << "AT+CIPSTART=" << "\"" << type << "\",\"" << ipAddr << "\"," << port << ",1\r\n";
	return RecvFind("OK", "ERROR", "ALREAY CONNECT");
}

bool esp8266::SendMultipleMode(char* data, unsigned int num, signed char muxID)
{
	if (muxID != -1)
		mUsart << "AT+CIPSEND=" << muxID << "," << (int)num << "\r\n";
	else
		mUsart << "AT+CIPSEND=" << (int)num << "\r\n";
	if (!RecvFind("OK\r\n> "))
		return false;
	mUsart.SendBytes((uint8_t*)data, num);
	return RecvFind("OK\r\n", "ERROR");
}

bool esp8266::Close(signed char muxID)
{
	if (muxID != -1)
		mUsart << "AT+CIPCLOSE=" << muxID << "\r\n";
	else
		mUsart << "AT+CIPCLOSE\r\n";
	return RecvFind("OK", "ERROR");
}

/**
  *未写完全
  *
  */
char esp8266::GetStatus(char* muxID, char* type, char* ipAddr, short remotePort, short localPort, esp8266_ROLE role)
{
	char status = 0;
	bool result = false;
	mUsart << "AT+CIPSTATUS\r\n";
	result = RecvFindAndFilter("+CIPSTATU", "STATUS:", "\r\n", &status);
	status -= '0';
	return result;
}

bool esp8266::ReceiveAndWait(const char* targetString, unsigned char timeOut)
{
	u8 temp;
	mReceiveBufferIndex = 0;
	ClearBuffer();
	double tartTime = TaskManager::Time();
	while ((TaskManager::Time() - tartTime) < timeOut)
	{
		while (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes(&temp, 1);//从串口接收缓冲区接收数据
			if (temp == '\0')
				continue;
			mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;//放入esp的缓冲区

		}

		if (strstr(mReceiveBuffer, targetString))
			return true;

	}
	if (mReceiveBufferIndex > 0)//接收到了数据，加上结束标志
	{
		mReceiveBuffer[mReceiveBufferIndex%ESP8266_RECEIVE_BUFFER_SIZE] = '\0';
	}
	return false;
}

bool esp8266::ReceiveAndWait(char const* targetString, const char* targetString2, unsigned char timeOut)
{
	u8 temp;
	mReceiveBufferIndex = 0;
	ClearBuffer();
	double tartTime = TaskManager::Time();
	while ((TaskManager::Time() - tartTime) < timeOut)
	{
		while (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes(&temp, 1);//从串口接收缓冲区接收数据
			if (temp == '\0')
				continue;
			mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;//放入esp的缓冲区

		}

		if (strstr(mReceiveBuffer, targetString) ||
			strstr(mReceiveBuffer, targetString2))
			return true;

	}
	if (mReceiveBufferIndex > 0)//接收到了数据，加上结束标志
	{
		mReceiveBuffer[mReceiveBufferIndex%ESP8266_RECEIVE_BUFFER_SIZE] = '\0';
	}
	return false;
}

bool esp8266::ReceiveAndWait(char const* targetString, const char* targetString2, const char* targetString3, unsigned char timeOut)
{
	u8 temp;
	mReceiveBufferIndex = 0;
	ClearBuffer();
	double tartTime = TaskManager::Time();
	while ((TaskManager::Time() - tartTime) < timeOut)
	{
		while (mUsart.RxSize() > 0)
		{
			mUsart.GetBytes(&temp, 1);//从串口接收缓冲区接收数据
			if (temp == '\0')
				continue;
			mReceiveBuffer[mReceiveBufferIndex++%ESP8266_RECEIVE_BUFFER_SIZE] = temp;//放入esp的缓冲区
		}

		if (strstr(mReceiveBuffer, targetString) ||
			strstr(mReceiveBuffer, targetString2) ||
			strstr(mReceiveBuffer, targetString3))
			return true;

	}
	if (mReceiveBufferIndex > 0)//接收到了数据，加上结束标志
	{
		mReceiveBuffer[mReceiveBufferIndex%ESP8266_RECEIVE_BUFFER_SIZE] = '\0';
	}
	return false;
}

bool esp8266::RecvFind(char const *target, unsigned char timeout)
{
	if (!ReceiveAndWait((char*)target, timeout))
		return false;
	return true;
}

bool esp8266::RecvFind(char const *target, char const *target2, unsigned char timeout)
{
	if (!ReceiveAndWait((char*)target, target2, timeout))
		return false;
	return true;
}

bool esp8266::RecvFind(char const *target, char const *target2, char const *target3, unsigned char timeout)
{
	if (!ReceiveAndWait((char*)target, target2, target3, timeout))
		return false;
	return true;
}

bool esp8266::RecvFindAndFilter(char const *target, char const * begin, char const * end, char* Data, float timeout)
{
	if (!ReceiveAndWait((char*)target, timeout))
		return false;
	char* index1 = strstr(mReceiveBuffer, begin);
	char* index2 = strstr(index1, end);

	if (index1&&index2)
	{
		index1 += strlen(begin);
		*index2 = '\0';
		strcpy(Data, index1);
		return true;
	}
	return false;
}

void esp8266::ClearBuffer()
{
	unsigned int i = 0;
	for (; i < ESP8266_RECEIVE_BUFFER_SIZE; ++i)
		mReceiveBuffer[i] = 0;
	mReceiveBufferIndex = 0;
}
