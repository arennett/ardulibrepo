/*
 * SerialPort.cpp
 *
 *  Created on: 04.11.2017
 *      Author: User
 */


#include  <tools.h>
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
				  XPRINTLNSVAL("SerialRx::data available on port: ", pport->remoteSysId);
				  XPRINTLNSVAL("SerialRx::data available size :",size );
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

void SerialPort::sendMessage(tSerialHeader* pHeader,const byte* pData,size_t datasize) {
		/* must be moved to SoftSerialPort
		if (available()) {
		  int size =available();
		   if (size > 0) {
			  XPRINTLNSVAL("SerialPort::sendMessage: data available by send on port: ", remoteSysId);
			  XPRINTLNSVAL("SerialPort::sendMessage read first: ",size );
			  while (available()){
				  getRx()->readNext();
			  }

		  }
		} */


		if (pHeader->aktid == 0) { // we need an aktId
			pHeader->aktid = AcbList::getInstance()->getNextAktId();
		}

	 	if (pHeader->isReplyExpected()) {
	 		tAcb* pAcb =AcbList::getList(pHeader->fromAddr.sysId, true)->createOrUseAcb(pHeader);
			pAcb->portId = getId();
			listen();
		}
		XPRINTS("SerialPort::sendMessage> ");XPRINTLNHEADER(pHeader);
	    getTx()->sendPreamble();
		getTx()->sendRawData((byte*) pHeader, sizeof(tSerialHeader));
		if (pData && datasize > 0) {
			getTx()->sendRawData((byte*) pData, datasize);
		}
		getTx()->sendPostamble();

		if (pData) {
			XPRINTLNSVAL(" datasize: ", datasize);
		}else{
			XPRINTLN("");
		}
}

void SerialPort::sendMessage(const byte* pMessage, size_t messageSize) {
	ASSERTP( sizeof(tSerialHeader) <= messageSize, "SerialPort::sendMessage message < header size");
	tSerialHeader*  pHeader = ( tSerialHeader* ) pMessage;
	byte* pData = ( byte*) pHeader + sizeof(tSerialHeader);
	size_t datsize = messageSize- sizeof(tSerialHeader);
	return sendMessage(pHeader,pData,datsize);
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


