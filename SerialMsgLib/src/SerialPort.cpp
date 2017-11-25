/*
 * SerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#include "SerialPort.h"

static SerialPort* SerialPort::GetNextPort(SerialPort* pPort ) {
       if (pPort==NULL) {
    	   return pSerialPortList;
       }
      return (pPort->pNext) ? ( SerialPort*) pPort->pNext : pSerialPortList;
}
static SerialPort* SerialPort::GetPortList() {
	return pSerialPortList;
}

SerialPort::SerialPort() {
	this->id=0;

}



SerialPort::~SerialPort() {
	// TODO Auto-generated destructor stub
}

