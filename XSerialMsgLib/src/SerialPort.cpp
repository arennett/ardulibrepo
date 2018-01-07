/*
 * SerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */


#include "SerialMsg.h"
#include "SerialHeader.h"
#include "SerialPort.h"
#include "SerialNode.h"
#include "SerialNodeNet.h"

namespace SerialMsgLib {
SerialPort* SerialPort::pSerialPortList=NULL;

SerialPort::SerialPort(byte remoteSysId){


	DPRINTLNSVAL("SerialPort::SerialPort> sysId: ", remoteSysId);
	ASSERTP( remoteSysId > 0 ,"remoteSysId must be > 0 !");
	ASSERTP( remoteSysId != SerialNodeNet::getInstance()->getSystemId(),"remoteSysId must be diffrent from systemId");

	this->remoteSysId=remoteSysId;
	SerialPort* pLast = pSerialPortList;

	uint8_t cnt=0;
	if (!pLast) {
		pSerialPortList =this;
		++cnt;

	}else{
		++cnt;
		while(pLast->pNext) {
			pLast=(SerialPort*)pLast->pNext;
			++cnt;
		}
		pLast->pNext=this;
		++cnt;
	}

    pPortRxTxMapper =new SerialPortRxTxMapper(this);
	createDataBuffer(0);
	DPRINTLNSVALS("SerialPort::SerialPort> cnt: ",cnt, " inited");
	XPRINTLNSVAL("SerialPort::SerialPort() free ",freeRam());

	//pSerialRx= new SerialRx(this,20,SerialNode::update);
};




void SerialPort::readNextOnAllPorts() {

	SerialPort* pport = SerialPort::pSerialPortList;


	assert(pport);
	byte nrofports = 0;
	while (pport) {
		++nrofports;
		if (pport->getRx()) {
				//DPRINTLNS("SerialRx::readNextOnAllPorts> data available");

			  int size = pport->available();
			  if (size > 0) {
				  DPRINTLNSVAL("SerialRx::data available on port: ", pport->remoteSysId);
				  DPRINTLNSVAL("SerialRx::data available size :",size );
				  while (pport->available()){
				  	  pport->getRx()->readNext();
				  }

			  }

		} else {
			DPRINTLNSVAL("SerialRx::port has no receiver: ", pport->remoteSysId);
		}


		pport = (SerialPort*) pport->pNext;
	}

}


SerialPort::~SerialPort() {
	delete pPortRxTxMapper;

};

byte SerialPort::getId(){
	return remoteSysId;
}

SerialPort* SerialPort::getNext(){
	return (SerialPort*) pNext;
}


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
}


