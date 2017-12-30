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


SerialNode* SerialNode::pSerialNodeList = NULL;
SerialNode* SerialNode::pProcessingNode = NULL;

LcbList SerialNode::lcbList;
void (*SerialNode::pCallBackOnMessage)(const tSerialHeader* pHeader, const byte* pData, size_t datasize,
		SerialNode* pNode)=NULL; // user callback
void (*SerialNode::pCallBackOnPreConnect)(SerialNode* pNode)=NULL; // user callback before node connects

void SerialNode::update(const byte* pMessage, size_t messageSize, SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	DPRINTLNSVAL("SerialNode::update> size: ", messageSize);
	assert(pHeader);
	DPRINTLNHEADER(pHeader);

	size_t hsize = sizeof(tSerialHeader);
	size_t dsize = 0;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}
	if (cmd >= CMD_NAK && cmd < CMD_ACD) { //System telegramm
		if (dsize > 0) {
			DPRINTLNSVAL("SerialNode::update> message has corrupted size, expected : ", hsize);
			return;
		}
	}

	if (pHeader->toAddr.sysId == 0) {
		DPRINTLNS("SerialNode::update> systemId must be > 0 ");
		return;
	}

	if (pHeader->toAddr.sysId == SerialNode::systemId) {
		//local system
		SerialNode* pNode = getNodeList();
		if (!pNode) {
			DPRINTLNS("SerialNode::update> no nodes found");
			assert(false);
		}

		if (pHeader->toAddr.nodeId > 0) {
			// find node in our system
			while (pNode) {
				if (pNode->pCcb->localAddr == pHeader->toAddr) {
					// node found
					DPRINTLNS("SerialNode::update> node found");
					pNode->onMessage(pHeader, (dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize, pPort);
					return;
				}
				pNode = (SerialNode*) pNode->pNext;
			}DPRINTLNS("SerialNode::update> node not found : ");
			DPRINTLNHEADER(pHeader);
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
			}DPRINTLNS("SerialNode::update> no ready node for CR found");
			return;
		} else {
			DPRINTLNS("SerialNode::update> message without nodeId in toAddr received");
			return;
		}
		// end of local system
	}

	if (pHeader->fromAddr.sysId == systemId) {
		DPRINTLNS("SerialNode::update> remote system has the same id as local system , message discarded");
		return;
	}

	//remote system
	forward(pMessage, messageSize, pPort);

}

bool SerialNode::forward(const byte* pMessage, size_t messageSize, SerialPort* pSourcePort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	DPRINTLNS("SerialNode::forward>");
	DPRINTLNHEADER(pHeader);

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
		DPRINTSVAL("SerialNode::Update> no link found to system: ", pHeader->toAddr.sysId);
		return false;
	}

	if (lcbList.createLcb(pHeader, pSourcePort)) {
		DPRINTS("SerialNode::Update> LINK created: ");
		DPRINTLNADDR(pHeader->fromAddr);
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
	}DPRINTS("SerialNode::Update> forward for CR failed: ");
	DPRINTLNADDR(pHeader->fromAddr);
	DPRINTLNADDR(pHeader->toAddr);
	return false;

}

void SerialNode::init(byte systemId) {
	SerialNode::systemId = systemId;
	DPRINTLNSVAL("SerialNode::Init> systemId: ", systemId);
}

SerialNode* SerialNode::getNodeList() {
	return pSerialNodeList;
}

SerialNode* SerialNode::createNode(byte localNodeId, bool active, byte remoteSysId, byte remoteNodeId,
		SerialPort* pPort) {
	return new SerialNode(localNodeId, active, remoteSysId, remoteNodeId, pPort);
}

void SerialNode::setOnMessageCallBack(
		void (*ptr)(const tSerialHeader* pHeader, const byte* pData, size_t datasize, SerialNode* pNode)) {
	pCallBackOnMessage = ptr;
}

void SerialNode::setOnPreConnectCallBack(void (*ptr)(SerialNode* pNode)) {
	pCallBackOnPreConnect = ptr;
}

SerialNode::SerialNode(byte localNodeId, bool active, byte remoteSysId, byte remoteNodeId, SerialPort* pPort) {

	DPRINTLNS("SerialNode::SerialNode>");
	ASSERTP(systemId > 0, "please call System.init(yourSystemId) before you create nodes.");

	this->pSerialPort = pPort;
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
	}DPRINTS("SerialNode:SerialNode> created : ");
	DPRINTLNADDR(pCcb->localAddr);

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
	return pCcb->status == CONNECTION_STATUS_READY;
}

bool SerialNode::isLifeCheckLate() {
	if (!isActive()) {
		return false;
	}
	if ((millis() - lastReceiveTimeStamp) > SERIALNODE_TIME_LIFECHECK_LATE_MSEC) {
		MPRINTSVAL(" SerialNode::isLifeCheckLate> node late: " ,getId());MPRINTLNSVAL(" : ",millis() - lastReceiveTimeStamp);
		return true;
	}

	return false;

}

bool SerialNode::isLifeCheckExpired() {
	if (!isActive()) {
		return false;
	}

	tAcb* prevSendAcb = AcbList::instance.getAcbEntry(lastSendAcbAktId);
	if (prevSendAcb) {

		if ((millis()-prevSendAcb->timeStamp) > SERIALNODE_TIME_LIFECHECK_EXPIRED_MSEC) {
			MPRINTSVAL("SerialNode::isLifeCheckExpired> node expired: " ,getId());MPRINTLNSVAL(" round trip : ",millis()-prevSendAcb->timeStamp);
			AcbList::instance.deleteAcbEntry(prevSendAcb->aktid);
			return true;
		}


	}else{
		if ((millis() - lastReceiveTimeStamp) > SERIALNODE_TIME_LIFECHECK_LATE_EXPIRED_MSEC) {
			MPRINTSVAL(" SerialNode::isLifeCheckLate> node very late ,expired: " ,getId());MPRINTLNSVAL(" : ",millis() - lastReceiveTimeStamp);
			return true;
		}
	}
	return false;
}

void SerialNode::onMessage(tSerialHeader* pSerialHeader, const byte* pData, size_t datasize, SerialPort* pPort) {

	DPRINTLNS("SerialNode::onMessage>");
	assert(pSerialHeader);
	DPRINTLNHEADER(pSerialHeader);
	assert(pData ? datasize > 0 : datasize == 0);
	assert(pPort);
	assert(pCcb->remoteAddr.sysId != systemId);
	lastReceiveTimeStamp = millis();

	tSerialCmd cmd = pSerialHeader->cmd;
	tAcb* pAcb = NULL;
	/* for received replies*/

	bool connected = isConnected();
	bool userCall = false;
	bool acbNotFound = false;

	MPRINTSVALS("SerialNode::onMessage> node : ", getId(), "  <<< ");
	MPRINTSS(tSerialHeader::cmd2Str(cmd));
	MPRINTLNS(" <<<< ");
	MPRINTLNHEADER(pSerialHeader);

	if (!connected) {
		if (!(cmd == CMD_ACK || cmd == CMD_NAK || cmd == CMD_CR)) {
			MPRINTLNSVAL("SerialNode::onMessage> unallowed message, status <> connected, aktid  :",
					pSerialHeader->aktid);
			return;
		}
	}

	// protocol requests from remote

	switch (cmd) {

	case CMD_NULL:
		DPRINTLNS("SerialNode::onMessage> CMD==0 received");
		return;

	case CMD_CR:

		if (!isActive() ){
			if (pCallBackOnPreConnect) {
				pCallBackOnPreConnect(this);
			}

			if (!isReadyToConnect()){
				MPRINTLNSVAL("SerialNode::onMessage> node not ready :" ,getId());
				return;
			}

			if (pCcb->remoteAddr.sysId == 0) {
				// connect, no matter who it is
				pCcb->remoteAddr = pSerialHeader->fromAddr;
				MPRINTLNS("pcb remote address:");
				MPRINTLNADDR(pCcb->remoteAddr);
			} else if (pCcb->remoteAddr != pSerialHeader->fromAddr) {
				// unknown node wants to connect, send NAK
				MPRINTSVAL("SerialNode::onMessage> unknown remote node wants to connect: ", pCcb->localAddr.nodeId);DPRINTLNS("SerialNode::onMessage> unknown remote node address:");
				MPRINTLNADDR(pCcb->remoteAddr);
				send(CMD_NAK, pSerialHeader->aktid);

			}
			this->pSerialPort = pPort;
			pCcb->status = CONNECTION_STATUS_CONNECTED;
			pCcb->remoteAddr = pSerialHeader->fromAddr;
			send(CMD_ACK, pSerialHeader->aktid);
			MPRINTLNSVAL("SerialNode::onMessage> CONNECTION_STATUS_CONNECTED node : ", pCcb->localAddr.nodeId);

		} else {
			if (isActive()) {
				DPRINTLNSVAL("SerialNode::onMessage> node is active, send NAK : ", pCcb->localAddr.nodeId);
			} else if (isConnected()) {

				if (pSerialHeader->fromAddr == pCcb->remoteAddr) {
					DPRINTLNSVAL("SerialNode::onMessage>nodes are already connected, send ACK : ",
							pCcb->localAddr.nodeId);
					send(CMD_ACK, pSerialHeader->aktid);
					break;
				} else {
					DPRINTLNSVAL("SerialNode::onMessage>node is already connected, send NAK : ", pCcb->localAddr.nodeId);
				}
			} else if (!isReadyToConnect()) {
				DPRINTLNSVAL("SerialNode::onMessage> node is not ready, send NAK : ", pCcb->localAddr.nodeId);
			}
			send(CMD_NAK, pSerialHeader->aktid);
		}

		break;
	case CMD_CD:
		if (connected) {
			send(CMD_ACK, pSerialHeader->aktid);
			pSerialPort = NULL;
			pCcb->status = CONNECTION_STATUS_DISCONNECTED;
			DPRINTLNSVAL("SerialNode::onMessage> node disconnected : ", pCcb->localAddr.nodeId);
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

	case CMD_ACD:
		userCall=true;
		MPRINTSVALS("SerialNode::onMessage> CMD_ACD : ", getId(), "  <<< ");
		break;
	case CMD_ARQ:
		userCall=true;
		break;
	case CMD_ARP:
		userCall=true;
		pAcb = AcbList::instance.getAcbEntry(pSerialHeader->aktid);
		if (!pAcb) {
			userCall = false;
			acbNotFound = true;
		}
		break;
	case CMD_ACK:
		pAcb = AcbList::instance.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
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
				pSerialPort = NULL;
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				DPRINTS("SerialNode::onMessage> node disconnected : ");
				DPRINTLNADDR(pCcb->localAddr)
				;
				break;
			case CMD_LIVE:
				userCall = false;
				break;
			case CMD_ARQ:
				userCall=true;
			case CMD_ACD:
				break;
				//nothing to do
			}
		} else {
			acbNotFound = true;
		}
		break;
	case CMD_NAK:
		pAcb = AcbList::instance.getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				DPRINTSVAL("SerialNode::onMessage> NAK ON CR : ", pCcb->localAddr.nodeId)
				;

				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				break;
			case CMD_CD:
				break;
			case CMD_LIVE:
				pCcb->status = CONNECTION_STATUS_DISCONNECTED;
				break;
			case CMD_ARQ:
				userCall=true;
			case CMD_ACD:
				break;
			}
		} else {
			acbNotFound = true;
		}
	} // end of switch

	if (acbNotFound) {
		DPRINTLNSVAL("SerialNode::onMessage> acb not found - aktid:", pSerialHeader->aktid);
		userCall = false;
	}

	if (userCall) {
		if (SerialNode::pCallBackOnMessage) {
			SerialNode::pCallBackOnMessage(pSerialHeader, pData, datasize, this);
		}
	}
	if (pAcb) {

		XPRINTLNSVAL("SerialNode::onMessage> round trip time : ", millis() - pAcb->timeStamp);

		unsigned int count = AcbList::instance.count();

		DPRINTLNSVAL("SerialNode::onMessage> ACB COUNT : ",count );

		AcbList::instance.deleteAcbEntry(pSerialHeader->aktid);
		assert(AcbList::instance.count() < count);
		XPRINTFREE
		;

	} else {
		// old reply, acb already reused
		//assert(false);
	}

// user request /reply
}

bool SerialNode::areAllNodesConnected() {
	static bool bStart=false;
	SerialNode* pNode = SerialNode::pSerialNodeList;
	bool connected = true;

	while (pNode) {
		connected = connected && pNode->isConnected();
		pNode = (SerialNode*) pNode->pNext;
	}
	if (connected ) {
		if (!bStart) {
			MPRINTLNS(" SerialNode::areAllNodesConnected()> *** ALL NODES CONNECTED WITH REMOTE ***");
			bStart = true;
		}
	}else{
		bStart=false;
	}
	return connected;
}


void SerialNode::reconnect() {
	// call on reconnect();
	setReady(false);
	if (isActive()) {
		if (pCallBackOnPreConnect) {
			pCallBackOnPreConnect(this);
		}
	}else{
		MPRINTLNSVAL("SerialNode:reconnect()> waiting for CR message, node: ", getId());
		return;
	}

	if (isReadyToConnect()) {
		MPRINTLNSVAL("SerialNode:reconnect()> send CR for node ", getId());
		send(CMD_CR);
	}else {
		MPRINTLNSVAL("SerialNode:reconnect> node not ready: ",getId());
		return;
	}
}

void SerialNode::checkConnection(SerialNode* pNode, unsigned long period) {

	unsigned long now = millis();
	if (pNode->lastLiveCheckTimeStamp && (now - pNode->lastLiveCheckTimeStamp) < period) {
		return;
	}
	if (!pNode->isConnected()) {
		pNode->reconnect();
	} else if (pNode->isLifeCheckExpired()) {
		// set all nodes setReady(false)
		pNode->reconnect();


	} else if (pNode->isLifeCheckLate()) {
		//if (AcbList::instance.count(pNode->getPort()->remoteSysId) == 0) { // node waited on other nodes
		//	pNode->lastReceiveTimeStamp = now; // reset time stamp
		//}
		tAcb* prevSendAcb = AcbList::instance.getAcbEntry(pNode->lastSendAcbAktId);
		MPRINTLNSVAL("SerialNode::checkConnection> lastSendAktId: " ,pNode->lastSendAcbAktId);

		if (!prevSendAcb) {
			pNode->send(CMD_LIVE);
			//pNode->lastReceiveTimeStamp = now;
		}else{
			MPRINTLNS("SerialNode::checkConnection> acb found");
		}
	}

	pNode->lastLiveCheckTimeStamp = now;

}

SerialPort* SerialNode::getPort() {
	return pSerialPort;
}

SerialNode* SerialNode::getRoot(){
	return pSerialNodeList;
}

SerialNode* SerialNode::getNext() {
	return (SerialNode*) pNext;
}


SerialNode* SerialNode::cycleNextNodeOnPort() {
	SerialNode * _pnext =this;
	do {

		_pnext=_pnext->getNext();
		if (!_pnext) {
			_pnext=pSerialNodeList;
		}
		if (!(getPort() && _pnext->getPort())) {
			return this;
		}

		if (_pnext->getPort()==getPort()){
			return _pnext;
		}

	}while (_pnext!=this);

	return this;
}

void SerialNode::processNodes(bool lifeCheck) {

	if (pProcessingNode == NULL) {
		pProcessingNode = pSerialNodeList;

	} else if (AcbList::instance.count() == 0) {
		pProcessingNode = (SerialNode*) pProcessingNode->pNext;
	} else if (pProcessingNode->getPort()) {

		if (AcbList::instance.count(pProcessingNode->getPort()==0)) {
			pProcessingNode = (SerialNode*) pProcessingNode->pNext;
		}else if (pProcessingNode->getPort()->getType()==PORTTYPE_SOFTSERIAL) {
			pProcessingNode= pProcessingNode->cycleNextNodeOnPort();
		}
	}


	if (pProcessingNode == NULL) {
		pProcessingNode = pSerialNodeList;
	}
    MPRINTLNSVAL("SerialNode::processNode : ",pProcessingNode->getId());

	ASSERTP(pProcessingNode, "SerialNode::processNodes> no nodes found");

	SoftSerialPort::cycleListenerPort(); // if SoftSerials used , swi

	if (lifeCheck) {
		checkConnection(pProcessingNode);
	}
	SerialPort::readNextOnAllPorts();
}



tAktId SerialNode::send(tSerialCmd cmd, tAktId replyOn, byte par, byte* pData, size_t datasize, byte replyToSys,
		byte replyToNode) {

	tSerialHeader header, *pHeader;
	pHeader = &header;
	MPRINTS("SerialNode::send>  >>> ");DPRINTSS(tSerialHeader::cmd2Str(cmd));DPRINTLNS(" >>>");
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
			//DPRINTLNSVAL("Address of node: ", long((void* ) this));
			if (this->isConnected()) {
				header.toAddr = pCcb->remoteAddr;
			} else {
				MPRINTLNS("SerialNode::send> node, not connected , unknown remote address:");
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

	if (getPort()) { //connected or port was preset
		SerialNode::writeToPort(pHeader, pData, datasize, getPort());
		DPRINTLNS("SerialNode::send> to node's port - success");
		return header.aktid;
	} else { // not connected

		SerialPort* pPort = SerialPort::getPort(header.toAddr.sysId);
		if (pPort) {
			SerialNode::writeToPort(pHeader, pData, datasize, pPort);
			DPRINTLNS("SerialNode::send> to header toAddr port - success");
			return header.aktid;
		} else {
			pPort = SerialPort::pSerialPortList;
			if (!pPort) {
				DPRINTLNS("SerialNode::send> no port in list !");
				return 0;
			}
			if (pPort) {
				DPRINTLNS("SerialNode::send> send to all ports ...");
				while (pPort) {
					SerialNode::writeToPort(pHeader, pData, datasize, pPort);
					pPort = (SerialPort*) pPort->pNext;
				}DPRINTLNS("SerialNode::send> to port in list - success");
				return header.aktid;
			}
		}
	}DPRINTLNS("SerialNode::send> failed");
	return 0;
}

tAktId SerialNode::writeToPort(tSerialHeader* pHeader, byte* pData, size_t datasize, SerialPort* pPort) {
	MPRINTSVALS("SerialNode::writeToPort> node : ", pHeader->fromAddr.nodeId, "  >>> ");
	MPRINTSS(tSerialHeader::cmd2Str(pHeader->cmd));
	MPRINTLNSVAL(" >>> port: ", pPort->remoteSysId);

	// no outstanding replies required by CR or LIVE messages ( for better round trip time)
//	if (pHeader->cmd == CMD_LIVE && AcbList::instance.count() >0  && pPort->isListening() ) {
//		MPRINTLNS("SerialNode::writeToPort> waiting for reply,  LIVE message canceled");
//		return 0;
//	}

	if (pPort->getType() == PORTTYPE_SOFTSERIAL && !pPort->isListening()) {
		SoftSerialPort* pListener = SoftSerialPort::getListenerPort();
		ASSERTP(pListener != pPort, "SerialNode::writeToPort> GLEICHER LISTENER ?");
		unsigned int otherAcbCount = AcbList::instance.count(pListener->remoteSysId);
		// open acbs on other SoftSerialPort
		if (otherAcbCount > 0) {

			if (pHeader->cmd == CMD_LIVE) {
				MPRINTLNS("SerialNode::writeToPort> waiting for replies on other soft serial port,  cancel");
				return 0;
			} else if (pHeader->cmd == CMD_ARQ) {
				MPRINTLNS("SerialNode::writeToPort> waiting for replies on other soft serial port...");
				MPRINTFREE;
						unsigned long tStamp= millis();
						while(otherAcbCount > 0) {
							 processNodes();
							 otherAcbCount = AcbList::instance.count(pListener->remoteSysId);
						}
				MPRINTLNSVAL("SerialNode::writeToPort> waiting end, waited for : " ,millis()-tStamp);
				pPort->listen();

			}
		}
	}

	if (!pPort->isListening() && tSerialHeader::isReplyExpected(pHeader->cmd)) {
		pPort->listen();
	}

	DPRINTLNSVAL("SerialNode::writeToPort> send to port: ", pPort->remoteSysId);

	assert(pHeader);
	assert(pHeader->toAddr.sysId != pHeader->fromAddr.sysId);

	if (pHeader->aktid == 0) { // we need an aktId
		pHeader->aktid = AcbList::instance.getNextAktId();

		if (tSerialHeader::isReplyExpected(pHeader->cmd)) {
			AcbList::instance.createOrUseAcb(pHeader)->portId = pPort->remoteSysId;
			lastSendAcbAktId = pHeader->aktid;
		}
	}

	pPort->getTx()->sendPreamble();
	pPort->getTx()->sendRawData((byte*) pHeader, sizeof(tSerialHeader));
	if (pData && datasize > 0) {
		pPort->getTx()->sendRawData((byte*) pData, datasize);
	}
	pPort->getTx()->sendPostamble();

	if (pData) {
		DPRINTLNSVAL("datasize: ", datasize);
	}
	MPRINTLNS("SerialNode::writeToPort> success , header...");
	MPRINTLNHEADER(pHeader);
	return pHeader->aktid;
}

bool waitOnReply(tAktId, unsigned long) {

	DPRINTLNS("SerialNode::waitOnReply , not implemented yet");
	return false;

}

