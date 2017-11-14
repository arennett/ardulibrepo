/*
 * SerialHeaderRx.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADERRX_H_
#define SERIALHEADERRX_H_


#include <stddef.h>
#include "SerialRx.h"
#include "SerialPort.h"
#include "SoftSerialPort.h"


typedef struct {
	byte addr;
	void (*pUserCallBack)(const byte* data, size_t data_size);
	void* pNext=NULL;
} tCallBackMapper;



class SerialHeaderRx {
public:
	SerialHeaderRx(SerialPort* pSerialPort,size_t maxDataSize);
	virtual ~SerialHeaderRx();


	/*
	 * void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size),byte addr);
	 * here the user registers one ore more static callback methods
	 * the callback method is only called if the received addrTo is equal to addr
	 */
	void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size),byte addr);

	/**
	 * void internalCallBack(const byte* pData, size_t data_size);
	 * - is called by serialTx when serialTx receives a message
	 */
	void internalCallBack(const byte* pData, size_t data_size);

private:

	/**
	 * void getLastCallBackMapperEntry();
	 * - iterates through the mapper list, and returns the last entry
	 * < returns the last entry or null if list is empty*/
	tCallBackMapper* getLastCallBackMapperEntry();


	/*
	 * void deleteCallBackList(){
	 * - deletes all entries from  the list
	 */
	void             deleteCallBackList();

	SerialPort* pSerialPort = NULL;
	void* pSerialRx = NULL;
	tCallBackMapper *pCallBackMapperList =NULL;

};

#endif /* SERIALHEADERRX_H_ */
