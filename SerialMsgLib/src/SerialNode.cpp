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
#include "SoftSerialPort.h"

tAddr fromAddr;
tAddr toAddr;
tAktId aktid = 0;
byte cmd = 0;
byte par = 0;

byte SerialNode::systemId = 0;
unsigned long SerialNode::lastLiveCheckTimeStamp = 0;
SerialNode* SerialNode::pSerialNodeList = NULL;
AcbList SerialNode::acbList;
LcbList SerialNode::lcbList;

void SerialNode::update(const byte* pMessage, size_t messageSize,
		SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	MPRINTLNSVAL("SerialNode::update> size: ", messageSize);
	assert(pHeader);
	PRINTLNHEADER(pHeader);

	size_t hsize = sizeof(tSerialHeader);
	size_t dsize = 0;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}
	assert(messageSize >= hsize);

	if (pHeader->toAddr.sysId == 0) {
		MPRINTLNS("SerialNode::update> systemId must be > 0 ");
		return;
	}

	if (pHeader->toAddr.sysId == SerialNode::systemId) {
		//local system
		SerialNode* pNode = getNodeList();
		if (!pNode) {
			MPRINTLNS("SerialNode::update> no nodes found");
			assert(false);
		}

		if (pHeader->toAddr.nodeId > 0) {
			// find node in our system
			while (pNode) {
				if (pNode->pCcb->localAddr == pHeader->toAddr) {
					// node found
					MPRINTLNS("SerialNode::update> node found");
					pNode->onMessage(pHeader,
							(dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize,
							pPort, pNode);
					return;
				}
				pNode = (SerialNode*) pNode->pNext;
			}
			MPRINTLNS("SerialNode::update> node not found : ");
			PRINTLNHEADER(pHeader);
			return;
		}

		assert(pHeader->toAddr.nodeId == 0);
		// send CR to first found passive but ready(to connect) node without remote node id
		// and whithout remote system id or equal system id to headers from system id

		if (pHeader->cmd == CMD_CR) {
			SerialNode* pNode = getNodeList();
			while (pNode) {
				if (!pNode->isActive() && pNode->isReadyToConnect()
						&& (pNode->pCcb->remoteAddr.sysId == 0
								|| pNode->pCcb->remoteAddr.sysId
										== pHeader->fromAddr.sysId)
						&& pNode->pCcb->remoteAddr.nodeId == 0) {// only as comment
					SerialNode::onMessage(pHeader,
							(dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize,
							pPort, pNode);
					return;
				}
				pNode = (SerialNode*) pNode->pNext;
			}
			MPRINTLNS("SerialNode::update> no ready node for CR found");
			return;
		} else {
			MPRINTLNS(
					"SerialNode::update> message without nodeId in toAddr received");
			return;
		}
		// end of local system
	}
	{
		//remote system
		forward(pMessage, messageSize, pPort);
	}
}

bool SerialNode::forward(const byte* pMessage, size_t messageSize,
		SerialPort* pSourcePort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	MPRINTLNS("SerialNode::forward>");
	PRINTLNHEADER(pHeader);

	SerialPort* pTargetPort = SerialPort::getPort(pHeader->toAddr.sysId);

	if (!pTargetPort) {
		// find link
		pTargetPort = lcbList.getTargetPort(pHeader);
	}

// port or linked port found
	if (pTargetPort) {
		pTargetPort->getTx()->sendData(pMessage, messageSize);
		return true;
	}

// if CR create a link an send to all ports without ports to the source system

	if (pHeader->cmd != CMD_CR) { //create a link only by connection request
		MPRINTSVAL("SerialNode::Update> no link found to system: ",
				pHeader->toAddr.sysId);
		return false;
	}

	if (lcbList.createLcb(pHeader, pSourcePort)) {
		MPRINTS("SerialNode::Update> LINK created: ");
		PRINTLNADDR(pHeader->fromAddr);
	}

	SerialPort* pport = SerialPort::pSerialPortList;

	byte cnt = 0;
	while (pport && pport->remoteSysId != pSourcePort->remoteSysId) {
		pport->getTx()->sendData(pMessage, messageSize);
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

void SerialNode::init(byte systemId) {
	SerialNode::systemId = systemId;
	MPRINTLNSVAL("SerialNode::Init> systemId: ", systemId);
}

SerialNode* SerialNode::getNodeList() {
	return pSerialNodeList;
}

SerialNode* SerialNode::createNode(byte localNodeId, bool active,
		byte remoteSysId, byte remoteNodeId, SerialPort* pSerialPort) {
	return new SerialNode(localNodeId, active, remoteSysId, remoteNodeId,
			pSerialPort);
}

SerialNode::SerialNode(byte localNodeId, bool active, byte remoteSysId,
		byte remoteNodeId, SerialPort* pSerialPort) {

	MPRINTLNS("SerialNode::SerialNode>");
	ASSERTP(systemId > 0,
			"please call System.init(yourSystemId) before you create nodes.");

	this->pSerialPort = pSerialPort;
	this->pCcb = new tCcb();
	tAddr lAddr(systemId, localNodeId);
	tAddr rAddr(remoteSysId, remoteNodeId);

	pCcb->localAddr = lAddr;
	pCcb->remoteAddr = rAddr;
	setActive(active);

	SerialNode* pNode = getNodeList();

	while (pNode && pNode->pNext) {
		pNode = (SerialNode*) pNode->pNext;
	}
	if (!pNode) {
		SerialNode::pSerialNodeList = this;
	} else {
		pNode->pNext = this;
	}
	MPRINTS("SerialNode:SerialNode> created : ");
	PRINTLNADDR(pCcb->localAddr);

}

SerialNode::~SerialNode() {
	if (this->pNext) {
		SerialNode* pPrev = pSerialNodeList;
		while (pPrev && pPrev->pNext) {
			if (pPrev->pNext == this) {
				break;
			}
			pPrev = (SerialNode*) pPrev->pNext;
		}
		if (pPrev) { // set next of previous to next of this node
			pPrev->pNext = this->pNext;
		}
	}
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
	this->pCcb->active = bActive;
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
	bool isConnected = pCcb
			&& (pSerialPort && pCcb->status == CONNECTION_STATUS_CONNECTED);
	return isConnected;
}
bool SerialNode::isLifeCheckLate() {

	return (millis() - lastReceiveTimeStamp)
			> SERIALNODE_TIME_LIFECHECK_LATE_MSEC;
}

bool SerialNode::isLifeCheckExpired() {
	return (millis() - lastReceiveTimeStamp)
			> SERIALNODE_TIME_LIFECHECK_EXPIRED_MSEC;
}

void SerialNode::onMessage(tSerialHeader* pSerialHeader, const byte* pData,
		size_t datasize, SerialPort* pPort, SerialNode* pNode) {

	MPRINTLNS("SerialNode::onMessage>");
	assert(pSerialHeader);
	PRINTLNHEADER(pSerialHeader);
	assert(pData ? datasize > 0 : datasize == 0);
	assert(pPort);
	assert(pNode->pCcb->remoteAddr.sysId!=systemId);
	pNode->lastReceiveTimeStamp = millis();

	byte cmd = pSerialHeader->cmd;
	tAcb* pAcb = NULL;
	/* for received replies*/

	bool connected = pNode->isConnected();
	bool userCall = true;
	bool acbNotFound = false;

	if (!connected) {
		if (!(cmd == CMD_ACK || cmd == CMD_NAK || cmd == CMD_CR)
				|| cmd == CMD_AFA) {
			MPRINTLNSVAL(
					"SerialNode::onMessage> unallowed message, status <> connected, aktid  :",
					pSerialHeader->aktid);
			return;
		}
	}

	// protocol requests from remote

	switch (cmd) {

	case CMD_CR:
		MPRINTLNS("SerialNode::onMessage> CMD_CR");
		if (!pNode->isActive() && pNode->isReadyToConnect()) { //if passiv and ready to be connected

			if (pNode->pCcb->remoteAddr.sysId == 0) {
				// connect, no matter who it is
				pNode->pCcb->remoteAddr = pSerialHeader->fromAddr;
				MPRINTLNS("pcb remote address:");
				PRINTLNADDR(pNode->pCcb->remoteAddr);
			} else if (pNode->pCcb->remoteAddr != pSerialHeader->fromAddr) {
				// unknown node wants to connect, send NAK
				MPRINTSVAL(
						"SerialNode::onMessage> unknown remote node wants to node: ",
						pNode->pCcb->localAddr.nodeId);
				MPRINTLNS(
						"SerialNode::onMessage> unknown remote node address:");
				PRINTLNADDR(pNode->pCcb->remoteAddr);
				pNode->send(CMD_NAK, pSerialHeader->aktid);

			}
			pNode->pSerialPort = pPort;
			pNode->pCcb->status = CONNECTION_STATUS_CONNECTED;
			pNode->pCcb->remoteAddr = pSerialHeader->fromAddr;

			MPRINTLNS("CMD_ACK -> remote address:");
			PRINTLNADDR(pNode->pCcb->remoteAddr);
			pNode->send(CMD_ACK, pSerialHeader->aktid);
			MPRINTLNSVAL(
					"SerialNode::onMessage> CONNECTION_STATUS_CONNECTED node : ",
					pNode->pCcb->localAddr.nodeId);

		} else {
			if (pNode->isActive()) {
				MPRINTLNSVAL(
						"SerialNode::onMessage> node is active, send NAK : ",
						pNode->pCcb->localAddr.nodeId);
			} else if (pNode->isConnected()) {
				MPRINTLNSVAL(
						"SerialNode::onMessage>node is already connected, send NAK : ",
						pNode->pCcb->localAddr.nodeId);

			} else if (!pNode->isReadyToConnect()) {
				MPRINTLNSVAL(
						"SerialNode::onMessage> node is not ready, send NAK : ",
						pNode->pCcb->localAddr.nodeId);
			}

			//send(CMD_NAK,223,0,0,0,0,4);return;
			pNode->send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_CD:
		if (connected) {
			pNode->send(CMD_ACK, pSerialHeader->aktid);
			pNode->pSerialPort = NULL;
			pNode->pCcb->status = CONNECTION_STATUS_DISCONNECTED;
			MPRINTLNSVAL("SerialNode::onMessage> node disconnected : ",
					pNode->pCcb->localAddr.nodeId);
		} else {
			pNode->send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_LIVE:
		if (connected) {
			pNode->send(CMD_ACK, pSerialHeader->aktid);

		} else {
			pNode->send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_AFA:
		if (pSerialHeader->toAddr == pNode->pCcb->localAddr) {
			pNode->send(CMD_ACK, pSerialHeader->aktid);
		} else {
			pNode->send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_ACD:
	case CMD_ARQ:
		break;
	case CMD_ARP:
		pAcb = SerialNode::acbList.getAcbEntry(pSerialHeader->aktid);
		if (!pAcb) {
			userCall = false;
			acbNotFound = true;
		}
		break;
	case CMD_ACK:
		pAcb = SerialNode::acbList.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				userCall = false;
				pNode->pSerialPort = pPort;
				if (!pNode->isActive()) {
					MPRINTSVAL(
							"SerialNode::onMessage>ACK ON CR node is not active: ",
							pNode->pCcb->localAddr.nodeId);
				} else if (pNode->isConnected()) {
					MPRINTLNSVAL(
							"SerialNode::onMessage>node is already connected: ",
							pNode->pCcb->localAddr.nodeId);

				} else if (!pNode->isReadyToConnect()) {
					MPRINTSVAL(
							"SerialNode::onMessage>ACK ON CR node is not ready: ",
							pNode->pCcb->localAddr.nodeId);

				} else {
					pNode->pCcb->remoteAddr= pSerialHeader->fromAddr;
					pNode->pCcb->status = CONNECTION_STATUS_CONNECTED;
					pNode->pSerialPort=pPort;
					MPRINTLNSVAL(
							"SerialNode::onMessage> CONNECTION_STATUS_CONNECTED node (active) : ",
							pNode->getId());
					PRINTLNADDR(pNode->pCcb->localAddr)
				}
				;
				break;
			case CMD_CD:
				userCall = false;
				pNode->pSerialPort = NULL;
				pNode->pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				MPRINTS("SerialNode::onMessage> node disconnected : ");
				PRINTLNADDR(pNode->pCcb->localAddr)
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
		pAcb = SerialNode::acbList.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				MPRINTSVAL("SerialNode::onMessage> NAK ON CR : ",
						pNode->pCcb->localAddr.nodeId)
				;

				userCall = false;
				pNode->pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				break;
			case CMD_CD:
				userCall = false;
				break;
			case CMD_LIVE:
				userCall = false;
				pNode->pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				break;
			case CMD_AFA: //application
			case CMD_ARQ:
			case CMD_ACD:
				break;
			}
		} else {
			acbNotFound = true;
		}
	} // end of switch

	if (acbNotFound) {
		MPRINTSVAL("SerialNode::onMessage> acb not found - aktid:", pSerialHeader->aktid);
		userCall = false;
	}

	if (userCall) {
		if (pNode->pCallBack) {
			pNode->pCallBack(pSerialHeader, pData, datasize);
		}
	}
	if (pAcb) {
		// it fucks off WHY ???????

		MPRINTLNSVAL("SerialNode::onMessage> ACB COUNT : " ,SerialNode::acbList.count());
		SerialNode::acbList.deleteAcbEntry(pSerialHeader->aktid);
	}

}

// user request /reply

bool SerialNode::areAllNodesConnected() {
	MPRINTLNS("SerialNode::areAllNodesConnected>");
	SerialNode* pNode = SerialNode::pSerialNodeList;
	bool connected = true;

	while (pNode) {
		connected = connected && pNode->isConnected();
		pNode = (SerialNode*) pNode->pNext;
	}
	return connected;
}

bool SerialNode::connectNodes(unsigned long timeOut, unsigned long reqPeriod) {
	MPRINTLNS("SerialNode::connectNodes>");
	SerialNode* pNode = SerialNode::pSerialNodeList;
	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;
	bool connected = areAllNodesConnected();

	if (pNode && !connected) {
		while ((millis() <= endMillis || timeOut == 0) && !connected) {
			SerialRx::readNextOnAllPorts();
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
		if (pNode) {
			MPRINTLNS("SerialNode::connectNodes> all nodes connected");
		} else {
			MPRINTLNS("SerialNode::connectNodes> no nodes to connect");
		}
		connected = true;
	}

	return connected;
}

void SerialNode::reconnect() {

	setReady(true);
	if (isActive()) {
		MPRINTLNSVAL("SerialNode:reconnect()> send CR for node ", getId());
		send(CMD_CR);
	}else{
		MPRINTLNSVAL("SerialNode:reconnect()> waiting for CR message, node: ", getId());
	}
}

bool SerialNode::checkLifeNodes(unsigned long period) {

	SerialNode* pNode = SerialNode::pSerialNodeList;
	if ((millis() - SerialNode::lastLiveCheckTimeStamp) > period) {


		SerialNode::lastLiveCheckTimeStamp = millis();
		while (pNode) {
			if (!pNode->isConnected()){
				pNode->reconnect();
			}else if (pNode->isLifeCheckExpired()) {
				MPRINTLNSVAL("SerialNode::checkLifeNodes> isLifeCheckExpired for node : ", pNode->getId());
				pNode->reconnect();
			} else if (pNode->isLifeCheckLate()) {
				MPRINTLNSVAL("SerialNode::checkLifeNodes> isLate for node : ", pNode->getId());
				if (pNode->isConnected()) {
					if (pNode->isActive()) {
						pNode->send(CMD_LIVE);
					}
				} else {
					pNode->reconnect();
				}
			}
			pNode = (SerialNode*) pNode->pNext;
		}
	}
}

static void SerialNode::processNodes(bool lifeCheck,
		unsigned long lifeCheckPeriodMsec) {
	if (lifeCheck) {
		checkLifeNodes(lifeCheckPeriodMsec);
	}
	SerialRx::readNextOnAllPorts();
}

bool SerialNode::connect(byte remoteSysId, byte remoteNodeId,
		unsigned long timeOut, unsigned long checkPeriod) {

	MPRINTLNS("SerialNode::connect>");
	pCcb->remoteAddr.sysId =
			(remoteSysId > 0) ? remoteSysId : pCcb->remoteAddr.sysId;
	pCcb->remoteAddr.nodeId =
			(remoteNodeId > 0) ? remoteNodeId : pCcb->remoteAddr.nodeId;

	ASSERTP(isActive() ? pCcb->remoteAddr.sysId > 0 :true,
			"unknown remote system id, must be > 0 for active nodes");

	setReady(true);
	unsigned long endMillis = millis() + timeOut;
	unsigned long reqMillis = 0;
	MPRINTLNSVAL("SerialNode::connect> node: ", pCcb->localAddr.nodeId);
	if (isActive()) {
		MPRINTLNS("SerialNode::connect> node is active");
	} else {
		MPRINTLNS("SerialNode::connect> wait for connection request");
	}

	while ((timeOut == 0) || millis() <= endMillis) {
		SerialRx::readNextOnAllPorts(); // receive next byte
		if (!isConnected()) {
			if (isActive()) {
				if (millis() > reqMillis) {
					MPRINTLNS("SerialNode::connect>  try to connect... ");
					send(CMD_CR);
					reqMillis = millis() + checkPeriod;
				}
			}
		} else {
			MPRINTLNS("SerialNode::connect>  connected: ");
			PRINTLNADDR(pCcb->localAddr);
			MPRINTS(" to ");
			PRINTLNADDR(pCcb->remoteAddr);
			return true;
		}

	}
	if (timeOut > 0) {
		MPRINTLNS("SerialNode::connect> TIMEOUT");
		acbList.deleteAcbEntry(pCcb, CMD_CR);
	}

	return false; // no connections or timeout

}

tAktId SerialNode::send(tSerialCmd cmd, tAktId replyOn, byte par, byte* pData,
		byte datasize, byte replyToSys, byte replyToNode) {

	tSerialHeader header, *pHeader;
	pHeader = &header;
	MPRINTLNSVAL("SerialNode::send> cmd: ", cmd);
	assert(pCcb->remoteAddr.sysId!=systemId);

// not connected
	if (pCcb->status != CONNECTION_STATUS_CONNECTED
			&& !(cmd == CMD_CR || cmd == CMD_ACK || cmd == CMD_NAK)) {
		MPRINTSVAL("SerialNode::send > not connected, cmd not allowed : ", cmd);
		return 0;
	}
	header.fromAddr.sysId = SerialNode::systemId;
	if (replyOn > 0) {
		MPRINTLNSVAL("SerialNode::send> reply to:", replyOn);
		header.aktid = replyOn;
		if (replyToSys > 0) {
			header.toAddr.sysId = replyToSys;
			header.toAddr.nodeId = replyToNode;

		} else {
			//MPRINTLNSVAL("Address of node: ", long((void* ) this));
			if (this->isConnected()) {
				header.toAddr = pCcb->remoteAddr;
			} else {
				MPRINTLNS(
						"SerialNode::send> node, not connected , unknown remote adress:");
				return 0;
			}

		}
	} else {
		header.aktid = 0;
		header.toAddr = pCcb->remoteAddr;
	}

	header.fromAddr = pCcb->localAddr;

	header.cmd = cmd;
	header.par = par;
	MPRINTLNS("SerialNode::send> header ...");

	if (pSerialPort) { //connected or port was preset
		SerialNode::writeToPort(pHeader, pData, datasize, pSerialPort);
		MPRINTLNS("SerialNode::send> to node's port - success");
		return header.aktid;
	} else { // not connected

		SerialPort* pPort = SerialPort::getPort(header.toAddr.sysId);
		if (pPort) {
			SerialNode::writeToPort(pHeader, pData, datasize, pPort);
			MPRINTLNS("SerialNode::send> to header toAddr port - success");
			return header.aktid;
		} else {
			pPort = SerialPort::pSerialPortList;
			if (!pPort) {
				MPRINTLNS("SerialNode::send> no port in list !");
				return 0;
			}
			if (pPort) {
				MPRINTLNS("SerialNode::send> send to all ports ...");
				while (pPort) {
					SerialNode::writeToPort(pHeader, pData, datasize, pPort);
					pPort = (SerialPort*) pPort->pNext;
				}
				MPRINTLNS("SerialNode::send> to port in list - success");
				return header.aktid;
			}
		}
	}
	MPRINTLNS("SerialNode::send> failed");
	return 0;
}

tAktId SerialNode::writeToPort(tSerialHeader* pHeader, byte* pData,
		size_t datasize, SerialPort* pPort) {
	MPRINTLNSVAL("SerialNode::writeToPort> send to port: ", pPort->remoteSysId);
	assert(pHeader);
	assert(pHeader->toAddr.sysId!=pHeader->fromAddr.sysId);

	if (pHeader->aktid == 0) { // we need an aktId
		acbList.createOrUseAcb(pHeader);
	}

	PRINTLNHEADER(pHeader);

	pPort->getTx()->sendPreamble();
	pPort->getTx()->sendRawData((byte*) pHeader, sizeof(tSerialHeader));
	if (pData && datasize > 0) {
		pPort->getTx()->sendRawData((byte*) pData, datasize);
	}
	pPort->getTx()->sendPostamble();

	if (pData) {
		MPRINTLNSVAL("datasize: ", datasize);
	}
	MPRINTFREE
	;
	MPRINTLNS("SerialNode::writeToPort success");
	return pHeader->aktid;
}

bool waitOnReply(tAktId aktId, unsigned long timeout = 500) {

	MPRINTLNS("SerialNode::waitOnReply");

}

void SerialNode::setReceiveCallBack(
		void (*ptr)(const tSerialHeader* pHeader, const byte* pData,
				size_t datasize)) {
	pCallBack = ptr;
}

