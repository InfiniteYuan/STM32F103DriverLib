#include "IIC.h"
#include "Interrupt.h"

///constructor, for i2c initialize
///@param i2c:   i2c select
///@param speed: speed of i2c bus
///@param remap: gpio remap flag
IIC::IIC(I2C_TypeDef* i2c, u32 speed, u8 remap)
{
	_i2c = i2c;          //i2c device
	_i2c_speed = speed;  //speed of i2c bus
	_gpio_remap = remap; //remap flag for i2c gpio pin
	_evt_irq_cnt = 0;
	
	//set i2c pin, rcc, irq
	if(_i2c==I2C1) //I2C1
	{
		if(_gpio_remap==1) //remap gpio pin for i2c pin
		{
			_scl_pin = GPIO_Pin_8; //SCL pin
			_sda_pin = GPIO_Pin_9; //SDA pin
		}
		else if(_gpio_remap==0) //not remap gpio pin
		{
			_scl_pin = GPIO_Pin_6; //SCL pin
			_sda_pin = GPIO_Pin_7; //SDA pin
		}
		_i2c_rcc = RCC_APB1Periph_I2C1; //i2c clock
		_evt_irqn = I2C1_EV_IRQn;   //i2c event IRQn
		_err_irqn = I2C1_ER_IRQn;   //i2c error IRQn
		
		#ifdef USE_I2C1
		pI2C1 = this;
		#endif
	}
	else if(_i2c==I2C2)//I2C2
	{
		_scl_pin = GPIO_Pin_10; //SCL pin
		_sda_pin = GPIO_Pin_11; //SDA pin
		_i2c_rcc = RCC_APB1Periph_I2C2; //i2c clock
		_evt_irqn = I2C2_EV_IRQn;   //i2c event IRQn
		_err_irqn = I2C2_ER_IRQn;   //i2c error IRQn
		
		#ifdef USE_I2C2
		pI2C2 = this;
		#endif
	}
	//interrupt priority
	_evt_subprio = 0;    //i2c event interrupt sub priority
	_evt_preprio = 1;    //i2c event interrupt preemption priority
	_err_subprio = 0;    //i2c error interrupt sub priority
	_err_preprio = 0;    //i2c error interrupt preemption priority
	
	Initialize();
}



//GPIO Initialization for i2c pin
void IIC::InitGPIO()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//enable clock for i2c gpio pin
	
	if(_i2c==I2C1 && _gpio_remap==1)//I2C1 on gpio remap case
	{
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_AFIO),ENABLE); //Enable GPIO Alternative Functions
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);//Enable I2C1 pin Remap
	}
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//Set GPIO frequency to 50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;  //Select Output open-drain mode
  GPIO_InitStructure.GPIO_Pin = _scl_pin;          //Initialize I2Cx SCL Pin
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = _sda_pin;          //Initialize I2Cx SDA Pin
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
}

//set gpio as default io
void IIC::SetDefaultGPIO()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    //Set GPIO frequency to 50MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//Select Input floating mode
  GPIO_InitStructure.GPIO_Pin = _scl_pin;              //Default initialize I2Cx SCL Pin
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = _sda_pin;              //Default initialize I2Cx SDA Pin
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}



//check the BUSY bit of i2c device, and fix it if BUSY=1
void IIC::Check_Busy_Fix()
{
	u8 Time_out=0;
	GPIO_InitTypeDef GPIO_InitStructure;

	while(I2C_GetFlagStatus(_i2c, I2C_FLAG_BUSY) && Time_out<20)
	{
		RCC_APB1PeriphClockCmd(_i2c_rcc,DISABLE);           //disable i2c clock
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//enable gpio clock		
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //Set GPIO frequency to 50MHz
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //Select Output open-drain mode
		GPIO_InitStructure.GPIO_Pin = _scl_pin;             //Initialize I2Cx SCL Pin
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = _sda_pin;             //Initialize I2Cx SDA Pin
		GPIO_Init(GPIOB, &GPIO_InitStructure); 
		  
		//generate STOP command with direct gpio pin
		volatile u16 i;
		GPIO_ResetBits(GPIOB, _scl_pin);
		GPIO_ResetBits(GPIOB, _sda_pin);
		for(i=0;i<100;i++);
		GPIO_SetBits(GPIOB, _scl_pin);
		for(i=0;i<100;i++);
		GPIO_SetBits(GPIOB, _sda_pin);
		for(i=0;i<100;i++);
		
		SetDefaultGPIO();//set i2c pin as normal io pin
		RCC_APB1PeriphResetCmd(_i2c_rcc,ENABLE); //reset i2c clock
		RCC_APB1PeriphResetCmd(_i2c_rcc,DISABLE);//end i2c clock reseting
		RCC_APB1PeriphClockCmd(_i2c_rcc,ENABLE); //open i2c clock
		++Time_out;
	}
	
}

void IIC::Initialize()
{
	I2C_InitTypeDef I2C_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	_i2c->CR1 &= ~I2C_CR1_PE; // = I2C_Cmd(_i2c,DISABLE);  disable i2c device
	
	SetDefaultGPIO();//set i2c pin as normal io pin
	RCC_APB1PeriphResetCmd(_i2c_rcc,ENABLE); //reset i2c clock
	RCC_APB1PeriphResetCmd(_i2c_rcc,DISABLE);//end i2c clock reset
	RCC_APB1PeriphClockCmd(_i2c_rcc,DISABLE);//close i2c clock
		
	RCC_APB1PeriphResetCmd(_i2c_rcc,ENABLE); //reset i2c clock
	RCC_APB1PeriphResetCmd(_i2c_rcc,DISABLE);//end i2c clock reset
	RCC_APB1PeriphClockCmd(_i2c_rcc,ENABLE); //open i2c clock
	
	Check_Busy_Fix(); //check if sda is low, and fix it if ture
	
	InitGPIO(); //inilitialze gpio for i2c pin
	
	//_i2c->CR1 |= I2C_CR1_PE;// = I2C_Cmd(I2C,ENABLE);   enable i2c device
		
	I2C_InitStructure.I2C_ClockSpeed          = _i2c_speed;                  //Initialize the I2C_ClockSpeed member
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;                //Initialize the I2C_Mode member
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;             //Initialize the I2C_DutyCycle member
	I2C_InitStructure.I2C_OwnAddress1         = 0;                           //Initialize the I2C_OwnAddress1 member
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;              //Initialize the I2C_Ack member
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;//Initialize the I2C_AcknowledgedAddress member
	I2C_Init(_i2c,&I2C_InitStructure); //initialize i2c device
	
	//interrupt initialize
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //group 2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //Enable the IRQ channel
	
	// Configure NVIC for I2Cx EVT Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = _evt_irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = _evt_preprio;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = _evt_subprio;
	NVIC_Init(&NVIC_InitStructure);
	_i2c->CR2 |= I2C_CR2_ITEVTEN;//enable i2c event interrupt
	
	// Configure NVIC for I2Cx ERR Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = _err_irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = _err_preprio;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = _err_subprio;
	NVIC_Init(&NVIC_InitStructure);
	_i2c->CR2 |= I2C_CR2_ITERREN;//enable i2c error interrupt
	_i2c->CR2 |= I2C_IT_BUF;     //enable i2c buffer interrupt
	
	ClearCommand();
	_healthy = true;
}

//software reset i2c bus
void IIC::Reset()
{
	_i2c->CR1 |= I2C_CR1_SWRST;  //set reset bit
	_i2c->CR1 &= ~I2C_CR1_SWRST; //clear reset bit
}

//get one i2c command from i2c command queue
bool IIC::StartNextCommand()
{
	_evt_irq_cnt = 0;   //reset event interrupt counter
	if(_i2c_cmd_queue.Get(_current_cmd))//get next i2c command
	{
		_i2c_tx_cnt = 0; //clear tx counter
		_i2c_rx_cnt = 0; //clear rx counter
		if(_current_cmd.cmdType == OUT2IN) //ist transmit, then turn to receive
			_i2c_direction = I2C_Direction_Transmitter;
		else  //IN2OUT: 1st receive, then turn to transmit
			_i2c_direction = I2C_Direction_Receiver;
		
		I2C_GenerateSTART(_i2c,ENABLE); //to generate I2C Start signal
		return true;
	}
	return false; 
}



//add one i2c command to command queue
bool IIC::AddCommand(u8 slaveAddr, IIC_CMD_Type cmdType, u8 *pTxData, u8 txNum, u8 *pRxData, u8 rxNum)
{
	IIC_Command cmd;
	cmd.slaveAddr = slaveAddr;  //slave i2c address
	cmd.cmdType = cmdType;      //i2c command type	
	cmd.outDataLen = txNum;	    //number of bytes send to slave
	cmd.inDataLen = rxNum;      //number of bytes to recieve from slave
	cmd.pDataIn = pRxData;		  //ponter to receive data from slave
	for(u8 i=0; i<txNum; i++)   //data send to slave
		cmd.DataOut[i] = pTxData[i];
	return _i2c_cmd_queue.Put(cmd); //add to command queue
}
//clear all i2c commands in queue
void IIC::ClearCommand()
{
	_i2c_cmd_queue.Clear();
}	

//i2c event interrupt handler
void IIC::EventIRQ(void)
{
	u32 I2C_Status = I2C_GetLastEvent(_i2c);
	switch(I2C_Status)
	{
		//after I2C_GenerateSTART(), need to send slave address and write/read bit
		case I2C_EVENT_MASTER_MODE_SELECT:
			I2C_AcknowledgeConfig(_i2c,ENABLE);
			I2C_Send7bitAddress(_i2c, _current_cmd.slaveAddr, _i2c_direction);
      _i2c_tx_cnt = 0;
			_i2c_rx_cnt = 0;
			break;
		
		//after I2C_Send7bitAddress(addr+write), need to send data to slave
		case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
			I2C_SendData(_i2c,_current_cmd.DataOut[_i2c_tx_cnt++]);//send one byte data
			break;
		//after I2C_Send7bitAddress(addr+read)
		case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
			if(_current_cmd.inDataLen == 1) //if only read one byte data
			{
				I2C_AcknowledgeConfig(_i2c,DISABLE); //NO ACK
				I2C_GenerateSTOP(_i2c,ENABLE);       //generate STOP signal
			}
			break;
			
		//Receive mode, and new byte arrived, need to receive data
		case I2C_EVENT_MASTER_BYTE_RECEIVED:
			_current_cmd.pDataIn[_i2c_rx_cnt++] = I2C_ReceiveData(_i2c); //receive one byte data
			if(_i2c_rx_cnt == _current_cmd.inDataLen-1)//only one byte data left to receive
			{
				I2C_AcknowledgeConfig(_i2c,DISABLE); //NO ACK
				I2C_GenerateSTOP(I2C2,ENABLE);		   //generate STOP signal
			}
			else if(_i2c_rx_cnt == _current_cmd.inDataLen)//all bytes of data is received
			{
				//1st in then out, and need to send data
				if(_current_cmd.cmdType==IN2OUT && _current_cmd.outDataLen>0)//need to switch to transmit mode
				{
					_i2c_direction = I2C_Direction_Transmitter; //switch to transmit mode
					I2C_GenerateSTART(_i2c,ENABLE);         //generate START signal
				}
				else //OUT2IN, current i2c command is complete
				{
					StartNextCommand(); //perform next i2c command
				}
			}
			break;
		
		//transmit mode, and one byte transmit completed
		case I2C_EVENT_MASTER_BYTE_TRANSMITTED:
			if(_i2c_tx_cnt >= _current_cmd.outDataLen)//data transmit is completed
			{
				//current command is completed
				if(_current_cmd.cmdType == IN2OUT)
				{
					I2C_GenerateSTOP(_i2c,ENABLE);
					StartNextCommand(); //switch to next command
				}
				else  //cmdType == OUT2IN
				{
					if(_current_cmd.inDataLen==0)//no data to read
					{
						I2C_GenerateSTOP(_i2c,ENABLE);
						StartNextCommand(); //switch to next command
					}
					else //need to read data from slave
					{
						_i2c_direction = I2C_Direction_Receiver;	//switch to receiver mode
						I2C_GenerateSTART(_i2c,ENABLE);    //generate start signal
					}
				}
			}
			else//still have data need to transmit
			{
				I2C_SendData(_i2c, _current_cmd.DataOut[_i2c_tx_cnt++]);//·¢ËÍÊý¾Ý
			}
			break;
	}
	//event irq dead detect
	if(++_evt_irq_cnt>50)
	{
		_healthy = false;
		Reset();
	}
}	

//i2c error interrupt handler
void IIC::ErrorIRQ()     
{
	//_i2c->SR1 = ~((uint16_t)0x0F00);
	//Reset();
	_i2c->CR1 |= I2C_CR1_SWRST;  //set reset bit
	_i2c->CR1 &= ~I2C_CR1_SWRST; //clear reset bit
	
	_healthy = false;
	//SPI1->CR1 = 1;
	//for(u32 i=0;i<10000;i++) ;
	//USART1->DR = 'X';  //I dont know why,
}
//check if i2c is work correctly
bool IIC::IsWorkingAndFree()
{
	return (_healthy && (I2C_GetLastEvent(_i2c)==0));
}




