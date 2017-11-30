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

SerialPort::SerialPort(){};
SerialPort::~SerialPort() {};


void SerialPort::createBuffer(size_t maxDataSize) {
	if(serialRxState.pBuffer){
			delete serialRxState.pBuffer;
	};
	serialRxState.bufferSize = maxDataSize + sizeof(serPostamble) +sizeof (tSerialHeader);
	serialRxState.pBuffer = new byte[serialRxState.bufferSize];
}



