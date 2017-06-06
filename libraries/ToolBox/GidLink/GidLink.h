#ifndef _UID_LINK_H_
#define _UID_LINK_H_

#include "stm32f10x.h"
#include "FIFOBuffer.h"
#include "USART.h"


class UidLink
{
public:
	DataFrame txFrame;
	DataFrame rxFrame;
	USART &com;
public:
	UidLink(USART &uart);
	bool CheckFrame();
	bool Send();
};


#endif

