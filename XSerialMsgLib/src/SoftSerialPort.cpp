/*
 * SoftSerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#include <SoftwareSerial.h>
#include <tools.h>
#include "SerialMsg.h"
#include "SoftSerialPort.h"
#include "AcbList.h"

#define MAX_SOFT_LISTEN_TIME 200
//static
SoftSerialPort* SoftSerialPort::pSoftSerialPortList = NULL;

//static
void SoftSerialPort::cycleListenerPort() {
	SoftSerialPort* pPort = getListenerPort();
	// if not at least 2 SoftwareSerialPorts we do not need to switch
	if (!pPort || !pPort->pNext) {
		return;
	};

	if (pPort && pPort->available() == 0
			&& AcbList::instance.count(pPort->remoteSysId) == 0
			&& (millis() - pPort->listenTimeStamp) > MAX_SOFT_LISTEN_TIME
			// uncomment listenTimeStamp in listen
		){
		pPort->cycleNextSoftSerialPort()->listen();
	}
}

SoftSerialPort* SoftSerialPort::getListenerPort() {
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		if (pPort->getType() == PORTTYPE_SOFTSERIAL && pPort->isListening()) {
			return (SoftSerialPort*) pPort;
		}
		pPort = (SerialPort*) pPort->pNext;
	}
	if (!pPort){
		pPort = pSerialPortList;
		while (pPort) {
			if (pPort->getType() == PORTTYPE_SOFTSERIAL) {
				pPort->listen();
				return (SoftSerialPort*) pPort;
			}
			pPort = (SerialPort*) pPort->pNext;
		}

	}
	ASSERTP(pPort,"Port not found");
	return NULL;
}

SoftSerialPort::SoftSerialPort(byte pinRx, byte pinRy, byte remoteSysId) :
		SerialPort(remoteSysId) {

	pSoftwareSerial = new SoftwareSerial(pinRx, pinRy);

}

SoftSerialPort::SoftSerialPort(SoftwareSerial* pSoftwareSerial, byte remoteSysId) :
		SerialPort(remoteSysId) {
	this->pSoftwareSerial = pSoftwareSerial;


}

SoftSerialPort::~SoftSerialPort() {

}



tPortType SoftSerialPort::getType() {
	return tPortType::PORTTYPE_SOFTSERIAL;
}

byte SoftSerialPort::read() {
	byte b = pSoftwareSerial->read();
	return b;
}

bool SoftSerialPort::write(byte b) {
	return (pSoftwareSerial->write(b)) > 0;
}

size_t SoftSerialPort::write(const byte* bb, size_t len) {
	return pSoftwareSerial->write(bb, len);
}

bool SoftSerialPort::listen() {
	if (!isListening()) {
		listenTimeStamp = millis();
		//XPRINTLNSVAL("SoftSerialPort::listen> on port :", remoteSysId);
		return pSoftwareSerial->listen();
	}
	return false;
}

void SoftSerialPort::begin(long speed) {
	pSoftwareSerial->begin(speed);
}

int SoftSerialPort::available() {
	return pSoftwareSerial->available();
}
bool SoftSerialPort::isListening() {
	return pSoftwareSerial->isListening();
}

SoftSerialPort* SoftSerialPort::cycleNextSoftSerialPort() {

	SoftSerialPort* pPort= this->pNext;
	if (!pPort) {// next is first one
		pPort= SerialPort::pSerialPortList;
	}
	while (pPort) {
		// its supposed to find at least itself
		if (pPort->getType()==PORTTYPE_SOFTSERIAL) {
			return (SoftSerialPort*)pPort;
		}
		pPort = pPort->pNext;

		if(!pPort) {
			pPort= SerialPort::pSerialPortList;
		}
	}
	assert(false);
	return NULL;
}
