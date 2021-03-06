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

namespace SerialMsgLib {

//static
SoftSerialPort* SoftSerialPort::pSoftSerialPortList = NULL;
SoftSerialPort* SoftSerialPort::pMaster = NULL;

//--------------------static---------------------------------------------

void SoftSerialPort::cycleListenerPort() {
		SoftSerialPort* pPort = getListenerPort();
		// if not at least 2 SoftwareSerialPorts we do not need to switch

		if (count() == 1) {
			pSoftSerialPortList->setMaster();
			pMaster->listen();
			return;
		}

		DPRINTLNSVAL("SoftSerialPort::cycleListenerPort() listener : ",pPort->getId());
		DPRINTLNSVAL("SoftSerialPort::cycleListenerPort() acb count all for listener: ",AcbList::countAll(pPort->getId()));

		// only change listen if no replies expected on this port
		if (pPort && pPort->available() == 0  // no unread data on listener port
				&& AcbList::countAll(pPort->getId()) == 0 //no acbs to listener port found
				&& (millis() - pPort->listenTimeStamp) > MAX_SOFT_LISTEN_TIME // we listen long enough
		){
			if (!pMaster) {
				pPort->cycleNextSoftSerialPort()->listen();
			}else{
				pMaster->listen();
			}

		}
}

//static
SoftSerialPort* SoftSerialPort::getListenerPort() {
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		if (pPort->getType() == PORTTYPE_SOFTSERIAL && pPort->isListening()) {
			return (SoftSerialPort*) pPort;
		}
		pPort = (SerialPort*) pPort->getNext();
	}
	if (!pPort){
		pPort = pSerialPortList;
		while (pPort) {
			if (pPort->getType() == PORTTYPE_SOFTSERIAL) {
				pPort->listen();
				return (SoftSerialPort*) pPort;
			}
			pPort = (SerialPort*) pPort->getNext();
		}

	}
	return NULL;
}

//static
void SoftSerialPort::resetMaster(){
	SoftSerialPort::pMaster=NULL;
}

bool SoftSerialPort::isMaster(){
	return this==pMaster;
}

void SoftSerialPort::setMaster() {
	if (pMaster!=this) {
			pMaster=this;
			MPRINTLNSVAL("SoftSerialPort::setMaster : port : " ,getId());
	}
}

byte SoftSerialPort::count() {
	byte cnt=0;
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		++cnt;
		pPort = (SerialPort*) pPort->getNext();
	}
	return cnt;
}

//-----------------------end static------------------------------------------

SoftSerialPort::SoftSerialPort(byte pinRx, byte pinRy, byte remoteSysId) :
		SerialPort(remoteSysId) {

	pSoftwareSerial = new SoftwareSerial(pinRx, pinRy);
	MPRINTLNSVAL("SoftSerialPort::SoftSerialPort free ",freeRam());
}

SoftSerialPort::SoftSerialPort(SoftwareSerial* pSoftwareSerial, byte remoteSysId) :
		SerialPort(remoteSysId) {
	this->pSoftwareSerial = pSoftwareSerial;

	MPRINTLNSVAL("SoftSerialPort::SoftSerialPort free ",freeRam());
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
		MPRINTLNSVAL("SoftSerialPort::listen> on port :", getId());
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
	DPRINTLNS("SoftSerialPort::cycleNextSoftSerialPort>");
	SerialPort* pPort=  (SerialPort*) this->getNext();
	if (!pPort) {// next is first one
		pPort= SerialPort::pSerialPortList;
	}
	while (pPort) {
		// its supposed to find at least itself
		if (pPort->getType()==PORTTYPE_SOFTSERIAL) {
			return (SoftSerialPort*)pPort;
		}
		pPort =  (SerialPort*) pPort->getNext();

		if(!pPort) {
			pPort= SerialPort::pSerialPortList;
		}
	}
	assert(false);
	return NULL;
}
}
