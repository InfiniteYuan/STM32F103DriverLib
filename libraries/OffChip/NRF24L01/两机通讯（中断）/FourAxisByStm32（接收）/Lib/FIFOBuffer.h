/**
*@file FIFOBuffer.h
*@author cq_dan3  2015-10-20 first creat FIFOBuffer class
*        Neutree  2015-10-28 add comments for FIFOBuffer class
*@version v1.1
*@brief A template for FIFO queue
*@copyright CQUTIOTLIB all right reserved
*
*/

#ifndef _FIFO_BUFFER_H_
#define _FIFO_BUFFER_H_

#include "stm32f10x.h"


/**
  *@breif A FIFO queue Template for FIFO application
  *
  *
  */
template<typename T, u16 MAX_SIZE>
class FIFOBuffer
{
private:
	T _buf[MAX_SIZE];  //fifo buffer data
	u16 _max_size;     //max buffer size
	u16 _size;         //current buffer nodes
	u16 _out_idx;      //output index of buffer
	u16 _in_idx;       //input index of buffer
public:

	/////////////////////////
	///@breif constructor
	/////////////////////////
	FIFOBuffer();


	/////////////////////////
	///@breif put node data to queue 
	///@param data the node data will put into queue
	////////////////////////
	bool Put(const T &data);


	////////////////////////
	///@breif get one node data from queue,the length of queue will decrease one
	///@param data the destination of node data from queue
	////////////////////////
	bool Get(T &data);


	//////////////////////////////
	///@breif put multiple node data to queue 
	///@param pdata the first node data adress of node data array will put into queue
	///@param num the length of node data that will put into queue
	/////////////////////////////
	bool Puts(T *pData, u16 num);


	//////////////////////////////
	///@breif get multiple node data to queue 
	///@param pdata the first node data adress of node data array will get from the queue
	///@param num the length of node data that will get from the queue
	/////////////////////////////
	bool Gets(T *pData, u16 num);

	////////////////////////////
	///@breif get the size of queue
	///@retval the size of queue
	///////////////////////////
	u16 Size();


	////////////////////////////////
	///@breif clear the queue
	////////////////////////////////
	void Clear();
};


template<typename T, u16 MAX_SIZE>
FIFOBuffer<T, MAX_SIZE>::FIFOBuffer()
{
	_max_size = MAX_SIZE;     //max buffer size
	_size = 0;         //current buffer nodes
	_out_idx = 0;      //output index of buffer
	_in_idx = 0;       //input index of buffer
}

//input one node to buffer
template<typename T, u16 MAX_SIZE>
bool FIFOBuffer<T, MAX_SIZE>::Put(const T &data)
{
	if (_size >= _max_size) //buffer overflow
		return false;

	_buf[_in_idx++] = data; //input data

	if (_in_idx >= _max_size) //input index validate
		_in_idx = 0;
	_size++;  //update buffer size 
	return true;
}


//input multi nodes to buffer
template<typename T, u16 MAX_SIZE>
bool FIFOBuffer<T, MAX_SIZE>::Puts(T *pData, u16 num)
{
	if ((_size + num) > _max_size) //buffer overflow
		return false;

	for (u16 i = 0; i < num; i++)
	{
		_buf[_in_idx++] = pData[i]; //input data
		if (_in_idx >= _max_size) //input index validate
			_in_idx = 0;
	}
	_size += num;  //update buffer size 
	return true;
}


//get one node from buffer
template<typename T, u16 MAX_SIZE>
bool FIFOBuffer<T, MAX_SIZE>::Get(T &data)
{
	if (_size <= 0)    //no data to get
		return false;

	data = _buf[_out_idx++]; //output data

	if (_out_idx >= _max_size)  //output index validate
		_out_idx = 0;

	_size--;//update buffer size
	return true;
}


//get multi nodes from buffer
template<typename T, u16 MAX_SIZE>
bool FIFOBuffer<T, MAX_SIZE>::Gets(T *pData, u16 num)
{
	if (_size < num) return false;  //not enough data to get

	for (u16 i = 0; i < num; i++)
	{
		pData[i] = _buf[_out_idx++]; //output data

		if (_out_idx >= _max_size)  //output index validate
			_out_idx = 0;
	}
	_size -= num;//update buffer size
	return true;
}

//Get the number of nodes in buffer (_size)
template<typename T, u16 MAX_SIZE>
u16 FIFOBuffer<T, MAX_SIZE>::Size()
{
	return _size;
}


//clear all nodes in buffer
template<typename T, u16 MAX_SIZE>
void FIFOBuffer<T, MAX_SIZE>::Clear()
{
	_size = 0;         //current buffer nodes
	_out_idx = 0;      //output index of buffer
	_in_idx = 0;       //input index of buffer
}





#endif


