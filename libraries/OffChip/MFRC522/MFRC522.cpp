#include "MFRC522.h"

unsigned char  DefaultKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

MFRC522::MFRC522(USART &usart) :mUsart(usart)
{

}

MFRC522::~MFRC522()
{

}

void MFRC522::WaitCardOff(void)
{
	char          status;
	unsigned char	TagType[2];

	while (1)
	{
		status = PcdRequest(REQ_ALL, TagType);
		if (status)
		{
			status = PcdRequest(REQ_ALL, TagType);
			if (status)
			{
				status = PcdRequest(REQ_ALL, TagType);
				if (status)
				{
					return;
				}
			}
		}
		tskmgr.DelayMs(100);
	}
}

void MFRC522::WriteRCReg(unsigned char Address, unsigned char value)
{
	unsigned char address = Address & 0x7f;			//最高位为0，写模式，0~5位为地址
	unsigned char temp = 0;

	mUsart.SendBytes(&address, 1);						//发送需要写寄存器的地址

	tskmgr.DelayMs(5);									//由于采用中断发送，需延时同步

	if (mUsart.RxSize() > 0)
	{
		mUsart.GetBytes(&temp, 1);
	}

	mUsart.SendBytes(&value, 1);							//发送数据

}

unsigned char MFRC522::ReadRCReg(unsigned char Address)
{
	unsigned char value = 0;
	unsigned char address = Address | 0x80;	//最高位为1，读模式，0~5位为地址

	mUsart.ClearReceiveBuffer();

	mUsart.SendBytes(&address, 1);						//发送需要读取的寄存器的地址

	//	while(!mUsart.GetReceivedData(&value,1));			//等待接收数据

	tskmgr.DelayMs(5);

	if (mUsart.RxSize() > 0)
		mUsart.GetBytes(&value, 1);
	return value;

}

//软件CRC16生成函数
unsigned short MFRC522::SoftWareCrc16(unsigned char *buf, int len)
{
	unsigned short crc = 0xffff;
	int i_byte = 0, i_bit = 0;
	unsigned char t = 0;
	for (i_byte = 0; i_byte < len; i_byte++)
	{
		crc = crc ^ buf[i_byte];
		for (i_bit = 0; i_bit < 8; i_bit++)
		{
			t = crc & 0x0001;
			crc >>= 1;
			crc &= 0x7fff;
			if (t == 1)
				crc = crc ^ 0xa001;
		}
	}
	return crc;
}

//硬件CRC16生成函数
unsigned short MFRC522::HardWareCrc16(unsigned char *pIndata, int len)
{
	unsigned char i, n, crcL = 0, crcH = 0;
	ClearBitMask(DivIrqReg, 0x04);
	WriteRCReg(CommandReg, PCD_IDLE);
	SetBitMask(FIFOLevelReg, 0x80);
	for (i = 0; i < len; i++)
	{
		WriteRCReg(FIFODataReg, *(pIndata + i));
	}

	WriteRCReg(CommandReg, PCD_CALCCRC);		//写命令寄存器，开始计算CRC16

	i = 0xFF;
	do
	{
		n = ReadRCReg(DivIrqReg);
		i--;
	} while ((i != 0) && !(n & 0x04));				//等待crc校验码的生成

	crcL = ReadRCReg(CRCResultRegL);
	crcH = ReadRCReg(CRCResultRegM);

	return ((crcH << 8) + crcL);
}

RCState MFRC522::PcdReset(void)
{
	RCState Status = MI_OK;

	WriteRCReg(CommandReg, PCD_RESETPHASE);			//写命令寄存器，复位命令
	WriteRCReg(ModeReg, 0x3D);            			//和Mifare卡通讯，CRC初始值0x6363
	WriteRCReg(TReloadRegL, 30);
	WriteRCReg(TReloadRegH, 0);
	WriteRCReg(TModeReg, 0x8D);
	WriteRCReg(TPrescalerReg, 0x3E);
	WriteRCReg(TxAskReg, 0x40);

	return Status;
}



void MFRC522::SetBitMask(unsigned char reg, unsigned char mask)
{
	char tmp = 0x00;
	tmp = ReadRCReg(reg);
	WriteRCReg(reg, tmp | mask);  // set bit mask
}

void MFRC522::ClearBitMask(unsigned char reg, unsigned char mask)
{
	char tmp = 0x00;
	tmp = ReadRCReg(reg);
	WriteRCReg(reg, tmp & ~mask);  // clear bit mask
}

//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
void MFRC522::PcdAntennaOn()
{
	unsigned char i;
	i = ReadRCReg(TxControlReg);
	if (!(i & 0x03))
	{
		SetBitMask(TxControlReg, 0x03);
	}
}


//关闭天线
void MFRC522::PcdAntennaOff()
{
	ClearBitMask(TxControlReg, 0x03);
}

//写数据到卡
RCState MFRC522::PcdWrite(unsigned char addr, unsigned char *pData)
{
	RCState status = MI_OK;
	unsigned short crc = 0;
	unsigned int  unLen;
	unsigned char i, ucComMF522Buf[MFRC522COM_BUF_SIZE];

	ucComMF522Buf[0] = PICC_WRITE;
	ucComMF522Buf[1] = addr;

	crc = HardWareCrc16(ucComMF522Buf, 2);

	ucComMF522Buf[2] = crc & 0xff;
	ucComMF522Buf[3] = crc >> 8;

	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

	if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
	{
		status = MI_ERR;
	}

	if (status == MI_OK)
	{
		for (i = 0; i < 16; i++)
		{
			ucComMF522Buf[i] = *(pData + i);
		}

		crc = HardWareCrc16(ucComMF522Buf, 16);

		ucComMF522Buf[16] = crc & 0xff;
		ucComMF522Buf[17] = crc >> 8;

		status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
		if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
		{
			status = MI_ERR;
		}
	}

	return status;
}

//从卡上读数据
RCState MFRC522::PcdRead(unsigned char addr, unsigned char *pData)
{
	RCState status = MI_OK;
	unsigned short crc = 0;
	unsigned int  unLen;
	unsigned char i, ucComMF522Buf[MFRC522COM_BUF_SIZE];

	ucComMF522Buf[0] = PICC_READ;
	ucComMF522Buf[1] = addr;

	crc = HardWareCrc16(ucComMF522Buf, 2);

	ucComMF522Buf[2] = crc & 0xff;
	ucComMF522Buf[3] = crc >> 8;

	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

	if ((status == MI_OK) && (unLen == 0x90))
	{
		for (i = 0; i < 16; i++)
		{
			*(pData + i) = ucComMF522Buf[i];
		}
	}
	else
	{
		status = MI_ERR;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
RCState MFRC522::PcdComMF522(unsigned char Command, unsigned char *pInData, unsigned char InLenByte, unsigned char *pOutData, unsigned int  *pOutLenBit)
{
	RCState status = MI_ERR;
	unsigned char irqEn = 0x00;
	unsigned char waitFor = 0x00;
	unsigned char lastBits;
	unsigned char n;
	unsigned int i;
	switch (Command)
	{
	case PCD_AUTHENT:				//认证命令
		irqEn = 0x12;
		waitFor = 0x10;
		break;
	case PCD_TRANSCEIVE:				//发送并接收数据命令
		irqEn = 0x77;
		waitFor = 0x30;
		break;
	default:;
	}

	WriteRCReg(ComIEnReg, irqEn | 0x80);		//使能相应中断
	ClearBitMask(ComIrqReg, 0x80);
	WriteRCReg(CommandReg, PCD_IDLE);		//无动作，或取消当前正在执行的命令
	SetBitMask(FIFOLevelReg, 0x80);

	for (i = 0; i < InLenByte; i++)
	{
		WriteRCReg(FIFODataReg, pInData[i]);
	}	//向FIFO数据缓冲区写数据

	WriteRCReg(CommandReg, Command);				//将相应命令写入命令寄存器

	if (Command == PCD_TRANSCEIVE)
	{
		SetBitMask(BitFramingReg, 0x80);
	}			//开始发送

	i = 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms
	do
	{
		n = ReadRCReg(ComIrqReg);				//读取中断标志寄存器
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n&waitFor));
	ClearBitMask(BitFramingReg, 0x80);				//清除启动发送数据标志

	if (i != 0)									//如果在等待M1的时间未超过一定量
	{
		if (!(ReadRCReg(ErrorReg) & 0x1B))		//读错误标志寄存器，如果FIFO未溢出或奇偶校验未出错，或协议出错
		{
			status = MI_OK;
			if (n & irqEn & 0x01)				//定时器超时，说明未检测到标签
			{
				status = MI_NOTAGERR;
			}
			if (Command == PCD_TRANSCEIVE)
			{
				n = ReadRCReg(FIFOLevelReg);
				lastBits = ReadRCReg(ControlReg) & 0x07;
				if (lastBits)
				{
					*pOutLenBit = (n - 1) * 8 + lastBits;
				}
				else
				{
					*pOutLenBit = n * 8;
				}

				if (n == 0)
				{
					n = 1;
				}

				if (n > MFRC522COM_BUF_SIZE)
				{
					n = MFRC522COM_BUF_SIZE;
				}

				for (i = 0; i < n; i++)
				{
					pOutData[i] = ReadRCReg(FIFODataReg);
				}
			}
		}
		else
		{
			status = MI_ERR;
		}

	}

	SetBitMask(ControlReg, 0x80);           // stop timer now
	WriteRCReg(CommandReg, PCD_IDLE);

	return status;
}


//功    能：寻卡，此时卡由Idle状态变为Ready状态
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡（WAKE-UP命令）
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
RCState MFRC522::PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
	RCState status = MI_OK;
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MFRC522COM_BUF_SIZE];

	ClearBitMask(Status2Reg, 0x08);		//清除成功进行认证后所置的位
	WriteRCReg(BitFramingReg, 0x07);
	SetBitMask(TxControlReg, 0x03);

	ucComMF522Buf[0] = req_code;

	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);

	if ((status == MI_OK) && (unLen == 0x10))
	{
		*pTagType = ucComMF522Buf[0];
		*(pTagType + 1) = ucComMF522Buf[1];
	}
	else
	{
		status = MI_ERR;
	}

	return status;
}

//功    能：防冲撞,此时获得卡片序列号
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
RCState MFRC522::PcdAnticoll(unsigned char *pSnr)
{
	RCState status = MI_OK;
	unsigned char i, snr_check = 0;
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MFRC522COM_BUF_SIZE];


	ClearBitMask(Status2Reg, 0x08);		//清除成功进行认证后所置的位
	WriteRCReg(BitFramingReg, 0x00);		//最后一个字节的所有位都应发送
	ClearBitMask(CollReg, 0x80);			//所有接收的位在冲突后将被清除

	ucComMF522Buf[0] = PICC_ANTICOLL1;
	ucComMF522Buf[1] = 0x20;	//该值定义了该 PCD 将不发送 UID CLn 的任何部分。因此该命令迫使工作场内的所有 PICC 以其完整的
	//UID CLn 表示响应

	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

	if (status == MI_OK)
	{
		for (i = 0; i < 4; i++)
		{
			*(pSnr + i) = ucComMF522Buf[i];
			snr_check ^= ucComMF522Buf[i];
		}
		if (snr_check != ucComMF522Buf[i])
		{
			status = MI_ERR;
		}
	}

	SetBitMask(CollReg, 0x80);
	return status;
}

//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
RCState MFRC522::PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
	RCState status = MI_OK;
	unsigned int  unLen;
	unsigned char i, ucComMF522Buf[MFRC522COM_BUF_SIZE];

	ucComMF522Buf[0] = auth_mode;
	ucComMF522Buf[1] = addr;
	for (i = 0; i < 6; i++)
	{
		ucComMF522Buf[i + 2] = *(pKey + i);
	}

	for (i = 0; i < 6; i++)
	{
		ucComMF522Buf[i + 8] = *(pSnr + i);
	}

	status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);

	if ((status != MI_OK) || (!(ReadRCReg(Status2Reg) & 0x08)))
	{
		status = MI_ERR;
	}

	return status;
}

//功    能：选定卡片,当卡接收到其完整的UID时进入激活状态
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
RCState MFRC522::PcdSelect(unsigned char *pSnr)
{
	RCState status = MI_OK;
	unsigned short crc = 0;
	unsigned char i;
	unsigned int  unLen = 0;
	unsigned char ucComMF522Buf[MFRC522COM_BUF_SIZE];

	ucComMF522Buf[0] = PICC_ANTICOLL1;
	ucComMF522Buf[1] = 0x70;
	ucComMF522Buf[6] = 0;
	for (i = 0; i < 4; i++)
	{
		ucComMF522Buf[i + 2] = *(pSnr + i);
		ucComMF522Buf[6] ^= *(pSnr + i);
	}

	crc = HardWareCrc16(ucComMF522Buf, 7);

	ucComMF522Buf[7] = crc & 0xff;
	ucComMF522Buf[8] = crc >> 8;

	ClearBitMask(Status2Reg, 0x08);

	status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

	if ((status == MI_OK) && (unLen == 0x18))
	{
		status = MI_OK;
	}
	else
	{
		status = MI_ERR;
	}

	return status;
}


void MFRC522::InitializeSystem()
{
	PcdReset();								//阅读器复位
	tskmgr.DelayMs(888);						//延时10ms

	PcdAntennaOff(); 						//天线关闭
	tskmgr.DelayMs(888);

	PcdAntennaOn(); 						//天线打开
	tskmgr.DelayMs(888);

}

RCState MFRC522::ReadyComunication(unsigned char req_code)
{
	RCState status = MI_OK;

	status = PcdRequest(req_code, mCardType);		//获得卡的类型
	if (status != MI_OK)
	{
//		return status;
	}
	status = PcdAnticoll(mSerialNum);				//获得卡的序列号

	if (status != MI_OK)
	{
		return status;
	}
//	status = PcdSelect(mSerialNum);   //选择卡进入激活状态

//	if (status != MI_OK)
//	{
//		return status;
//	}
//	status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, mSerialNum);//验证卡片密码

	return status;

}

//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
RCState MFRC522::PiccHalt(void)
{
	RCState Status = MI_OK;

	unsigned int  unLen = 0;
	unsigned char ucComMF522Buf[MFRC522COM_BUF_SIZE];
	unsigned short crc = 0;

	ucComMF522Buf[0] = PICC_HALT;
	ucComMF522Buf[1] = 0;

	crc = HardWareCrc16(ucComMF522Buf, 2);

	ucComMF522Buf[2] = crc & 0xff;
	ucComMF522Buf[3] = crc >> 8;


	Status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

	return Status;
}

void MFRC522::HardWareReset(GPIO &Reset)
{
	Reset.SetLevel(1);
	tskmgr.DelayMs(888);
	Reset.SetLevel(0);
	tskmgr.DelayMs(888);
	Reset.SetLevel(1);
	tskmgr.DelayMs(888);
}

unsigned char* MFRC522::GetmCardType()
{
	return mCardType;
}

unsigned char* MFRC522::GetmSerialNum()
{
	return mSerialNum;
}
