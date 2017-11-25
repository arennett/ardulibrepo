/*
 * SerialNode.cpp
 *
 *  Created on: 23.11.2017
 *      Author: rea
 */

#include "SerialPort.h"
#include "SerialNode.h"
#include <tools.h>

unsigned long serialNodeAktId = 0;

void SerialNode::Update(const byte* pMessage, size_t messageSize,
		SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	size_t hsize = sizeof(tSerialHeader);
	size_t dsize;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}



	SerialNode* pNode = SerialNode::pNodeList;
	while (pNode != NULL && pNode->pCcb->localAddr != pHeader->toAddr) {
		pNode = pNode->pNext;

	}
	if (pNode) {
		pNode->setPort(pPort);
		pNode->pCallBack((tSerialHeader*)pHeader, (dsize > 0) ?(byte*) pHeader + hsize : NULL, dsize);
	}

}

static void SerialNode::Init(size_t maxDataSize) {
	serialRx.setUpdateCallback(Update);
	serialRx.createBuffer(maxDataSize+sizeof(tSerialHeader));
}

static unsigned int SerialNode::GetNextAktId() {
	if (serialNodeAktId >= 65535) {
		serialNodeAktId = 0;
	}
	return ++serialNodeAktId;
}

SerialNode::SerialNode(byte address, SerialPort* pSerialPort = NULL) {
	this->pSerialPort = pSerialPort;
	this->pCcb = new tCcb();
	pCcb->localAddr = address;

	SerialNode* pNode = pNodeList;

	while (pNode && pNode->pNext) {
		pNode = (SerialNode*) pNode->pNext;
	}
	if (!pNode) {
		pNodeList = this;
	} else {
		pNode->pNext = this;
	}
}

SerialNode::~SerialNode() {
	delete pCcb;
}

void SerialNode::setPort(SerialPort* pPort) {
	this->pSerialPort = pPort;
}

bool SerialNode::connect(byte remoteAddress, bool active = true,
		unsigned long timeOut = 0, unsigned long reqPeriod = 0) {

	pCcb->remoteAddr = remoteAddress;
	pCcb->status = CONNECTION_STATUS_NOT_READY;

	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;

	while (millis() < endMillis) {
		if (pCcb->status != CONNECTION_STATUS_CONNECTED) {
			if (pCcb->master) {
				if (millis() > reqMillis) {
					send(CMD_CR);
					reqMillis = millis() + reqPeriod;
				}
			}
		} else {
			MPRINTLN("SerialHeaderRx::connect: UP");
			return true;
		}

	}
	MPRINTLN("SerialHeaderRx::connect: TIMEOUT");
	return false; // no connections or timeout

}

tAktId SerialNode::send(tSerialCmd cmd, byte par = 0, byte* pData = NULL,
		byte datasize = 0, tAktId replyOn = 0, byte replyTo = 0) {
	tSerialHeader header;

	// not connected
	if (pCcb->status != CONNECTION_STATUS_CONNECTED
			&& (cmd != CMD_CR && cmd == CMD_ACK || cmd != CMD_NAK)) {
		    MPRINTSVAL("SerialNode::send > not connected, cmd not allowed : " ,cmd);
		return 0;
	}

	if (replyOn > 0) {
		header.aktid = replyOn;
		header.toAddr = replyTo;
	} else {
		header.aktid = GetNextAktId();
		header.toAddr = pCcb->remoteAddr;
	}
	header.fromAddr = pCcb->localAddr;

	header.cmd = cmd;
	header.par = par;

	if (pSerialPort) { //connected or port was preset
		serialTx.setPort(pSerialPort);
		serialTx.sendPreamble();
		serialTx.sendRawData((byte*) &header, sizeof(tSerialHeader));
		if (pData && datasize > 0) {
			serialTx.sendRawData((byte*) pData, datasize);
		}
		serialTx.sendPostamble();
		return header.aktid;
	} else { // not connected
		// try all ports
		SerialPort* pPort = SerialPort::GetPortList();
		while (pPort != NULL) {
			serialTx.setPort(pSerialPort);
			serialTx.sendPreamble();
			serialTx.sendRawData((byte*) &header, sizeof(tSerialHeader));
			if (pData && datasize > 0) {
				serialTx.sendRawData((byte*) pData, datasize);
			}
			serialTx.sendPostamble();
			pPort = pPort->pNext;
		}
		if (SerialPort::GetPortList()) {
			return header.aktid;
		}
	}

	return 0;
}

void SerialNode::setReceiveCallBack(
		void (*ptr)(tSerialHeader* pHeader, byte* pData, size_t datasize)) {
	pCallBack = ptr;
}

