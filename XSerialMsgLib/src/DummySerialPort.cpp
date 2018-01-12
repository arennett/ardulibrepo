/*
 * DummySerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#include <tools.h>
#include "SerialMsg.h"
#include "SerialPort.h"
#include "DummySerialPort.h"
#include "AcbList.h"
#define MAX_SOFT_LISTEN_TIME 200

namespace SerialMsgLib {
//static
DummySerialPort* DummySerialPort::pDummySerialPortList = NULL;

//--------------------static---------------------------------------------

void DummySerialPort::cycleListenerPort() {
	DummySerialPort* pPort = getListenerPort();
	// if not at least 2 SoftwareSerialPorts we do not need to switch
	if (!pPort || !pPort->getNext()) {
		return;
	};

	if (pPort && pPort->available() == 0
			&& AcbList::getInstance()->countAll(pPort->getId()) == 0
			&& (millis() - pPort->listenTimeStamp) > MAX_SOFT_LISTEN_TIME
			// uncomment listenTimeStamp in listen
		){
		pPort->cycleNextDummySerialPort()->listen();
	}
}

DummySerialPort* DummySerialPort::getListenerPort() {
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		if (pPort->getType() == PORTTYPE_SOFTSERIAL && pPort->isListening()) {
			return (DummySerialPort*) pPort;
		}
		pPort = (SerialPort*) pPort->getNext();
	}
	if (!pPort){
		pPort = pSerialPortList;
		while (pPort) {
			if (pPort->getType() == PORTTYPE_SOFTSERIAL) {
				pPort->listen();
				return (DummySerialPort*) pPort;
			}
			pPort = (SerialPort*) pPort->getNext();
		}

	}
	return NULL;
}

byte DummySerialPort::count() {
	byte cnt=0;
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		++cnt;
		pPort = (SerialPort*) pPort->getNext();
	}
	return cnt;
}

//-----------------------end static------------------------------------------

DummySerialPort::DummySerialPort(byte , byte , byte remoteSysId) :
		SerialPort(remoteSysId) {

	//pSoftwareSerial = new SoftwareSerial(pinRx, pinRy);

}

DummySerialPort::DummySerialPort(byte remoteSysId) :
		SerialPort(remoteSysId) {
	//this->pSoftwareSerial = pSoftwareSerial;


}

DummySerialPort::~DummySerialPort() {

}




tPortType DummySerialPort::getType() {
	return tPortType::PORTTYPE_SOFTSERIAL;
}

byte DummySerialPort::read() {
	//byte b = pSoftwareSerial->read();
	return 0;
}

bool DummySerialPort::write(byte ) {
	//return (pSoftwareSerial->write(b)) > 0;
	return true;
}

size_t DummySerialPort::write(const byte* , size_t ) {
	//return pSoftwareSerial->write(bb, len);
	return 0;
}

bool DummySerialPort::listen() {
	if (!isListening()) {
		listenTimeStamp = millis();
		XPRINTLNSVAL("DummySerialPort::listen> on port :", getId());
		//return pSoftwareSerial->listen();
		return false;
	}
	return false;
}

void DummySerialPort::begin(long ) {
	//pSoftwareSerial->begin(speed);
}

int DummySerialPort::available() {
	//return pSoftwareSerial->available();
	return 0;
}
bool DummySerialPort::isListening() {
	//return pSoftwareSerial->isListening();
	return true;
}

DummySerialPort* DummySerialPort::cycleNextDummySerialPort() {

	SerialPort* pPort=  (SerialPort*) this->getNext();
	if (!pPort) {// next is first one
		pPort= SerialPort::pSerialPortList;
	}
	while (pPort) {
		// its supposed to find at least itself
		if (pPort->getType()==PORTTYPE_SOFTSERIAL) {
			return (DummySerialPort*)pPort;
		}
		pPort =  (SerialPort*) pPort->getNext();

		if(!pPort) {
			pPort= SerialPort::pSerialPortList;
		}
	}
	assert(false);
	return NULL;
}
};
