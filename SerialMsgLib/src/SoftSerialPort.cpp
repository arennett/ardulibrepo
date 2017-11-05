/*
 * SoftSerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#include <SoftwareSerial.h>
#include "SoftSerialPort.h"


SoftSerialPort::SoftSerialPort(byte pinRx ,byte pinRy) {
	pSoftwareSerial =new SoftwareSerial(pinRx,pinRy);
	deleteSoftwareSerial=true;
}


SoftSerialPort::SoftSerialPort(SoftwareSerial* pSoftwareSerial) {
	this->pSoftwareSerial= pSoftwareSerial;
	deleteSoftwareSerial=false;
}

SoftSerialPort::~SoftSerialPort(){
	if (deleteSoftwareSerial) {
		delete pSoftwareSerial;
	}
}


byte SoftSerialPort::read(){
	return pSoftwareSerial->read();
}

bool SoftSerialPort::write(byte b){
	 return (pSoftwareSerial->write(b)) > 0;
}

size_t SoftSerialPort::write(byte* bb,size_t len){
	 return pSoftwareSerial->write(bb,len) ;
}


bool SoftSerialPort::listen(){
	 return pSoftwareSerial->listen();
}

void SoftSerialPort::begin(long speed){
	 return pSoftwareSerial->begin(speed);
}

int  SoftSerialPort::available(){
	return pSoftwareSerial->available();
}
bool SoftSerialPort::isListening(){
	 return pSoftwareSerial->isListening();
}
