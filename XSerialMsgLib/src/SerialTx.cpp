/*
 * SoftSerialTx.cpp

 *
 *  Created on: 31.10.2017
 *      Author: Andre Rennett
 *
 *  Implementation for SoftSerialTx
 *  comments see SoftSerialTx.h
 */

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <tools.h> //see ToolsLib2
#include "SerialMsg.h"
#include "SerialTx.h"
#include "SerialPort.h"




namespace SerialMsgLib {

SerialTx::SerialTx(){
	pSerialPort=NULL;
	MPRINTLNSVAL("SerialTx::SerialTx() free ",freeRam());
}

SerialTx::SerialTx(SerialPort* pSerialPort){
	setPort(pSerialPort);
	MPRINTLNSVAL("SerialTx::SerialTx() free ",freeRam());
}


SerialTx::~SerialTx() {

}

bool SerialTx::setPort(SerialPort* pSerialPort) {
	this->pSerialPort =pSerialPort;
	return true;
}



SerialPort* SerialTx::getSerialPort(){
	return pSerialPort;
}


void SerialTx::sendData(const byte* data, size_t datasize) {

	sendPreamble();
	sendRawData(data,datasize);
	sendPostamble();
}

void SerialTx::sendRawData(const byte* data, size_t datasize) {
	DPRINTLNSVAL("SerialTx::sendRawData - size:",datasize);
	pSerialPort->write(data,datasize);
}

void SerialTx::sendPreamble() {
	DPRINTLNS("SerialTx::sendPreamble");
	pSerialPort->write(serPreamble,sizeof serPreamble);

}

void SerialTx::sendPostamble() {
	DPRINTLNS("SerialTx::sendPostamble");
	pSerialPort->write(serPostamble,sizeof serPostamble);

}
};



