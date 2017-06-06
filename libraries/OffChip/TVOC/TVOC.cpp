#include "TVOC.h"

TVOC::TVOC(USART &uart):com(uart)
{
	mValue = 0.0;
}

char TVOC::sum(char *data, u8 num)
{
	u8 i = 0, result = 0;
	for(i = 0; i < num; i++)
	{
		result += data[i];
	}
	return result;
}

void TVOC::Update(void)
{
	u8 temp[9] = {0};
	u8 CheckSum = 0;
	u16 tempResult = 0.0;
	if(com.RxSize() >= 9)
	{
		com.GetBytes(temp, 9);
		CheckSum = sum((char*)temp, 9);
		if((temp[0] == 0xFF) && (temp[1] == 0xFF) && (CheckSum == temp[8]))
		{
			tempResult = (temp[4] * 256 + temp[5]);
			if(tempResult != 0){
				mValue = (temp[4] * 256 + temp[5]);
				if(temp[6] != 0x00)
					mValue = mValue / (temp[6] * 10.0);
				mValue = mValue * temp[7];
			}
		}
	}
}

float TVOC::GetValue(void)
{
	return mValue;
}
