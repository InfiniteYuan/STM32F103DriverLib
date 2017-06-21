#ifndef _MFRC522_H
#define _MFRC522_H

#include "stm32f10x.h"
#include "USART.h"
#include "GPIO.h"
#include "TaskManager.h"


/***	Define	***/
#define	MFRC522COM_BUF_SIZE	20

/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算

/////////////////////////////////////////////////////////////////////
//M1卡共16个扇区（64个块），每个块16字节，以块为存取单位
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞（串联级别1）
#define PICC_ANTICOLL2        0x95               //防冲撞（串联级别2）
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01    
#define     ComIEnReg             0x02    
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAskReg              0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  		  			0x3F

#define     REQ_ALL               0x52
#define     KEYA                  0x60

/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////

typedef enum
{
	MI_OK,				//正常
	MI_NOTAGERR,		//未读到标签
	MI_ERR				//其它错误
}RCState;


class MFRC522
{
private:
	USART &mUsart;

	unsigned char mCardType[2];		//用于保存卡的类型
	unsigned char mSerialNum[4];	//保存卡的序列号

public:


	MFRC522(USART &usart);

	~MFRC522();

	/**
	 *@brief 写RC寄存器
	 *@param Address 寄存器地址
	 *@param value 要写入的值
	 */
	void WriteRCReg(unsigned char Address, unsigned char value);

	/**
	 *@brief 读RC寄存器
	 *@param Address 寄存器地址
	 *@retval 读出的值
	 */
	unsigned char ReadRCReg(unsigned char Address);

	/**
	  *@brief 复位RC522
	  */
	RCState PcdReset(void);


	/**
	  *@brief 命令卡进入休眠命令
	  */
	RCState PiccHalt(void);
	
	/**
		*
		*/
	void WaitCardOff(void);
	
	/**
	  *@brief 写数据到卡
	  *@param blockAddr 要写入的块地址
	  *@param *pData 数据指针，将该串数据写入到相应的数据块中
	  */
	RCState PcdWrite(unsigned char blockAddr, unsigned char *pData);

	/**
	  *@brief 从卡上读数据
	  *@param blockAddr 要写入的块地址
	  *@param *pData 数据指针，将所读到的数据放入该指针指示的空间
	  */
	RCState PcdRead(unsigned char blockAddr, unsigned char *pData);

	/**
	  *@brief 寻卡
	  *@param req_code[IN]:寻卡方式
	  0x52 = 寻感应区内所有符合14443A标准的卡
	  0x26 = 寻未进入休眠状态的卡

	  *@param pTagType[OUT]：卡片类型代码
	  0x4400 = Mifare_UltraLight
	  0x0400 = Mifare_One(S50)
	  0x0200 = Mifare_One(S70)
	  0x0800 = Mifare_Pro(X)
	  0x4403 = Mifare_DESFire
	  */
	RCState PcdRequest(unsigned char req_code, unsigned char *pTagType);


	/**
	  *@brief 防碰撞
	  *@param pSnr:卡片序列号，4字节
	  */
	RCState PcdAnticoll(unsigned char *pSnr);

	/**
	  *@brief 选定卡片
	  *@param pSnr:卡片序列号，4字节
	  */
	RCState PcdSelect(unsigned char *pSnr);

	/**
	  *@brief 验证卡片密码
	  *@param auth_mode:密码验证模式,0x60 = 验证A密钥, 0x61 = 验证B密钥
	  *@param addr:块地址
	  *@param pKey:密码
	  *@param pSnr:卡片序列号，4字节
	  */
	RCState PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr);


	/**
	  *@brief 软件CRC16生成函数
	  *@param *buf 数据指针
	  *@param len  数据长度
	  */
	unsigned short SoftWareCrc16(unsigned char *buf, int len);

	/**
	  *@brief 硬件CRC16生成函数
	  *@param *pIndata 数据指针
	  *@param len  数据长度
	  */
	unsigned short HardWareCrc16(unsigned char *pIndata, int len);

	/**
	  *@brief 置屏蔽位
	  *@param reg 寄存器
	  *@param mask 屏蔽位
	  */
	void SetBitMask(unsigned char reg, unsigned char mask);

	/**
	  *@brief 清屏蔽位
	  *@param reg 寄存器
	  *@param mask 屏蔽位
	  */
	void ClearBitMask(unsigned char reg, unsigned char mask);

	/**
	  *@brief 开启天线
	  *
	  */
	void PcdAntennaOn();

	/**
	  *@brief 关闭天线
	  *
	  */
	void PcdAntennaOff();

	/**
	  *@brief 通过RC522和ISO14443卡通讯
	  *@param Command[IN]:RC522命令字
	  *@param pInData[IN]:通过RC522发送到卡片的数据
	  *@param InLenByte[IN]:发送数据的字节长度
	  *@param pOutData[OUT]:接收到的卡片返回数据
	  *@param *pOutLenBit[OUT]:返回数据的位长度
	  */
	RCState PcdComMF522(unsigned char Command, unsigned char *pInData, unsigned char InLenByte, unsigned char *pOutData, unsigned int  *pOutLenBit);

	/**
	  *@brief 初始化系统
	  *
	  */
	void InitializeSystem();

	/**
	  *@brief 准备启动系统
	  *@param req_code: (1)PICC_REQIDL			//请求所有未休眠的卡
	  *					(2)PICC_REQALL			//请求所有卡（包括已经睡眠的）
	  */
	RCState ReadyComunication(unsigned char req_code);

	/**
	  *@brief 获得卡类型
	  *
	  */
	unsigned char* GetmCardType();

	/**
	  *@brief 获得卡序列号
	  *
	  */
	unsigned char* GetmSerialNum();

	/**
	  *@brief 硬件复位
	  *
	  */
	void HardWareReset(GPIO &Reset);

};
#endif
