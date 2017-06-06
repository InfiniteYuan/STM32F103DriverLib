#include "DPPrint.h"
//#include "DP_Print_inc.h"
#include <string.h>

DPPrint::DPPrint(USART &usar) :com(usar)
{

}
void DPPrint::InitializePrint(void)
{
	u8 data[2];
	data[0] = 0x1B;
	data[1] = 0x40;
	com << data;
}
/**
* 1A 5B 00
* 打印页面宽384点，高1200点，不旋转，参考点为当前位置左上角
*/
void DPPrint::PrintPageStart(void)
{
	u8 data[3];
	data[0] = 0x1A;
	data[1] = 0x5B;
	data[2] = 0x00;
	com << data;
}
/**
*1A 5B 01 x_L x_H y_L y_H Width_L width_H Height_L Height_H Rotate 1点 = 0.125mm
*/
void DPPrint::PrintPageStart(uint16_t off_x, uint16_t off_y, uint16_t width, uint16_t height, bool IsRotate)
{
	u8 data[12];//打印页面宽width点，高heigth点，旋转，参考点
	data[0] = 0x1A;
	data[1] = 0x5B;
	data[2] = 0x01;
	data[3] = off_x % 256;
	data[4] = off_x / 256;
	data[5] = off_y % 256;
	data[6] = off_y / 256;
	data[7] = width % 256;
	data[8] = width / 256;
	data[9] = height % 256;
	data[10] = height / 256;
	data[11] = (char)IsRotate;
	com.SendBytes(data, 12);
}
/**
*1A 5D 00
*/
void DPPrint::PrintPageEnd(void)
{
	u8 data[3];
	data[0] = 0x1A;
	data[1] = 0x5D;
	data[2] = 0x00;
	com.SendBytes(data, 3);
}
/**
*1A 4F 01 PrintNum
*/
void DPPrint::PagePrint(u8 num)
{
	u8 data[4];
	data[0] = 0x1A;
	data[1] = 0x4F;
	data[2] = 0x01;
	data[3] = (char)num;
	com.SendBytes(data, 4);
}
/**
*1A 0C 01 StopPosition Offset_L Offset_H
*/
void DPPrint::PaperRun(u8 StopType, uint16_t Offset)
{
	u8 data[6];
	data[0] = 0x1A;
	data[1] = 0x0C;
	data[2] = 0x01;
	data[3] = (char)StopType;
	data[4] = Offset % 256;
	data[5] = Offset / 256;
	com.SendBytes(data, 6);
}
/**
* 1A 54 01 x_L x_H y_L y_H FontHeight_L FontHeight_H FontType_L FontType_H String00
*FontType 数据位					定义
*						0					加粗标志位
*						1					下划线标志位
*						2					反白标志位
*						3					删除线标志位
*					[5,4]				旋转标志位，00旋转0；01旋转90；10旋转180； 11旋转270
*					[11,8]			字体宽度放大倍数
*					[15,12]			字体高度放大倍数
*/
void DPPrint::TextDraw(uint16_t start_x, uint16_t start_y, uint16_t fontHeight, uint16_t fontType, char *string)
{
	u8 data[11];
	char *sendstring;
	data[0] = 0x1A;
	data[1] = 0x54;
	data[2] = 0x01;
	data[3] = start_x % 256;
	data[4] = start_x / 256;
	data[5] = start_y % 256;
	data[6] = start_y / 256;
	data[7] = fontHeight % 256;
	data[8] = fontHeight / 256;
	data[9] = fontType % 256;
	data[10] = fontType / 256;
	com.SendBytes(data, 11);
	strcpy(sendstring, string);
	com << string;
	data[0] = 0x00;
	com.SendBytes(data, 1);
}
/**
*1A 5C 01 StartX_L StartX_H StartY_L StartY_H EndX_L EndX_H EndY_L EndY_H Width_L Width_H Color
*/
void DPPrint::lineDraw(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint16_t width, char color)
{
	u8 data[14];
	data[0] = 0x1A;
	data[1] = 0x5C;
	data[2] = 0x01;
	data[3] = start_x % 256;
	data[4] = start_x / 256;
	data[5] = start_y % 256;
	data[6] = start_y / 256;
	data[7] = end_x % 256;
	data[8] = end_x / 256;
	data[9] = end_y % 256;
	data[10] = end_y / 256;
	data[11] = width % 256;
	data[12] = width / 256;
	data[13] = color;
	com.SendBytes(data, 14);
}
/**
*1A 26 01 Left_L Left_H Top_L Top_H Right_L Right_H Bottom_L Bottom_H Width_L Width_H Color
*/
void DPPrint::rectangleDraw(uint16_t left_x, uint16_t left_y, uint16_t right_x, uint16_t right_y, uint16_t width, char color)
{
	u8 data[14];
	data[0] = 0x1A;
	data[1] = 0x26;
	data[2] = 0x01;
	data[3] = left_x % 256;
	data[4] = left_x / 256;
	data[5] = left_y % 256;
	data[6] = left_y / 256;
	data[7] = right_x % 256;
	data[8] = right_x / 256;
	data[9] = right_y % 256;
	data[10] = right_y / 256;
	data[11] = width % 256;
	data[12] = width / 256;
	data[13] = color;
	com.SendBytes(data, 14);
}
/**
*1A 2A 00 Left_L Left_H Top_L Top_H Right_L Right_H Bottom_L Bottom_H Color
*/
void DPPrint::rectangleBlockDraw(uint16_t left_x, uint16_t left_y, uint16_t right_x, uint16_t right_y, char color)
{
	u8 data[12];
	data[0] = 0x1A;
	data[1] = 0x2A;
	data[2] = 0x00;
	data[3] = left_x % 256;
	data[4] = left_x / 256;
	data[5] = left_y % 256;
	data[6] = left_y / 256;
	data[7] = right_x % 256;
	data[8] = right_x / 256;
	data[9] = right_y % 256;
	data[10] = right_y / 256;
	data[11] = color;
	com.SendBytes(data, 12);
}

void DPPrint::jump_lattice(void)
{
	jump_lattice();
}

void DPPrint::print_And_Line(void)
{
	print_And_Line();
}

void DPPrint::Print_ASCII(unsigned char *Str, unsigned char StrLen)
{
	Print_ASCII(Str, StrLen);
}

void DPPrint::Set_Right_Interval(unsigned char interval)
{
	Set_Right_Interval(interval);
}

void DPPrint::Set_Print_Mode(unsigned char optbit)
{
	Set_Print_Mode(optbit);
}

void DPPrint::Set_Print_Position(unsigned char Lpos, unsigned char Hpos)
{
	Set_Print_Position(Lpos, Hpos);
}

void DPPrint::Set_Left_Interval(unsigned char Interval)
{
	Set_Left_Interval(Interval);
}

void DPPrint::Sel_Custom_Character(unsigned char SelOpt)
{
	Sel_Custom_Character(SelOpt);
}

void DPPrint::UserDefineCharacter(unsigned char yByte, unsigned char xDot, unsigned char DefChar, unsigned char *pData)
{
	UserDefineCharacter(yByte, xDot, DefChar, pData);
}

void DPPrint::Sel_Bitmap_Mode(unsigned char mode, unsigned char DatLenLowByte, unsigned char DatLenHightByte, unsigned char *pDotData)
{
	Sel_Bitmap_Mode(mode, DatLenLowByte, DatLenHightByte, pDotData);
}

void DPPrint::Set_UnderLine(unsigned char Opt)
{
	Set_UnderLine(Opt);
}

void DPPrint::SetDefaultLineInterval(void)
{
	SetDefaultLineInterval();
}

void DPPrint::Del_UserDefineCharacter(unsigned char SelCharacter)
{
	Del_UserDefineCharacter(SelCharacter);
}

void DPPrint::SetHorizPosition(unsigned char Position)
{
	SetHorizPosition(Position);
}

void DPPrint::SetBold(unsigned char opt)
{
	SetBold(opt);
}

void DPPrint::PrintGoPage(unsigned char nstep)
{
	PrintGoPage(nstep);
}

void DPPrint::SelCountryCharacter(unsigned char nsel)
{
	SelCountryCharacter(nsel);
}

void DPPrint::Set_Rotate(unsigned char nsel)
{
	Set_Rotate(nsel);
}

void DPPrint::Get_Print_State(void)
{
	Get_Print_State();
}

void DPPrint::Sel_Align_Way(unsigned char nsel)
{
	Sel_Align_Way(nsel);
}

void DPPrint::SetReversal(unsigned char nsel)
{
	SetReversal(nsel);
}

void DPPrint::SetCharacterSize(unsigned char width, unsigned char hight)
{
	SetCharacterSize(width, hight);
}

void DPPrint::DownLoadBitmap(unsigned char xDot, unsigned char yDot, unsigned char *pDat)
{
	DownLoadBitmap(xDot, yDot, pDat);
}

void DPPrint::PrintDownLoadBitmap(unsigned char mode)
{
	PrintDownLoadBitmap(mode);
}

void DPPrint::Set_Inverse(unsigned char opt)
{
	Set_Inverse(opt);
}

void DPPrint::Set_LeftSpaceNum(unsigned char nL, unsigned char nH)
{
	Set_LeftSpaceNum(nL, nH);
}

void DPPrint::Set_HRIPosition(unsigned char opt)
{
	Set_HRIPosition(opt);
}

void DPPrint::Set_BarCodeHight(unsigned char verticalDotNum)
{
	Set_BarCodeHight(verticalDotNum);
}

void DPPrint::Set_BarCodeLeftSpace(unsigned char SpaceNum)
{
	Set_BarCodeLeftSpace(SpaceNum);
}

void DPPrint::Set_BarCodeWidth(unsigned char widthsel)
{
	Set_BarCodeWidth(widthsel);
}

void DPPrint::PrintBarCode(unsigned char CodeType, unsigned char *pData, unsigned char pDataLen)
{
	PrintBarCode(CodeType, pData, pDataLen);
}

void DPPrint::SetChinesemode(unsigned char opt)
{
	SetChinesemode(opt);
}

void DPPrint::SelChineseChar(void)
{
	SelChineseChar();
}

void DPPrint::DisChineseChar(void)
{
	DisChineseChar();
}

void DPPrint::Set_ChineseCode(unsigned char selopt)
{
	Set_ChineseCode(selopt);
}

void DPPrint::TestPrintPage(void)
{
	TestPrintPage();
}

void DPPrint::PrintGratinmap(unsigned char mode, unsigned int xDot, unsigned int yDot, unsigned char *pData)
{
	PrintGratinmap(mode, xDot, yDot, pData);
}

void DPPrint::Set_QRcodeMode(unsigned char mode)
{
	Set_QRcodeMode(mode);
}

void DPPrint::Set_QRCodeAdjuLevel(unsigned char level)
{
	Set_QRCodeAdjuLevel(level);
}

void DPPrint::Set_QRCodeBuffer(unsigned int Len, unsigned char *pData)
{
	Set_QRCodeBuffer(Len, pData);
}

void DPPrint::PrintQRCode(void)
{
	PrintQRCode();
}

void DPPrint::TagPrint(TagData mTagData)
{
	char tempDate[20] = {0};
	char year[5] = {0};
	char monthday[6] = {0};
	strcpy(tempDate, mTagData.DateTIME);
	char * index = strstr(tempDate, "-");
	index[0] = '\0';
	index = strstr(&tempDate[5], ",");
	index[0] = '\0';
	strcpy(year, tempDate);
	strcpy(monthday, &tempDate[5]);
	InitializePrint();
	PrintPageStart(0, 0, 384, 640, 0);
	rectangleDraw(0, 0, 384, 576, 5, 1);

	lineDraw(96, 96, 384, 96, 5, 1);
	lineDraw(0, 192, 384, 192, 5, 1);
	lineDraw(0, 288, 384, 288, 5, 1);
	lineDraw(96, 384, 384, 384, 5, 1);
	lineDraw(96, 480, 384, 480, 5, 1);
	lineDraw(96, 0, 96, 576, 5, 1);
	lineDraw(192, 0, 192, 576, 5, 1);
	lineDraw(288, 0, 288, 576, 5, 1);
	
	TaskManager::DelayMs(300);  //***********等待发送**********

	TextDraw(357, 5, 32, 8465, (char *)" 车 次\0");
	TextDraw(357, 117, 32, 8465, mTagData.TrainNum);
	TextDraw(357, 197, 32, 8465, (char *)" 时 间\0");
	TextDraw(357, 315, 32, 4369, year);
	TextDraw(325, 309, 32, 4369, monthday);
//	TextDraw(357, 309, 32, 8465, mTagData.DateTIME);
	TextDraw(357, 389, 32, 8465, (char *)" 湿 度\0");
	TextDraw(357, 501, 32, 8465, mTagData.HUMIDTY);

	TextDraw(272, 21, 32, 4369, (char *)"D600+\0");
	TextDraw(272, 117, 32, 4369, (char *)"D600-\0");
	TextDraw(272, 218, 32, 4369, (char *)"D+/\0");
	TextDraw(272, 309, 32, 4369, (char *)"D110+\0");
	TextDraw(272, 405, 32, 4369, (char *)"D110-\0");
	
	TaskManager::DelayMs(300);  //***********等待发送**********
	
	TextDraw(240, 32, 32, 4369, (char *)"/地\0");
	TextDraw(240, 128, 32, 4369, (char *)"/地\0");
	TextDraw(240, 224, 32, 4369, (char *)"D-\0");
	TextDraw(240, 320, 32, 4369, (char *)"/地\0");
	TextDraw(240, 416, 32, 4369, (char *)"/地\0");

	TextDraw(168, 26, 32, 8465, mTagData.CH_MEA[CH_D600P]);
	TextDraw(168, 122, 32, 8465, mTagData.CH_MEA[CH_D600N]);
	TextDraw(168, 218, 32, 8465, mTagData.CH_MEA[CH_D110P]);
	TextDraw(168, 314, 32, 8465, mTagData.CH_MEA[CH_D110N]);
	TextDraw(168, 410, 32, 8465, mTagData.CH_MEA[CH_DPN]);
	
	TaskManager::DelayMs(300);  //***********等待发送**********

	TextDraw(80, 5, 32, 8465, (char *)" 单 位\0");
	TextDraw(80, 101, 32, 8465, (char *)"(兆欧)\0");
	TextDraw(80, 202, 32, 8465, (char *)"测试人\0");
	char people[50] = { '\0' };
	if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") == 0))
		sprintf(people, "%s", mTagData.PeoPle[PeoPleCh1]);
	else if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") != 0))
		sprintf(people, "%s、%s", mTagData.PeoPle[PeoPleCh2], mTagData.PeoPle[PeoPleCh3]);
	else if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") != 0))
		sprintf(people, "%s", mTagData.PeoPle[PeoPleCh3]);
	else if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") == 0))
		sprintf(people, "%s", mTagData.PeoPle[PeoPleCh2]);
	else if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") == 0))
		sprintf(people, "%s、%s", mTagData.PeoPle[PeoPleCh1], mTagData.PeoPle[PeoPleCh2]);
	else if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") != 0))
		sprintf(people, "%s、%s", mTagData.PeoPle[PeoPleCh1], mTagData.PeoPle[PeoPleCh3]);
	else if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") == 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") == 0))
		sprintf(people, "%s", mTagData.PeoPle[PeoPleCh1]);
	else if ((strcmp(mTagData.PeoPle[PeoPleCh1], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh2], "") != 0) && (strcmp(mTagData.PeoPle[PeoPleCh3], "") != 0))
		sprintf(people, "%s、%s、%s", mTagData.PeoPle[PeoPleCh1], mTagData.PeoPle[PeoPleCh2], mTagData.PeoPle[PeoPleCh3]);
	TextDraw(80, 298, 32, 8465, people);

	PrintPageEnd();
	PagePrint(0x01);
}
