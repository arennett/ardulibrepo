/*
 * SerialNode.cpp
 *
 *  Created on: 23.11.2017
 *      Author: rea
 */

#include "SerialNode.h"
#include <tools.h>

SerialNode::SerialNode(SerialRx* pRx, SerialTx* pTx, byte localAddr,
		bool master) {
	this->pTx = pTx;
	this->pRx = pRx;
	this->pCcb = new tCcb();
	pCcb->localAddr = localAddr;

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

bool SerialNode::connect(unsigned long speed, byte remoteAddress,
		unsigned long timeOut, unsigned long reqPeriod) {
	pCcb->remoteAddr = remoteAddress;
	pTx->begin(speed);
	pRx->begin(speed);
	pCcb->status = CONNECTION_STATUS_NOT_READY;
	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;

	while (millis() < endMillis) {
		if (pCcb->status != CONNECTION_STATUS_CONNECTED) {
			if (pCcb->master) {
				if (millis() > reqMillis) {
					sendCR(pCcb->localAddr, pCcb->remoteAddr);
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

bool SerialNode::sendCR(){
	return true;

}

void SerialNode::setReceiveCallBack(void (*ptr)(byte* pData, size_t datasize)) {
	pCallBack = ptr;
}

SerialNode::~SerialNode() {
	// TODO Auto-generated destructor stub
}

void serialRxCallBack(byte* pData, size_t datasize);
