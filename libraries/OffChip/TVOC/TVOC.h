#ifndef _TVOC_H_
#define _TVOC_H_

#include "stm32f10x.h"
#include "FIFOBuffer.h"
#include "USART.h"

#define DHT12_ADDRESS   0xB8

class TVOC{

private:
	USART &com;
	float mValue;
public:
	TVOC(USART &uart);
	void Update(void);
	float GetValue(void);
	char sum(char *data, u8 num);
};

#endif