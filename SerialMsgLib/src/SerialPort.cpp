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
	}
	return NULL;
}

SerialPort::SerialPort(){
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

size_t SerialPort::write(const byte*, size_t ){
	//DUMMY
}
void SerialPort::createBuffer(size_t maxDataSize) {
	if(serialRxState.pBuffer){
			delete serialRxState.pBuffer;
	};
	serialRxState.bufferSize = maxDataSize + sizeof(serPostamble) +sizeof (tSerialHeader);
	serialRxState.pBuffer = new byte[serialRxState.bufferSize];
}



