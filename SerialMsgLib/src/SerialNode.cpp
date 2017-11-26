/*
 * SerialNode.cpp
 *
 *  Created on: 23.11.2017
 *      Author: rea
 */
#include <tools.h>
#include "SerialPort.h"
#include "SerialNode.h"
#include "AcbList.h"

SerialTx SerialNode::serialTx;
SerialRx SerialNode::serialRx;
SerialNode* SerialNode::pSerialNodeList = NULL;
unsigned int SerialNode::serialNodeAktId = 0;
AcbList SerialNode::acbList;

void SerialNode::Update(const byte* pMessage, size_t messageSize,
		SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	size_t hsize = sizeof(tSerialHeader);
	size_t dsize = 0;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}

	SerialNode* pNode = GetNodeList();
	while (pNode != NULL && pNode->pCcb->localAddr != pHeader->toAddr) {
		pNode = (SerialNode*) pNode->pNext;

	}
	if (pNode) {
		pNode->setPort(pPort);
		pNode->pCallBack((tSerialHeader*) pHeader,
				(dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize);
	}

}

void SerialNode::Init(size_t maxDataSize) {
	serialRx.setUpdateCallback(Update);
	serialRx.createBuffer(maxDataSize + sizeof(tSerialHeader));
}

unsigned int SerialNode::GetNextAktId() {
	if (serialNodeAktId >= 65535) {
		serialNodeAktId = 0;
	}
	return ++serialNodeAktId;
}

SerialNode* SerialNode::GetNodeList() {
	//return pSerialNodeList;
	return NULL;
}

SerialNode::SerialNode(byte address, SerialPort* pSerialPort) {
	this->pSerialPort = pSerialPort;
	this->pCcb = new tCcb();
	pCcb->localAddr = address;

	SerialNode* pNode = GetNodeList();

	while (pNode && pNode->pNext) {
		pNode = (SerialNode*) pNode->pNext;
	}
	if (!pNode) {
		SerialNode::pSerialNodeList = this;
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
bool SerialNode::isReadyToConnect() {
	return pCcb && (pCcb->status == CONNECTION_STATUS_READY);
}

bool SerialNode::isConnected() {

	return pCcb && (pCcb->status == CONNECTION_STATUS_CONNECTED);
}
void SerialNode::onMessage(tSerialHeader* pSerialHeader, byte* pData,
		size_t datasize) {

	byte cmd = pSerialHeader->cmd;
	tAcb* pAcb = NULL;
	/* for received replies*/

	bool connected = isConnected();

	bool userCall = true;

	if (!connected) {
		if (!(cmd == CMD_ACK || cmd == CMD_NAK || cmd == CMD_CR)
				|| cmd == CMD_AFA) {

			MPRINTSVAL(
					"SerialHeaderRx::internalReceive> unallowed message in unconnected status :",
					pSerialHeader->aktid);
		}
		userCall = false;
		return;
	}

	// protocol requests from remote

	switch (cmd) {

	case CMD_CR:
		if (isReadyToConnect()) { //add and readystatus from CallBackMapper
			send(CMD_ACK, pSerialHeader->aktid);
			pCcb->status = CONNECTION_STATUS_CONNECTED;
		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}
		return;
	case CMD_CD:
		if (connected) {
			send(CMD_ACK, pSerialHeader->aktid);
			pCcb->status = CONNECTION_STATUS_DISCONNECTED;
		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}

		return;
	case CMD_LIVE:
		if (connected) {
			send(CMD_ACK, pSerialHeader->aktid);

		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}
		return;
	case CMD_AFA:
		if (pSerialHeader->toAddr == pCcb->localAddr) {
			send(CMD_ACK, pSerialHeader->aktid);
		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}
		return;
	case CMD_ACD:
	case CMD_ARQ:
		userCall = true;
		return;
	case CMD_ARP:
		userCall = true;
		if (!acbList.deleteAcbEntry(pSerialHeader->aktid)) {
			MPRINTSVAL("acb not found - aktid:", pSerialHeader->aktid);
			userCall = false;
		}
		return;
	case CMD_ACK:

		pAcb = acbList.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				;
				userCall = false;
				pCcb->status = CONNECTION_STATUS_CONNECTED;
			case CMD_CD:
				userCall = false;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
			case CMD_LIVE:
				userCall = false;
			case CMD_AFA:
				userCall = true;

			}
			acbList.deleteAcbEntry(pSerialHeader->aktid);
		}

	case CMD_NAK:
		pAcb = acbList.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				;
				userCall = false;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
			case CMD_CD:
				userCall = false;
			case CMD_LIVE:
				userCall = false;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
			case CMD_AFA:
				userCall = true;
			}
			acbList.deleteAcbEntry(pSerialHeader->aktid);
		}

		if (userCall) {

			pCallBack(pSerialHeader, pData, datasize);
		}
	}

}

// user request /reply

bool SerialNode::connect(byte remoteAddress, bool active, unsigned long timeOut,
		unsigned long reqPeriod) {

	pCcb->remoteAddr = remoteAddress;
	pCcb->status = CONNECTION_STATUS_NOT_READY;

	if (!active) {
		return false;
	}

	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;

	while (millis() < endMillis) {
		if (pCcb->status != CONNECTION_STATUS_CONNECTED) {
			if (pCcb->master) {
				if (millis() > reqMillis) {
					if (active)
						send(CMD_CR);
					reqMillis = millis() + reqPeriod;
				}
			}
		} else {
			MPRINTLN("SerialNode::connect>  connected: ");
			MPRINT(pCcb->localAddr);
			MPRINT(" to ");
			MPRINT(pCcb->remoteAddr);
			return true;
		}

	}
	MPRINTLN("SerialNode::connect: TIMEOUT");
	return false; // no connections or timeout

}

tAktId SerialNode::send(tSerialCmd cmd, tAktId replyOn, byte par, byte* pData,
		byte datasize, byte replyTo) {
	tSerialHeader header;

// not connected
	if (pCcb->status != CONNECTION_STATUS_CONNECTED
			&& !(cmd == CMD_CR || cmd == CMD_ACK || cmd == CMD_NAK)) {
		MPRINTSVAL("SerialNode::send > not connected, cmd not allowed : ", cmd);
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

	acbList.createOrUseAcb(cmd, header.fromAddr, header.toAddr, header.aktid);

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
		SerialPort* pPort = SerialPort::pSerialPortList;
		if (!pPort) {
			return 0;
		}

		while (pPort != NULL) {
			serialTx.setPort(pSerialPort);
			serialTx.sendPreamble();
			serialTx.sendRawData((byte*) &header, sizeof(tSerialHeader));
			if (pData && datasize > 0) {
				serialTx.sendRawData((byte*) pData, datasize);
			}
			serialTx.sendPostamble();
			pPort = (SerialPort*) pPort->pNext;
		}
		return header.aktid;
	}

	return 0;
}

void SerialNode::setReceiveCallBack(
		void (*ptr)(tSerialHeader* pHeader, byte* pData, size_t datasize)) {
	pCallBack = ptr;
}

