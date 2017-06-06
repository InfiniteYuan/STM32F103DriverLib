#ifndef __SOCKET_ESP8266_H
#define __SOCKET_ESP8266_H
#include "socket.h"
#include "esp8266.h"
#include "string.h"
#include "stdlib.h"
#include "USART.h"

typedef enum WIFIStatus
{
	WIFI_FREE,
	WIFI_CON,
	WIFI_ECHO,
	WIFI_SETMODE,
	WIFI_RESET,
	WIFI_SETMUX,
	WIFI_QUITAP,
	WIFI_JOINAP,
	WIFI_TCPCON,
	WIFI_TCPCLOSE,
	WIFI_TCPSTATUS,
	WIFI_GETCSQ,
} WIFIStatus;

typedef enum WIFIInitStep
{
	WIFIInit_START,
	WIFIInit_KICK,
	WIFIInit_ECHO,
	WIFIInit_MODE,
	WIFIInit_RESET,
	WIFIInit_MUX,
	WIFIInit_QUITAP,
	WIFIInit_JOINAP,
	WIFIInit_TCPCLOSE,
	WIFIInit_TCPCON,
	WIFIInit_FINISH,
}	WIFIInitStep;

class Socket_esp8266 :public esp8266, public Socket
{
private:
	USART& mUsart;
	char WIFISSID[30];
	char WIFIKEY[30];
	char IPAddress[20];
	char IPPort[10];
	float mTimeOut;
	bool mIsConnected;
	bool mIsEnableMUX;

public:
	WIFIStatus mWifiStatus;
	WIFIInitStep mWIFIInitStep;
	Socket_esp8266(USART& usart);
	unsigned short CSQvalue;
	virtual bool Init(void);
	virtual bool Connect(char* ipAddr, short port, Socket_Type socketType, Socket_Protocol socketProtocol = Socket_Protocol_IPV4);
	virtual bool Write(char* data, unsigned int num);
	virtual unsigned int Read(char* data);
	virtual unsigned int Read(char* data, unsigned int num);
	virtual bool IsAlive(void);
	virtual bool Close(void);
	virtual void SetTimeOut(float timetOut);

	bool SetMUX(bool isEnableMUX);
	bool GetConnectStatus(void);
	void GetTCPStatus(void);

	//****************** 应用于兆欧表
	void SetIP(char *WIFISSID, char *WIFIKEY, char *ipadder, char *ipport);
	void CheckStatus(void);
	u8 GetCSQ(void);
	//****************** 应用于兆欧表
};

#endif
