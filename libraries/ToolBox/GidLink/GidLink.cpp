#include "GidLink.h"




UidLink::UidLink(USART &uart):com(uart)
{
	
}

bool UidLink::CheckFrame()
{
	
	return com.CheckFrame(rxFrame);
}

bool UidLink::Send()
{
	if(!txFrame.isUpdated) //no new frame data, no need to send
		return false;
	if(!com.SendByte(txFrame.header))  //send frame header
		return false;
	if(txFrame.fnCode>MAX_FN_CODE || !com.SendByte(txFrame.fnCode))  //send function code
		return false;
	txFrame.dataLength = DATA_LENGTH[txFrame.fnCode][DIRECTION_SEND];
	if(!com.SendByte(txFrame.dataLength))  //send data length
		return false;
	if(!com.SendBytes(txFrame.data,txFrame.dataLength)) //send data;
		return false;
	txFrame.CreateCheckCode();
	if(!com.SendByte(txFrame.checkSum))    //send check code
		return false;
	txFrame.isUpdated = false;
	return true;
}







