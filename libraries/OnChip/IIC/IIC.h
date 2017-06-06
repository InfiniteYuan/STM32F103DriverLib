#ifndef _IIC_H_
#define _IIC_H_
	
#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "FIFOBuffer.h"

//i2c command type
enum IIC_CMD_Type
{
	OUT2IN   = 0x01,	//read one byte from given register of slave
	IN2OUT  = 0x02,	//read multi bytes from continue registers of slave
};

//data structure of one i2c command
struct IIC_Command
{
	IIC_CMD_Type cmdType; //type of i2c commmand
	u8 slaveAddr;		      //slave address
	u8 DataOut[5];	      //data send to slave
	u8 outDataLen;	      //number of bytes send to slave
	u8* pDataIn;		      //ponter to receive data from slave
	u8 inDataLen;         //number of bytes to recieve from slave
};

//class type for IIC bus
class IIC
{
private:
	I2C_TypeDef* _i2c;        //the device pointer of i2c
	u32 _i2c_speed;           //speed of i2c bus
 	u16 _scl_pin;             //gpio pin of scl
	u16 _sda_pin;             //gpio pin of sda
	u32 _i2c_rcc;             //rcc of gpio for i2c pin
	u8 _gpio_remap;           //flag of gpio pin remap
	bool _healthy;            //healthy of i2c (master or slave)
	u32 _evt_irq_cnt;         //i2c event interrupt counter in one i2c command, if larger than a given number, reset i2c
//interrupt setting
	u8 _evt_irqn;       //i2c event IRQn
  u8 _err_irqn;       //i2c error IRQn
	u8 _evt_subprio;    //i2c event interrupt sub priority
	u8 _evt_preprio;    //i2c event interrupt preemption priority
	u8 _err_subprio;    //i2c error interrupt sub priority
	u8 _err_preprio;    //i2c error interrupt preemption priority

//i2c command related varible
  FIFOBuffer<IIC_Command,50> _i2c_cmd_queue;  //i2c command queue
	IIC_Command _current_cmd;  //current i2c command
	u8 _i2c_direction; //current i2c direction (send or receive)
	u8 _i2c_tx_cnt;    //i2c transmit bytes counter within one i2c command
	u8 _i2c_rx_cnt;    //i2c receive bytes counter  within one i2c command  

private:
	void InitGPIO(void);      //initialize gpio of i2c
  void SetDefaultGPIO(void);//set gpio as default io
	void Check_Busy_Fix(void); //check the BUSY bit of i2c device, and fix it if BUSY=1
public:
	///constructor, for i2c initialize
	///@param i2c:   i2c select
	///@param speed: speed of i2c bus
	///@param remap: gpio remap flag
	IIC(I2C_TypeDef* i2c = I2C1, u32 speed = 400000, u8 remap = 0);  //construct function
	void Initialize(void);    //initilize i2c device
  bool StartNextCommand(void);      //transfer command from iic_cmd_queue to iic_cmd_current	
	void Reset(void);      //reset i2c bus
	bool AddCommand(u8 slaveAddr, IIC_CMD_Type cmdType, u8 *pTxData, u8 txNum, u8 *pRxData, u8 rxNum);//add command to i2c command queue
	void ClearCommand();  //clear all i2c command in queue
	void EventIRQ(void);     //i2c event interrupt handler
  void ErrorIRQ(void);     //i2c error interrupt handler
	bool IsWorkingAndFree();
};	


#endif

