#ifndef __MONITOR_H
#define __MONITOR_H

#include "USART.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "Protocol.h"
#include "TaskManager.h"

class Monitor{

private:
	u8 	count;
	USART &mCom;
	u8  temp[50];
	static const char FrameTail[4];
	float mTimeOut;

public:
	Monitor(USART &Com);

	CommandData DecodeCommand(void);
	bool ReadCommand(void);

	bool ReadDateTime(void);
	bool SetTemperature(char * data);
	bool SetHumidity(char * data);
	bool SetWifiSignal(u8 degree);
	bool SetWifiDataLogo(bool isData);
	bool SetGprsSignal(u8 degree);
	bool SetGPRSDataLogo(bool isData);
	bool SetUSBLogo(bool isCome);
	bool SetBattery(u8 value);

	/**** 界面相关 ****/
	bool RedirectTrainSelect(void);
	bool RedirectMain(void);
	bool RedirectInit(void);
	bool RedirectWIFI(void);
	bool RedirectGPRS(void);
	bool RedirectMeasure(void);
	bool RedirectPrint(void);
	bool HidePrinting(void);
	bool ComePrinting(void);
	bool DisableCancelBtn(void);
	bool EnableCancelBtn(void);
	bool StopAnimation(void);//停止测量的进度条
	bool SetChargeIcon(bool showFlag = false);//设置充电图标

	bool SendResidueTrain(u8 value, u8 pagesum);

	bool SetMainTrain(char *data);
	bool SetChTrain(u8 channel, char *data);
	bool SetChPeoPle(u8 channel, char *data);
	bool SetChMeasure(u8 channel, char *data);
	bool SetSelectChMea1(void);

	//************************* 打印预览
	bool SetPrintTrain(char * data);
	bool SetPrintDate(char * data);
	bool SetPrintHum(char * data);
	bool SetPrintD600P(char * data);
	bool SetPrintD600N(char * data);
	bool SetPrintD110P(char * data);
	bool SetPrintD110N(char * data);
	bool SetPrintDPN(char * data);
	bool SetPrintPeoPle(char * data, char * data1, char * data2);

	//********** 设置
	bool SetWiFiSSID(char *data);
	bool SetWiFiKEY(char *data);
	bool SetWiFiIP(char *data);
	bool SetWiFiIPPort(char *data);
	bool SetGPRSIP(char *data);
	bool SetGPRSIPPort(char *data);
};

#endif
