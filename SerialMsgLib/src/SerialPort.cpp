/*
 * SerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#include "SerialMsg.h"
#include "SerialHeader.h"
#include "SerialPort.h"


SerialPort* SerialPort::pSerialPortList=NULL;

SerialPort* SerialPort::getPort(byte remoteSysId){
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		if (pPort->remoteSysId==remoteSysId){
			return pPort;
		}
		pPort=(SerialPort*) pPort->pNext;
	}
	return NULL;
}

SerialPort::SerialPort(){
};

SerialPort::SerialPort(byte remoteSysId){
	this->remoteSysId=remoteSysId;
	SerialPort* pLast = pSerialPortList;

	if (!pLast) {
		pSerialPortList =this;
		return;
	}
	while(!pLast->pNext) {
		pLast=(SerialPort*)pLast->pNext;
	}
	pLast->pNext=this;
};
SerialPort::~SerialPort() {};


void SerialPort::createBuffer(size_t maxDataSize) {
	if(serialRxState.pBuffer){
			delete serialRxState.pBuffer;
	};
	serialRxState.bufferSize = maxDataSize + sizeof(serPostamble) +sizeof (tSerialHeader);
	serialRxState.pBuffer = new byte[serialRxState.bufferSize];
}



