/*
 * SerialNode.h
 *
 *  Created on: 23.11.2017
 *      Author: rea
 */
#include <stddef.h>
#include "SerialPort.h"
#include "SerialTx.h"
#include "SerialRx.h"
#include "SerialHeader.h"

#ifndef SERIALNODE_H_
#define SERIALNODE_H_




typedef struct { //Connection Control Block
	byte localAddr;
	byte remoteAddr;    //remote address
	#define  CONNECTION_STATUS_NOT_READY  	1
	#define  CONNECTION_STATUS_READY 		2
	#define  CONNECTION_STATUS_DISCONNECTED 3
	#define  CONNECTION_STATUS_CONNECTED 	4
	byte status=0;
	bool master=false;
	void (*pUserCallBack)(const byte* data, size_t data_size);
	void* pNext = NULL;
} tCcb;

class SerialNode {
public:
	SerialNode(SerialRx* pRx,SerialTx* pTx,byte addr,bool master=false);
	bool connect(unsigned long speed,byte remoteAddress,unsigned long timeOut,unsigned long reqPeriod);
	bool send(tSerialHeader* pHeader,byte* pData,byte datasize);

	void setReceiveCallBack(void (*ptr)(byte* pData,size_t datasize));
	void* pNext=NULL; // next SerialNode
private:
	tCcb*  pCcb = NULL;
	void (*pCallBack)(byte* pData,size_t datasize)=NULL;
	SerialTx* pTx =NULL;
	SerialRx* pRx =NULL;

	bool sendCR();
	bool internalReceive(byte*& pData,size_t datasize);
	virtual ~SerialNode();
};

SerialNode* pNodeList=NULL;

void serialRxCallBack(byte* pData, size_t datasize);


#endif /* SERIALNODE_H_ */
