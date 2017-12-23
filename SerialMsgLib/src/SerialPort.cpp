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


SerialPort* SerialPort::pSerialPortList=NULL;

SerialPort::SerialPort(byte remoteSysId){
	MPRINTLNSVAL("SerialPort::SerialPort> sysId: ", remoteSysId);
	ASSERTP( remoteSysId > 0 ,"remoteSysId must be > 0 !");
	ASSERTP( remoteSysId != SerialNode::systemId,"remoteSysId must be diffrent from systemId");

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
	MPRINTLNSVALS("SerialPort::SerialPort> cnt: ",cnt, " inited");


	//pSerialRx= new SerialRx(this,20,SerialNode::update);
};


void SerialPort::cycleListenerPort() {
	SerialPort* pPort = pSerialPortList;
	while (pPort) {
		if (pPort->isListening()
				&& ((millis()- pPort->listenTimeStamp) > MAX_LISTENTIME)
				&& (SerialNode::acbList.count() == 0)
				&& !pPort->available()){

			pPort->cycleNextPort()->listen();

			break;
		}
		pPort=(SerialPort*) pPort->pNext;
	}
}

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
				  XPRINTLNSVAL("SerialRx::data available on port: ", pport->remoteSysId);
				  XPRINTLNSVAL("SerialRx::data available size :",size );
				  while (pport->available()){
				  	  pport->getRx()->readNext();
				  }

			  }

		} else {
			MPRINTLNSVAL("SerialRx::port has no receiver: ", pport->remoteSysId);
		}


		pport = (SerialPort*) pport->pNext;
	}

}


SerialPort::~SerialPort() {
	delete pPortRxTxMapper;

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


SerialPort* SerialPort::cycleNextPort(){
   // XPRINTLNS("SerialPort::cycleNextPort") ;
	if (this->pNext) {
		return (SerialPort*)this->pNext;
	}else {
		return pSerialPortList;
	}
}


void SerialPort::createDataBuffer(size_t maxDataSize) {
	pPortRxTxMapper->createRxBuffer(maxDataSize);

}



