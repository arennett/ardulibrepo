/*
 * SerialHeaderRx.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADERRX_H_
#define SERIALHEADERRX_H_


#include <stddef.h>
#include "SerialTx.h"
#include "SerialRx.h"
#include "SerialPort.h"
#include "SoftSerialPort.h"


typedef struct {
	byte addr;
	void (*pUserCallBack)(byte* data, size_t data_size);
	void* pNext=NULL;
} tCallBackMapper;



class SerialHeaderRx {
public:
	SerialHeaderRx(SerialPort* pSerialPort,size_t maxDataSize,byte addr);
	virtual ~SerialHeaderRx();

	void setUpdateCallback(void (*ptr)(byte* pData, size_t data_size),byte addr);
	void internalCallBack(byte* pData, size_t data_size);

private:

	SerialPort* pSerialPort = NULL;
	SerialRx* pSerialRx = NULL;
	tCallBackMapper *pCallBackMapperList =NULL;


};

#endif /* SERIALHEADERRX_H_ */
