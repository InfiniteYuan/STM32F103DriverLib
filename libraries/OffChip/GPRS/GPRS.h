#ifndef _GPRS_H
#define _GPRS_H

#include "stm32f4xx.h"
#include "FIFOBuffer.h"
#include "USART.h"
#include "string.h"
#include "stdlib.h"
#include "socket.h"
#include "TaskManager.h"

//enum Socket_Type{Socket_Type_Stream,Socket_Type_Dgram};//TCP & UDP
//enum Socket_Protocol{Socket_Protocol_IPV4,Socket_Protocol_IPV6};

#define GPRS_RECEIVE_BUFFER_SIZE  50
#define GPRS_DEFAULT_PATTERN  esp8266_PATTERN_DEF
#define GPRS_DEFAULT_TIMEOUT  2 

typedef enum GPRSStatus
{
	GPRS_FREE,
	GPRS_CON,
	GPRS_SET_ECHO,
	GPRS_IS_PIN,
	GPRS_GET_CCSQ,
	GPRS_REG,
	GPRS_DEA_PDP,
	GPRS_SET_GPRSCLASS,
	GPRS_DEFIN_PDP,
	GPRS_ATTACH_GPRS,
	GPRS_SET_CSD,
	GPRS_ADD_IPHEAD,
	GPRS_CLOSE_TCP,
	GPRS_TCP_CONNECT,
	GPRS_TCP_SEND,
	GPRS_TCP_STATUS,
} GPRSStatus;

typedef enum GPRSInitStep
{
	GPRSInit_START,
	GPRSInit_KICK,
	GPRSInit_ECHO,
	GPRSInit_ISPIN,
	GPRSInit_GETCCSQ,
	GPRSInit_ISREG,
	GPRSInit_DEAPDP,
	GPRSInit_STACLA,
	GPRSInit_DEFPDP,
	GPRSInit_ATTACH,
	GPRSInit_SETCSD,
	GPRSInit_ADDHEAD,
	GPRSInit_CONNECT,
	GPRSInit_FINISH,
}	GPRSInitStep;

class GPRS
{
private:
	enum GPRS_pattern{ GPRS_PATTERN_NULL = 0, GPRS_PATTERN_CUR = 1, GPRS_PATTERN_DEF = 2 };

	USART& mUsart;
	char IPAddress[20];
	char IPPort[10];
	float mTimeOut;
	bool mIsConnected;
	bool mIsEnableMUX;
	unsigned int mReceiveBufferIndex;

	bool ReceiveAndWait(char const* targetString, unsigned char timeOut = GPRS_DEFAULT_TIMEOUT);
	bool ReceiveAndWait(char const* targetString, char const* targetString2, unsigned char timeOut = GPRS_DEFAULT_TIMEOUT);
	bool ReceiveAndWait(char const* targetString, char const* targetString2, char const* targetString3, unsigned char timeOut = GPRS_DEFAULT_TIMEOUT);

	bool RecvFind(char const *target, unsigned char timeout = GPRS_DEFAULT_TIMEOUT);
	bool RecvFind(char const *target, char const *target2, unsigned char timeout = GPRS_DEFAULT_TIMEOUT);
	bool RecvFind(char const *target, char const *target2, char const *target3, unsigned char timeout = GPRS_DEFAULT_TIMEOUT);
	bool RecvFindAndFilter(char const *target, char const * begin, char const * end, char* Data, float timeout = GPRS_DEFAULT_TIMEOUT);
protected:
	char mReceiveBuffer[GPRS_RECEIVE_BUFFER_SIZE];
public:
	GPRSStatus mGPRSStatus;
	GPRSInitStep mGPRSInitStep;
	unsigned short CSQvalue;
	GPRS(USART& usart);
	//*********GPRS_Init********
	bool Kick(void);
	bool Restart(void);
	bool GetVersion(char*);
	bool SetEcho(bool echo);
	bool Restore(void);
	bool SetUart(uint32_t baudrate, GPRS_pattern pattern);
	void SetIP(char *ipadder, char *ipport);

	void ClearBuffer();
	//*********TCP/IP***********
	bool init(void);
	bool GetConnectStatus(void);
	bool AttachGPRSService(bool IsAttached);
	//******关闭连接
	bool CloseConnected(bool IsFast = 0);
	//设置接收书显示IP头，
	bool Add_an_IP_Head(bool IsHead);
	//设置PDP上下文，互联网协议，接入点信息
	bool Define_PDP_Context(void);
	//设置GPRS移动台类别
	bool GPRS_Mobile_Station_Class(char* cla);
	/****关闭移动场景****/
	bool Deactivate_GPRS_PDP_Context(void);
	//激活移动场景 建立无线链路
	bool DeactivateMobileContext(void);
	//设置APN 接入点、用户名、密码、 启动任务 
	bool SetAPN(char* APNstr);
	//**********设置GPRS连接模式*****
	//0 set CSD as wireless connection mode
	//1 set GPRS as wireless connection mode
	bool Set_CSD_or_GPRS_for_Connection_Mode(void);
	bool PDPContextActivation(bool IsActive);
	bool SetMUX(bool isEnableMUX);
	bool CreateConnectMutipleMode(char* ipAddr, short port, Socket_Type socketType, signed char muxID = -1);
	bool Connect(char* ipAddr, short port, Socket_Type socketType, Socket_Protocol socketProtocol = Socket_Protocol_IPV4);

	unsigned int Read(char* data);
	unsigned int Read(char* data, unsigned int num);
	bool Write(char* data, unsigned int num);
	bool SendMultipleMode(char* data, unsigned int num, signed char muxID = -1);
	//**********Staus*********
	bool IsAlive(void);
	bool Close(signed char muxID = -1);
	u8 GetCCSQ(void);
	u8 GetCCSQVALUE(void);
	bool IsReged(void);
	void SetTimeOut(float timetOut);
	char GetStatus(char* muxID = 0, char* type = 0, char* ipAddr = 0, short remotePort = 0, short localPort = 0);
	void GetTCPStatus(void);
	//*********Call**********
	bool IsPin(void);
	bool Call(char* phone);
	bool SetMsgMode(bool IsText = true);
	bool SetTECharset(char* charset = (char*)"GSM");
	bool SendMsg(char* phone, char* msg, bool IsPDU = false, u8 length = 0);

	//****************** 应用于兆欧表
	bool SendTagData(const char* data, u8 num);
	void CheckStatus();
	//****************** 应用于兆欧表
};

#endif
