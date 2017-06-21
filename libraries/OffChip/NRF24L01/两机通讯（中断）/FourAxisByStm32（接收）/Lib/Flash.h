#ifndef  __FLASH_H
#define  __FLASH_H

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#define MEMORY_PAGE_SIZE  (uint16_t)0x400  /* Page size = 1KByte */


/* Page status definitions */
#define MEMORY_STATUS_ERASED                  ((uint16_t)0xFFFF)     /* PAGE is empty */
#define MEMORY_STATUS_RECEIVE_DATA            ((uint16_t)0xEEEE)     /* PAGE is marked to receive data */
#define MEMORY_STATUS_VALID_PAGE              ((uint16_t)0x0000)     /* PAGE containing valid data */


#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)		) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )



class flash
{
private:
	
	bool mUseHalfWord;//储存字长  16/32 默认16位
	uint32_t mStartAddress;//开始储存的地址
public:
///////////////////
///构造函数
///@param startAddress 存储数据的开始位置 默认：0x08010000 即63k byte 处
///@param useHalfWord 单个数据储存的长度 true:16位 false:32位  默认：16位
///////////////////
flash(uint32_t startAddress=(0x08000000+63*MEMORY_PAGE_SIZE),bool useHalfWord=true);


///////////////////////
///读取储存器中特定位置的值
///@param -relativeAddress 相对于开始地址的地址
///@param -Data 读出的数据存放的地址
///@retval -1 : 读取成功 -0：读取失败
///////////////////////
bool Read(uint16_t relativeAddress, uint8_t* Data,u16 length);
		
//position为一页中的位置，只能是偶数或者0
bool Read(u16 pageNumber,u16 position,u16* data,u16 length);


//bool Read(uint16_t relativeAddress, uint32_t* Data,u16 length);
		
///////////////////////
///向储存器中特定位置写值
///@param -pageNumber 相对于开始地址的页地址
///@param -Data 将要写入的数据
///@attention 如果构造构造函数的参数useHalfWord为true时，会现将其转换为u16再储存，否则会转换成u32再储存
///@retval -1 : 写入成功 -0：写入失败
///////////////////////
bool Write(uint16_t pageNumber, uint8_t* Data,u16 length);
		
//position为一页中的位置，只能是偶数或者0	,当传入0的时候将擦除这一页	
bool Write(uint16_t pageNumber,u16 position,uint16_t* data,u16 length);

bool Write(uint16_t pageNumber,u16 position,float data);
float Read(uint16_t pageNumber,u16 position);		

bool Write(uint16_t pageNumber, uint32_t* Data,u16 length);
		

//字符串的存储于读取,参数：页码，页码中位置，字符串
bool Write(uint16_t pageNumber,u16 position,char* str);
bool Read(uint16_t pageNumber,u16 position,char *str);
bool Clear(uint16_t pageNumber);


};


#endif

