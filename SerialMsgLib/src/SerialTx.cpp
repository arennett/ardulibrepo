/*
 * SoftSerialTx.cpp

 *
 *  Created on: 31.10.2017
 *      Author: Andre Rennett
 *
 *  Implementation for SoftSerialTx
 *  comments see SoftSerialTx.h
 */

#include "SerialTx.h"

#include "Arduino.h"
#include <SoftwareSerial.h>
#include "SoftSerial.h"
#include <src/tools.h> //see ToolsLib2
#include <src/tools.h> // see ToolsLib2



SerialTx::SerialTx(SerialPort* pSerialPort){
	this->pSerialPort = pSerialPort;

}


SerialTx::~SerialTx() {

}

void SerialTx::begin(long speed){
	pSerialPort->begin(speed);

}


SerialPort* SerialTx::getSerialPort(){
	return pSerialPort;
}


void SerialTx::sendData(byte* data, size_t datasize) {

	pSerialPort->write(serPreamble,sizeof serPreamble);
	pSerialPort->write(data,datasize);
	pSerialPort->write(serPostamble,sizeof serPostamble);
	MPRINTLN("write data");
}
