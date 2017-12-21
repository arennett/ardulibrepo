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
void (*SerialNode::pCallBackOnMessage)(const tSerialHeader* pHeader, const byte* pData, size_t datasize,
		SerialNode* pNode)=NULL; // user callback
void (*SerialNode::pCallBackOnPreConnect)(SerialNode* pNode)=NULL; // user callback before node connects

void SerialNode::update(const byte* pMessage, size_t messageSize, SerialPort* pPort) {
	XTIME_PICK;
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	MPRINTLNSVAL("SerialNode::update> size: ", messageSize);
	assert(pHeader);
	MPRINTLNHEADER(pHeader);

	size_t hsize = sizeof(tSerialHeader);
	size_t dsize = 0;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}
	if (cmd >= CMD_NAK && cmd < CMD_ACD) { //System telegramm
		if (dsize > 0) {
			MPRINTLNSVAL("SerialNode::update> message has corrupted size, expected : ", hsize);
			return;
		}
	}

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
					pNode->onMessage(pHeader, (dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize, pPort);
					return;
				}
				pNode = (SerialNode*) pNode->pNext;
			}
			MPRINTLNS("SerialNode::update> node not found : ");
			MPRINTLNHEADER(pHeader);
			return;
		}

		// send CR to first found passive but ready(to connect) node without remote node id
		// and whithout remote system id or equal system id to headers from system id

		if (pHeader->cmd == CMD_CR) {
			SerialNode* pNode = getNodeList();
			while (pNode) {
				if (!pNode->isActive() && pNode->isReadyToConnect()
						&& (pNode->pCcb->remoteAddr.sysId == 0
								|| pNode->pCcb->remoteAddr.sysId == pHeader->fromAddr.sysId)
						&& pNode->pCcb->remoteAddr.nodeId == 0) {		// only as comment
					pNode->onMessage(pHeader, (dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize, pPort);
					return;
				}
				pNode = (SerialNode*) pNode->pNext;
			}
			MPRINTLNS("SerialNode::update> no ready node for CR found");
			return;
		} else {
			MPRINTLNS("SerialNode::update> message without nodeId in toAddr received");
			return;
		}
		// end of local system
	}

	if (pHeader->fromAddr.sysId == systemId) {
		MPRINTLNS("SerialNode::update> remote system has the same id as local system , message discarded");
		return;
	}

	//remote system
	forward(pMessage, messageSize, pPort);

}

bool SerialNode::forward(const byte* pMessage, size_t messageSize, SerialPort* pSourcePort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	MPRINTLNS("SerialNode::forward>");
	MPRINTLNHEADER(pHeader);

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
		MPRINTSVAL("SerialNode::Update> no link found to system: ", pHeader->toAddr.sysId);
		return false;
	}

	if (lcbList.createLcb(pHeader, pSourcePort)) {
		MPRINTS("SerialNode::Update> LINK created: ");
		MPRINTLNADDR(pHeader->fromAddr);
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
	MPRINTLNADDR(pHeader->fromAddr);
	MPRINTLNADDR(pHeader->toAddr);
	return false;

}

void SerialNode::init(byte systemId) {
	SerialNode::systemId = systemId;
	MPRINTLNSVAL("SerialNode::Init> systemId: ", systemId);
}

SerialNode* SerialNode::getNodeList() {
	return pSerialNodeList;
}

SerialNode* SerialNode::createNode(byte localNodeId, bool active, byte remoteSysId, byte remoteNodeId,
		SerialPort* pSerialPort) {
	return new SerialNode(localNodeId, active, remoteSysId, remoteNodeId, pSerialPort);
}

void SerialNode::setOnMessageCallBack(
		void (*ptr)(const tSerialHeader* pHeader, const byte* pData, size_t datasize, SerialNode* pNode)) {
	pCallBackOnMessage = ptr;
}

void SerialNode::setOnPreConnectCallBack(void (*ptr)(SerialNode* pNode)) {
	pCallBackOnPreConnect = ptr;
}

SerialNode::SerialNode(byte localNodeId, bool active, byte remoteSysId, byte remoteNodeId, SerialPort* pSerialPort) {

	MPRINTLNS("SerialNode::SerialNode>");
	ASSERTP(systemId > 0, "please call System.init(yourSystemId) before you create nodes.");

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
	MPRINTLNADDR(pCcb->localAddr);

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

bool SerialNode::isLifeCheckLate() {
	if (!isActive()) {
		return false;
	}
	if ((millis() - lastReceiveTimeStamp) > SERIALNODE_TIME_LIFECHECK_LATE_MSEC){
		MPRINTSVAL(" SerialNode::isLifeCheckLate> node late: " ,getId());
		MPRINTLNSVAL(" : ",millis() - lastReceiveTimeStamp);
		return true;
	}

	return false;

}

bool SerialNode::isLifeCheckExpired() {
	if (!isActive()) {
		return false;
	}
	if ((millis() - lastReceiveTimeStamp) > SERIALNODE_TIME_LIFECHECK_EXPIRED_MSEC){
		MPRINTSVAL("SerialNode::isLifeCheckExpired> node expired: " ,getId());
		MPRINTLNSVAL(" : ",millis() - lastReceiveTimeStamp);
		return true;
	}
	return false;
}

void SerialNode::onMessage(tSerialHeader* pSerialHeader, const byte* pData, size_t datasize, SerialPort* pPort) {

	MPRINTLNS("SerialNode::onMessage>");
	assert(pSerialHeader);
	MPRINTLNHEADER(pSerialHeader);
	assert(pData ? datasize > 0 : datasize == 0);
	assert(pPort);
	assert(pCcb->remoteAddr.sysId != systemId);
	lastReceiveTimeStamp = millis();

	tSerialCmd cmd = pSerialHeader->cmd;
	tAcb* pAcb = NULL;
	/* for received replies*/

	bool connected = isConnected();
	bool userCall = true;
	bool acbNotFound = false;

	XPRINTSVALS("SerialNode::onMessage> node : ",getId(),"  <<< ");
	XPRINTSS(tSerialHeader::cmd2Str(cmd));
	XPRINTLNS(" <<< ");
	XPRINTLNHEADER(pSerialHeader);

	if (!connected) {
		if (!(cmd == CMD_ACK || cmd == CMD_NAK || cmd == CMD_CR) || cmd == CMD_AFA) {
			MPRINTLNSVAL("SerialNode::onMessage> unallowed message, status <> connected, aktid  :",
					pSerialHeader->aktid);
			return;
		}
	}

	// protocol requests from remote

	switch (cmd) {

	case CMD_NULL:
		MPRINTLNS("SerialNode::onMessage> CMD==0 received");
		return;

	case CMD_CR:

		if (!isActive() && isReadyToConnect()) { //if passiv and ready to be connected

			if (pCcb->remoteAddr.sysId == 0) {
				// connect, no matter who it is
				pCcb->remoteAddr = pSerialHeader->fromAddr;
				MPRINTLNS("pcb remote address:");
				MPRINTLNADDR(pCcb->remoteAddr);
			} else if (pCcb->remoteAddr != pSerialHeader->fromAddr) {
				// unknown node wants to connect, send NAK
				MPRINTSVAL("SerialNode::onMessage> unknown remote node wants to connect: ", pCcb->localAddr.nodeId);
				MPRINTLNS("SerialNode::onMessage> unknown remote node address:");
				MPRINTLNADDR(pCcb->remoteAddr);
				send(CMD_NAK, pSerialHeader->aktid);

			}
			pSerialPort = pPort;
			pCcb->status = CONNECTION_STATUS_CONNECTED;
			pCcb->remoteAddr = pSerialHeader->fromAddr;
			send(CMD_ACK, pSerialHeader->aktid);
			MPRINTLNSVAL("SerialNode::onMessage> CONNECTION_STATUS_CONNECTED node : ", pCcb->localAddr.nodeId);

		} else {
			if (isActive()) {
				MPRINTLNSVAL("SerialNode::onMessage> node is active, send NAK : ", pCcb->localAddr.nodeId);
			} else if (isConnected()) {

				if (pSerialHeader->fromAddr == pCcb->remoteAddr) {
					MPRINTLNSVAL("SerialNode::onMessage>nodes are already connected, send ACK : ",
							pCcb->localAddr.nodeId);
					send(CMD_ACK, pSerialHeader->aktid);
					break;
				} else {
					MPRINTLNSVAL("SerialNode::onMessage>node is already connected, send NAK : ", pCcb->localAddr.nodeId);
				}
			} else if (!isReadyToConnect()) {
				MPRINTLNSVAL("SerialNode::onMessage> node is not ready, send NAK : ", pCcb->localAddr.nodeId);
			}
			send(CMD_NAK, pSerialHeader->aktid);
		}
		break;
	case CMD_CD:
		if (connected) {
			send(CMD_ACK, pSerialHeader->aktid);
			pSerialPort = NULL;
			pCcb->status = CONNECTION_STATUS_DISCONNECTED;
			MPRINTLNSVAL("SerialNode::onMessage> node disconnected : ", pCcb->localAddr.nodeId);
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
				pSerialPort = pPort;
				if (!isActive()) {
					MPRINTSVAL("SerialNode::onMessage>ACK ON CR node is not active: ", pCcb->localAddr.nodeId);
				} else if (isConnected()) {
					MPRINTLNSVAL("SerialNode::onMessage>node is already connected: ", pCcb->localAddr.nodeId);

				} else if (!isReadyToConnect()) {
					MPRINTSVAL("SerialNode::onMessage>ACK ON CR node is not ready: ", pCcb->localAddr.nodeId);

				} else {
					pCcb->remoteAddr = pSerialHeader->fromAddr;
					pCcb->status = CONNECTION_STATUS_CONNECTED;
					pSerialPort = pPort;
					MPRINTLNSVAL("SerialNode::onMessage> CONNECTION_STATUS_CONNECTED node (active) : ", getId());
					MPRINTLNADDR(pCcb->localAddr)
				}
				;
				break;
			case CMD_CD:
				userCall = false;
				pSerialPort = NULL;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				MPRINTS("SerialNode::onMessage> node disconnected : ");
				MPRINTLNADDR(pCcb->localAddr)
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
				MPRINTSVAL("SerialNode::onMessage> NAK ON CR : ", pCcb->localAddr.nodeId)
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
	} // end of switch

	if (acbNotFound) {
		MPRINTLNSVAL("SerialNode::onMessage> acb not found - aktid:", pSerialHeader->aktid);
		userCall = false;
	}

	if (userCall) {
		if (SerialNode::pCallBackOnMessage) {
			SerialNode::pCallBackOnMessage(pSerialHeader, pData, datasize, this);
		}
	}
	if (pAcb) {

		XPRINTLNSVAL("SerialNode::onMessage> round trip time : ", millis() - pAcb->timeStamp);
		XPRINTFREE;

		MPRINTLNSVAL("SerialNode::onMessage> ACB COUNT : ", SerialNode::acbList.count());
		SerialNode::acbList.deleteAcbEntry(pSerialHeader->aktid);
	}

}

// user request /reply

bool SerialNode::areAllNodesConnected() {
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
	// call on reconnect();
	if (pCallBackOnPreConnect) {
		pCallBackOnPreConnect(this);
	} else {
		if (!isReadyToConnect()) {
			MPRINTLNS("SerialNode:reconnect()> no OnPreConnect-Handler found, set node ready to connect");
		}
		setReady(true);
	}
	if (isReadyToConnect()) {
		if (isActive()) {
			MPRINTLNSVAL("SerialNode:reconnect()> send CR for node ", getId());
			send(CMD_CR);
		} else {
			MPRINTLNSVAL("SerialNode:reconnect()> waiting for CR message, node: ", getId());
		}
	}
}

void SerialNode::checkLifeNodes(unsigned long period) {

	unsigned long now = millis();
	SerialNode* pNode = SerialNode::pSerialNodeList;
	if ((now - SerialNode::lastLiveCheckTimeStamp) < period) {
		return;
	}

	while (pNode) {
		if (!pNode->isConnected()) {
			if ((now - pNode->lastConnectionTrialTimeStamp) > 2000) {
				pNode->reconnect();
				pNode->lastConnectionTrialTimeStamp = now;
			}
		} else if (pNode->isLifeCheckExpired()) {
			if (now - pNode->lastConnectionTrialTimeStamp > 2000) {
				pNode->reconnect();
				pNode->lastConnectionTrialTimeStamp = now;
			}
		} else if (pNode->isLifeCheckLate()) {
			if (now - pNode->lastLiveTrialTimeStamp > 2000) {
				pNode->send(CMD_LIVE);
				pNode->lastLiveTrialTimeStamp = now;
			}

		}
		pNode = (SerialNode*) pNode->pNext;
	}
	SerialNode::lastLiveCheckTimeStamp = now;
}


void SerialNode::processNodes(bool lifeCheck, unsigned long lifeCheckPeriodMsec) {
if (lifeCheck) {
	checkLifeNodes(lifeCheckPeriodMsec);
}
SerialRx::readNextOnAllPorts();
}

bool SerialNode::connect(byte remoteSysId, byte remoteNodeId, unsigned long timeOut, unsigned long checkPeriod) {

MPRINTLNS("SerialNode::connect>");
pCcb->remoteAddr.sysId = (remoteSysId > 0) ? remoteSysId : pCcb->remoteAddr.sysId;
pCcb->remoteAddr.nodeId = (remoteNodeId > 0) ? remoteNodeId : pCcb->remoteAddr.nodeId;

ASSERTP(isActive() ? pCcb->remoteAddr.sysId > 0 :true, "unknown remote system id, must be > 0 for active nodes");

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
		MPRINTLNADDR(pCcb->localAddr);
		MPRINTS(" to ");
		MPRINTLNADDR(pCcb->remoteAddr);
		return true;
	}

}
if (timeOut > 0) {
	MPRINTLNS("SerialNode::connect> TIMEOUT");
	acbList.deleteAcbEntry(pCcb, CMD_CR);
}

return false; // no connections or timeout

}

tAktId SerialNode::send(tSerialCmd cmd, tAktId replyOn, byte par, byte* pData, byte datasize, byte replyToSys,
	byte replyToNode) {

tSerialHeader header, *pHeader;
pHeader = &header;
MPRINTS("SerialNode::send>  >>> ");
MPRINTSS(tSerialHeader::cmd2Str(cmd));
MPRINTLNS(" >>>");
assert(pCcb->remoteAddr.sysId != systemId);

// not connected
if (pCcb->status != CONNECTION_STATUS_CONNECTED && !(cmd == CMD_CR || cmd == CMD_ACK || cmd == CMD_NAK)) {
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
			MPRINTLNS("SerialNode::send> node, not connected , unknown remote adress:");
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

tAktId SerialNode::writeToPort(tSerialHeader* pHeader, byte* pData, size_t datasize, SerialPort* pPort) {
XPRINTSVALS("SerialNode::writeToPort> node : ",pHeader->fromAddr.nodeId,"  >>> ");
XPRINTSS(tSerialHeader::cmd2Str(pHeader->cmd));
XPRINTLNS(" >>> ");
XPRINTLNHEADER(pHeader);

MPRINTLNSVAL("SerialNode::writeToPort> send to port: ", pPort->remoteSysId);

assert(pHeader);
assert(pHeader->toAddr.sysId != pHeader->fromAddr.sysId);

if (pHeader->aktid == 0) { // we need an aktId
	acbList.createOrUseAcb(pHeader);
}

pPort->getTx()->sendPreamble();
pPort->getTx()->sendRawData((byte*) pHeader, sizeof(tSerialHeader));
if (pData && datasize > 0) {
	pPort->getTx()->sendRawData((byte*) pData, datasize);
}
pPort->getTx()->sendPostamble();
XTIME_RESET;
if (pData) {
	MPRINTLNSVAL("datasize: ", datasize);
}


return pHeader->aktid;
}

bool waitOnReply(tAktId , unsigned long ) {

	MPRINTLNS("SerialNode::waitOnReply , not implemented yet");
	return false;

}

