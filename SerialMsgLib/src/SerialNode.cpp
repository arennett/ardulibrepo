/*
 * SerialNode.cpp
 *
 *  Created on: 23.11.2017
 *      Author: rea
 */
#include <tools.h>
#include "SerialRx.h"
#include "SerialPort.h"
#include "SerialNode.h"
#include "AcbList.h"

#define PRINTLNADDR(x) MPRINT(x.sysId);MPRINT(".");MPRINTLN(x.nodeId);

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
	//search node
	while (pNode != NULL && pNode->pCcb->localAddr != pHeader->toAddr) {
		pNode = (SerialNode*) pNode->pNext;

	}

	if (pNode) {
		pNode->onMessage(pHeader, (dsize > 0) ? (byte*) pHeader + hsize : NULL,
				dsize, pPort);
	} else {
		// node not found must be in a remote system
		forward(pMessage, messageSize, pPort);
	}
}

bool SerialNode::forward(const byte* pMessage, size_t messageSize,
		SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	//check links to find port

	// if no link found create a tLcb

	//check open links if reply
	//from S4aktid/ aktId to S4   open

	// forward message to other systems
	SerialPort* pport = SerialPort::pSerialPortList;
	while (pport && pport->remoteSysId != pPort->remoteSysId) {
		serialTx.setPort(pport);
		serialTx.sendData(pMessage, messageSize);
		pport = (SerialPort*) pport->pNext;
	}
	if (pport) {

		// add open link
		//from S1/ aktId to S4   open

	} else {
		MPRINT("SerialNode::Update> forward failed for node: ");
		PRINTLNADDR(pHeader->toAddr);
	}

}

void SerialNode::Init(byte systemId) {
	SerialNode::systemId = systemId;
	serialRx.setUpdateCallback(Update);
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

SerialNode::SerialNode(byte localNodeId, byte remoteSysId, byte remoteNodeId,
		SerialPort* pSerialPort) {
	this->pSerialPort = pSerialPort;
	this->pCcb = new tCcb();
	tAddr lAddr(systemId, localNodeId);
	tAddr rAddr(remoteSysId, remoteNodeId);

	pCcb->localAddr = lAddr;
	pCcb->remoteAddr = rAddr;
	pCcb->active = (rAddr.sysId > 0) ? true : false;

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

void SerialNode::setReady(bool bReady) {
	if (bReady) {
		this->pCcb->status = CONNECTION_STATUS_READY;
	} else {
		this->pCcb->status = CONNECTION_STATUS_NOT_READY;
	}
}

bool SerialNode::setActive(bool bActive) {
	if (bActive && pCcb->remoteAddr.sysId > 0) {
		this->pCcb->active = true;
	} else {
		this->pCcb->active = false;
	}
	setReady(false);
	return this->pCcb->active;
}

bool SerialNode::isActive() {
	return this->pCcb->active;
}

bool SerialNode::isReadyToConnect() {
	return pCcb && (pCcb->status == CONNECTION_STATUS_READY);
}

bool SerialNode::isConnected() {

	return pCcb && (pCcb->status == CONNECTION_STATUS_CONNECTED);
}
void SerialNode::onMessage(tSerialHeader* pSerialHeader, byte* pData,
		size_t datasize, SerialPort* pPort) {

	byte cmd = pSerialHeader->cmd;
	tAcb* pAcb = NULL;
	/* for received replies*/

	bool connected = isConnected();
	bool userCall = true;
	bool acbNotFound = false;

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

			if (pCcb->remoteAddr.sysId == 0) {
				// first connect
				pCcb->remoteAddr = pSerialHeader->fromAddr;
			} else if (pCcb->remoteAddr != pSerialHeader->fromAddr) {
				// stranger wants to connect -> no
				send(CMD_NAK, pSerialHeader->aktid);
				return;
			}
			this->pSerialPort = pPort;
			pCcb->status = CONNECTION_STATUS_CONNECTED;
			send(CMD_ACK, pSerialHeader->aktid);
			MPRINT("SerialNode::onMessage> node connected : ");
			PRINTLNADDR(pCcb->localAddr);

		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_CD:
		if (connected) {
			send(CMD_ACK, pSerialHeader->aktid);
			this->pSerialPort = NULL;
			pCcb->status = CONNECTION_STATUS_DISCONNECTED;
		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_LIVE:
		if (connected) {
			send(CMD_ACK, pSerialHeader->aktid);

		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_AFA:
		if (pSerialHeader->toAddr == pCcb->localAddr) {
			send(CMD_ACK, pSerialHeader->aktid);
		} else {
			send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_ACD:
	case CMD_ARQ:
		break;
	case CMD_ARP:
		pAcb = acbList.getAcbEntry(pSerialHeader->aktid);
		if (!pAcb) {
			userCall = false;
			acbNotFound = true;
		}
		break;
	case CMD_ACK:
		pAcb = acbList.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				userCall = false;
				this->pSerialPort = pPort;
				pCcb->status = CONNECTION_STATUS_CONNECTED;
				MPRINT("SerialNode::onMessage> active node connected : ");
				PRINTLNADDR(pCcb->localAddr)
				;
				break;
			case CMD_CD:
				userCall = false;
				this->pSerialPort = NULL;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				MPRINT("SerialNode::onMessage> node disconnected : ");
				PRINTLNADDR(pCcb->localAddr)
				;
				break;
			case CMD_LIVE:
				userCall = false;
				break;
			case CMD_AFA: //application
			case CMD_ARQ:
			case CMD_ACD:
				break;
				//nothing to do
			}
		} else {
			acbNotFound = true;
		}
		break;
	case CMD_NAK:
		pAcb = acbList.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				;
				userCall = false;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				break;
			case CMD_CD:
				userCall = false;
				break;
			case CMD_LIVE:
				userCall = false;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				break;
			case CMD_AFA: //application
			case CMD_ARQ:
			case CMD_ACD:
				break;
			}
		} else {
			acbNotFound = true;
		}
	}

	if (acbNotFound) {
		MPRINTSVAL("acb not found - aktid:", pSerialHeader->aktid);
		userCall = false;
	}

	if (userCall) {
		pCallBack(pSerialHeader, pData, datasize);
	}
	if (pAcb) {
		acbList.deleteAcbEntry(pSerialHeader->aktid);
	}

}

// user request /reply

bool SerialNode::connectNodes(unsigned long timeOut, unsigned long reqPeriod) {

	SerialNode* pNode = SerialNode::pSerialNodeList;
	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;
	bool connected = false;

	if (pNode) {
		while (millis() <= endMillis && !connected) {
			serialRx.readNextOnAllPorts();
			connected = true;
			while (pNode) {
				connected = connected && pNode->isConnected();
				if (!pNode->isConnected()) {
					if (pNode->isReadyToConnect()) {

						if (pNode->isActive()) {
							if (millis() > reqMillis) {
								pNode->send(CMD_CR);
								reqMillis = millis() + reqPeriod;
							}
						}

					} //is ready
				}
				pNode = (SerialNode*) pNode->pNext;
			}
		}
	} else {
		connected = false;
	}

	return connected;
}

bool SerialNode::connectNode(tAddr remoteAddress, unsigned long timeOut,
		unsigned long reqPeriod) {

	pCcb->remoteAddr =
			(remoteAddress.sysId > 0) ? remoteAddress : pCcb->remoteAddr;

	setActive(pCcb->remoteAddr.sysId > 0);

	pCcb->status = CONNECTION_STATUS_NOT_READY;

	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;

	while (millis() <= endMillis) {
		serialRx.readNextOnAllPorts(); // receive next byte
		if (pCcb->status != CONNECTION_STATUS_CONNECTED) {
			if (pCcb->active) {
				if (millis() > reqMillis) {
					send(CMD_CR);
					reqMillis = millis() + reqPeriod;
				}
			}
		} else {
			MPRINTLN("SerialNode::connect>  connected: ");
			PRINTLNADDR(pCcb->localAddr);
			MPRINT(" to ");
			PRINTLNADDR(pCcb->remoteAddr);
			return true;
		}

	}
	if (timeOut > 0) {
		MPRINTLN("SerialNode::connect: TIMEOUT");
	}

	return false; // no connections or timeout

}

tAktId SerialNode::send(tSerialCmd cmd, tAktId replyOn, byte par, byte* pData,
		byte datasize, byte replyToSys, byte replyToNode) {
	tSerialHeader header;

	tAddr replyTo(replyToSys, replyToNode);

// not connected
	if (pCcb->status != CONNECTION_STATUS_CONNECTED
			&& !(cmd == CMD_CR || cmd == CMD_ACK || cmd == CMD_NAK)) {
		MPRINTSVAL("SerialNode::send > not connected, cmd not allowed : ", cmd);
		return 0;
	}

	header.fromAddr.sysId = SerialNode::systemId;

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

