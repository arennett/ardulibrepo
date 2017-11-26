/*
 * SerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#include "SerialPort.h"


static SerialPort* SerialPort::pSerialPortList=NULL;


SerialPort::SerialPort() {
	this->id=0;

}



SerialPort::~SerialPort() {
	// TODO Auto-generated destructor stub
}

