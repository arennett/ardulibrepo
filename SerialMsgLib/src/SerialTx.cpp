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

	sendPreamble();
	sendRawData(data,datasize);
	sendPostamble();
}

void SerialTx::sendRawData(byte* data, size_t datasize) {
	pSerialPort->write(data,datasize);
	DPRINTSVAL("SerialTx::sendRawData - size:",datasize);
}

void SerialTx::sendPreamble() {
	pSerialPort->write(serPreamble,sizeof serPreamble);
	DPRINTLN("SerialTx::sendPreamble");
}

void SerialTx::sendPostamble() {
	pSerialPort->write(serPostamble,sizeof serPostamble);
	DPRINTLN("SerialTx::sendPostamble");
}




