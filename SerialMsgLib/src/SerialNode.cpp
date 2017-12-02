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

#define PRINTADDR(x) MPRINT(x.sysId);MPRINTS(".");MPRINT(x.nodeId);
#define PRINTLNADDR(x) PRINTADDR(x);MPRINTLNS("");

#define PRINTHEADER(pH) PRINTLNADDR(pH->fromAddr);MPRINTS(" to ");PRINTLNADDR(pH->toAddr);\
						MPRINTSVAL("aktId: ",pH->aktid);MPRINTSVAL(" cmd: ",pH->cmd);MPRINTSVAL(" par: ",pH->cmd);
#define PRINTLNHEADER(pH);MPRINTLNS("");

tAddr fromAddr;
	tAddr toAddr;
	tAktId aktid=0;
	byte cmd=0;
	byte par=0;


byte SerialNode::systemId=0;
SerialTx SerialNode::serialTx;
SerialRx SerialNode::serialRx;
SerialNode* SerialNode::pSerialNodeList = NULL;
unsigned int SerialNode::serialNodeAktId = 0;
AcbList SerialNode::acbList;
LcbList SerialNode::lcbList;


void SerialNode::Update(const byte* pMessage, size_t messageSize,
		SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	size_t hsize = sizeof(tSerialHeader);
	size_t dsize = 0;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}

	SerialNode* pNode = GetNodeList();

	if (pHeader->toAddr.sysId==SerialNode::systemId) {
		//local system
		while (pNode != NULL && pNode->pCcb->localAddr != pHeader->toAddr) {
			pNode = (SerialNode*) pNode->pNext;

		}
		if (pNode) {
			pNode->onMessage(pHeader, (dsize > 0) ? (byte*) pHeader + hsize : NULL,
		          			dsize, pPort);
		}
	} else {
		//remote system
		// find port
		forward(pMessage, messageSize, pPort);
	}
}

bool SerialNode::forward(const byte* pMessage, size_t messageSize,
		SerialPort* pSourcePort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;


	SerialPort* pTargetPort =SerialPort::getPort(pHeader->toAddr.sysId);


	if (!pTargetPort) {
		// find link
		pTargetPort=lcbList.getTargetPort(pHeader);
	}

	// port or linked port found
	if (pTargetPort) {
		serialTx.setPort(pTargetPort);
		serialTx.sendData(pMessage, messageSize);
		return true;
	}


	// if CR create a link an send to all ports without ports to the source system

	if (pHeader->cmd != CMD_CR) { //create a link only by connection request
			MPRINTSVAL("SerialNode::Update> no link found to system: " ,pHeader->toAddr.sysId);
			return false;
	}

	if(lcbList.createLcb(pHeader, pSourcePort)){
		MPRINTS("SerialNode::Update> LINK created: ");
		PRINTLNADDR(pHeader->fromAddr);
	}

	SerialPort* pport = SerialPort::pSerialPortList;

	byte cnt=0;
	while (pport && pport->remoteSysId != pSourcePort->remoteSysId) {
		serialTx.setPort(pTargetPort);
		serialTx.sendData(pMessage, messageSize);
		pport = (SerialPort*) pport->pNext;
		++cnt;
	}
	if (cnt) {
		return true;
	}
	MPRINTS("SerialNode::Update> forward for CR failed: ");
	PRINTLNADDR(pHeader->fromAddr);
	PRINTLNADDR(pHeader->toAddr);
	return false;


}

void SerialNode::Init(byte systemId) {
	SerialNode::systemId = systemId;
	serialRx.setUpdateCallback(Update);
	MPRINTSVAL("SerialNode::Init> systemId: ",systemId);
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
	MPRINTLNSVAL("SerialNode:SerialNode> created id: ",pCcb->localAddr.nodeId);
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
    // if port was set, the node can only connect over that port
	// but the node is not connected before the the status is set to connected
	return pCcb && (pSerialPort && pCcb->status == CONNECTION_STATUS_CONNECTED );
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
			MPRINTS("SerialNode::onMessage> node connected : ");
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
				MPRINTS("SerialNode::onMessage> active node connected : ");
				PRINTLNADDR(pCcb->localAddr)
				;
				break;
			case CMD_CD:
				userCall = false;
				this->pSerialPort = NULL;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				MPRINTS("SerialNode::onMessage> node disconnected : ");
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

bool SerialNode::connect(byte remoteSysId,byte remoteNodeId, unsigned long timeOut,
		unsigned long checkPeriod) {

	pCcb->remoteAddr.sysId =(remoteSysId> 0) ? remoteSysId : pCcb->remoteAddr.sysId;
	pCcb->remoteAddr.nodeId =(remoteNodeId> 0) ? remoteNodeId : pCcb->remoteAddr.nodeId;

	setActive(pCcb->remoteAddr.sysId > 0);

	pCcb->status = CONNECTION_STATUS_READY;

	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;
    MPRINTLNSVAL("SerialNode::connect> node: " ,pCcb->localAddr.nodeId);
    if(isActive()) {
    	MPRINTLNS("SerialNode::connect> node is active");
    }


	while ((timeOut == 0) || millis() <= endMillis) {
		serialRx.readNextOnAllPorts(); // receive next byte
		if (pCcb->status != CONNECTION_STATUS_CONNECTED) {
			if (pCcb->active) {
				if (millis() > reqMillis) {
					send(CMD_CR);
					reqMillis = millis() + checkPeriod;
				}
			}
		} else {
			MPRINTLN("SerialNode::connect>  connected: ");
			PRINTLNADDR(pCcb->localAddr);
			MPRINTS(" to ");
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
		header.aktid = 0;
		header.toAddr = pCcb->remoteAddr;
	}


	header.fromAddr = pCcb->localAddr;

	header.cmd = cmd;
	header.par = par;



	if (pSerialPort) { //connected or port was preset
		SerialNode::writeToPort(&header,pData,datasize,pSerialPort);
		return header.aktid;
	} else { // not connected
		MPRINTLNS("SerialNode::send> try all ports...");
		SerialPort* pPort = SerialPort::pSerialPortList;
		if (!pPort) {
			MPRINTLNS("SerialNode::send> no port found!");
			return 0;
		}

		while (pPort != NULL) {
			SerialNode::writeToPort(&header,pData,datasize,pPort);
			pPort = (SerialPort*) pPort->pNext;
		}
		return header.aktid;
	}

	return 0;
}


tAktId SerialNode::writeToPort(tSerialHeader* pHeader,byte* pData, size_t datasize ,SerialPort* pPort){
	if (pHeader->aktid==0) {// we need an aktId
		pHeader->aktid = GetNextAktId();
		acbList.createOrUseAcb(pHeader);
	}
	serialTx.setPort(pPort);
	serialTx.sendPreamble();
	serialTx.sendRawData((byte*) &pHeader, sizeof(tSerialHeader));
	if (pData && datasize > 0) {
		serialTx.sendRawData((byte*) pData, datasize);
	}
	serialTx.sendPostamble();
	MPRINTLNSVAL("send to port: ",pPort->remoteSysId);
	PRINTLNHEADER(header);
	if(pData){
		MPRINTLNSVAL("datasize: ",datasize);
	}
}

void SerialNode::setReceiveCallBack(
		void (*ptr)(tSerialHeader* pHeader, byte* pData, size_t datasize)) {
	pCallBack = ptr;
}

