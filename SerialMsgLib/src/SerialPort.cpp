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

SerialPort::SerialPort(byte remoteSysId){
	MPRINTLNSVAL("SerialPort::SerialPort> sysId: ", remoteSysId);
	ASSERTP( remoteSysId > 0 ,"remoteSysId must be > 0 !");
	this->remoteSysId=remoteSysId;
	SerialPort* pLast = pSerialPortList;


	if (!pLast) {
		pSerialPortList =this;
	}else {
		while(!pLast->pNext) {
			pLast=(SerialPort*)pLast->pNext;
		}
		pLast->pNext=this;
	}

    pPortRxTxMapper =new SerialPortRxTxMapper(this);
	MPRINTFREE;
	createDataBuffer(0);
	MPRINTLNS("SerialPort::SerialPort> inited");

	//pSerialRx= new SerialRx(this,20,SerialNode::update);
};

SerialPort::~SerialPort() {

};


SerialPort* SerialPort::getPort(byte remoteSysId){
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		if (pPort->remoteSysId==remoteSysId){
			return pPort;
		}
		pPort=(SerialPort*) pPort->pNext;
	}
	return NULL;
}

SerialRx* SerialPort::getRx(){
	assert(pPortRxTxMapper->getRx());
	return pPortRxTxMapper->getRx();
}
SerialTx* SerialPort::getTx(){
	assert(pPortRxTxMapper->getTx());
	return pPortRxTxMapper->getTx();
}




SerialPort::SerialPort(){

};




void SerialPort::createDataBuffer(size_t maxDataSize) {
	pPortRxTxMapper->createRxBuffer(maxDataSize);

}



