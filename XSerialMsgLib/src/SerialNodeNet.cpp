/*
 * SerialNodeNet.cpp
 *
 *  Created on: 02.01.2018
 *      Author: rea
 */

#include <tools.h>
#include "SerialNodeNet.h"
#include "SerialHeader.h"
#include "SoftSerialPort.h"
#include "SerialNode.h"

namespace SerialMsgLib {
//static
SerialNodeNet* SerialNodeNet::pInst = NULL;

//static
SerialNodeNet* SerialNodeNet::init(byte systemId) {
	if (pInst) {
		delete pInst;
	}
	pInst = new SerialNodeNet(systemId);
	return pInst;
}

//static
SerialNodeNet* SerialNodeNet::getInstance() {
	return pInst;
}

//private
SerialNodeNet::SerialNodeNet(byte systemId) {
	this->systemId = systemId;
}

SerialNodeNet::~SerialNodeNet() {
	// TODO Auto-generated destructor stub
}

byte SerialNodeNet::getSystemId() {
	return systemId;
}

SerialNode* SerialNodeNet::getProcessingNode() {
	return pProcessingNode;
}

void SerialNodeNet::setProcessingNode(SerialNode* pNode) {
	pProcessingNode = pNode;
}

SerialNode* SerialNodeNet::getNodeByAcb(tAcb* pAcb) {
	SerialNode* pNode = getRootNode();
	while (pNode) {
		if (pNode->getCcb()->localAddr == pAcb->fromAddr) {
			return pNode;
		}
		pNode = pNode->getNext();
	}
	return NULL;
}

SerialNode* SerialNodeNet::createNode(byte localNodeId, bool active,
		byte remoteSysId, byte remoteNodeId, SerialPort* pPort) {
	return new SerialNode(localNodeId, active, remoteSysId, remoteNodeId, pPort);
}

SerialNode* SerialNodeNet::getNode(byte nodeId) {
	SerialNode* pNode = getRootNode();
	while (pNode) {
		if (pNode->getId() == nodeId) {
			return pNode;
		}
		pNode = pNode->getNext();
	}
	return NULL;
}

SerialNode* SerialNodeNet::getRootNode() {
	return pSerialNodeList;
}

void SerialNodeNet::setRootNode(SerialNode* pNode) {
	pSerialNodeList = pNode;
}

bool SerialNodeNet::areAllNodesConnected() {
	static bool bStart = false;
	SerialNode* pNode = SerialNodeNet::pInst->getRootNode();
	bool connected = true;

	while (pNode) {
		connected = connected && pNode->isConnected();
		pNode = (SerialNode*) pNode->getNext();
	}
	if (connected) {
		if (!bStart) {
			DPRINTLNS(
					" SerialNodeNet::areAllNodesConnected()> *** ALL NODES CONNECTED WITH REMOTE ***");
			//AcbList::getInstance()->deleteAcbList();
			//AcbList::getInstance()->printList();
			bStart = true;
		}
	} else {
		bStart = false;
	}
	return connected;
}
void SerialNodeNet::update(const byte* pMessage, size_t messageSize,
		SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	MPRINTS("SerialNodeNet::update> ");MPRINTLNHEADER(pHeader);
	MPRINTLNSVAL("SerialNodeNet::update> size: ", messageSize);
	assert(pHeader);

	size_t hsize = sizeof(tSerialHeader);
	size_t dsize = 0;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}
	if (pHeader->cmd >= CMD_NAK && pHeader->cmd < CMD_ACD) { //System telegramm
		if (dsize > 0) {
			DPRINTLNSVAL(
					"SerialNodeNet::update> message has corrupted size, expected : ",
					hsize);
			return;
		}
	}

	if (pHeader->toAddr.sysId == 0) {
		DPRINTLNS("SerialNodeNet::update> systemId must be > 0 ");
		return;
	}

	if (pHeader->toAddr.sysId == pInst->getSystemId()) {
		//local system
		SerialNode* pNode = pInst->getRootNode();
		if (!pNode) {
			DPRINTLNS("SerialNodeNet::update> no nodes found");
			assert(false);
		}

		if (pHeader->toAddr.nodeId > 0) {
			// find node in our system
			while (pNode) {
				if (pNode->getCcb()->localAddr == pHeader->toAddr) {
					// node found
					DPRINTLNS("SerialNodeNet::update> node found");
					pNode->onMessage(pHeader,
							(dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize,
							pPort);
					return;
				}
				pNode = (SerialNode*) pNode->getNext();
			}DPRINTLNS("SerialNodeNet::update> node not found : "); DPRINTLNHEADER(pHeader);
			return;
		}

		// send CR to first found passive but ready(to connect) node without remote node id
		// and whithout remote system id or equal system id to headers from system id

		if (pHeader->cmd == CMD_CR) {
			SerialNode* pNode = pInst->getRootNode();
			while (pNode) {
				if (!pNode->isActive() && pNode->isReadyToConnect()
						&& (pNode->getCcb()->remoteAddr.sysId == 0
								|| pNode->getCcb()->remoteAddr.sysId
										== pHeader->fromAddr.sysId)
						&& pNode->getCcb()->remoteAddr.nodeId == 0) {// only as comment
					pNode->onMessage(pHeader,
							(dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize,
							pPort);
					return;
				}
				pNode = (SerialNode*) pNode->getNext();
			}
			DPRINTLNS("SerialNodeNet::update> no ready node for CR found");
			return;
		} else {
			DPRINTLNS(
					"SerialNodeNet::update> message without nodeId in toAddr received");
			return;
		}
		// end of local system
	}

	if (pHeader->fromAddr.sysId == pInst->getSystemId()) {
		DPRINTLNS(
				"SerialNodeNet::update> remote system has the same id as local system , message discarded");
		return;
	}

	//remote system
	pInst->forward(pMessage, messageSize, pPort);

}

bool SerialNodeNet::forward(const byte* pMessage, size_t messageSize,
		SerialPort* pSourcePort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	MPRINTLNS("SerialNodeNet::forward>");
	DPRINTLNHEADER(pHeader);


	// check if link cab ve completed
	if (pHeader->cmd == CMD_ACK) { //no port no completed link
		tLcb* pLcb = lcbList.getOpenLcb(pHeader);
		if (pLcb) { //complete link
			pLcb->sysIdB = pHeader->fromAddr.sysId;
			pLcb->pPortB = pSourcePort;
			MPRINTLNS("SerialNodeNet::forward> LINK completed");
		}
	}

	SerialPort* pTargetPort = SerialPort::getPort(pHeader->toAddr.sysId);
	SerialPort* pMessageSourcePort = SerialPort::getPort(pHeader->fromAddr.sysId);


	// set new masterport if softserial
	if(pHeader->isReplyExpected()  && pSourcePort->getType()==PORTTYPE_SOFTSERIAL){
		((SoftSerialPort*)pSourcePort)->setMaster();
	}


	if (!pTargetPort) {


		// find link
		pTargetPort = lcbList.getTargetPort(pHeader);
		if (pTargetPort) {
			MPRINTLNSVAL("SerialNodeNet::forward> LINK found to port : ",
					pTargetPort->getId());
		} else {
			MPRINTLNS("SerialNodeNet::forward> no Link found");
		}
	}

// port or linked port found
	if (pTargetPort) {
		DPRINTLNS("SerialNodeNet::forward> port found");
		if (!pHeader->isReplyExpected()) {
			// if reply -> find the acb of the forwarded request and delete it
			AcbList* pList = AcbList::getList(pHeader->toAddr.sysId);
			if (pList) {
				tAcb* pAcb = pList->getAcbEntry(pHeader->aktid);
				if (pAcb) {
					pList->deleteAcbEntry(pHeader->aktid);
				}
			}
		}
		if (pHeader->cmd==CMD_CR && !pMessageSourcePort) { // if forward and far sourceport
			if (!lcbList.getLinkedLcb(pHeader)) {
				DPRINTLNSVAL(
						"SerialNodeNet::forward> CR message source port not found: ",
						pHeader->fromAddr.sysId);
				if (!lcbList.getOpenLcb(pHeader)) {
					lcbList.createLcb(pHeader, pSourcePort);
					DPRINTLNSVAL(
							"SerialNodeNet::forward> open LINK created from: ",
							pSourcePort->getId());
				} else {
					DPRINTLNS("SerialNodeNet::forward> open LINK found");
				}
			} else {
				DPRINTLNS(
						"SerialNodeNet::forward> LINK found (far source port)");
			}
		}

		DPRINTLNSVAL("SerialNodeNet::forward> send message to port : ",
				pTargetPort->getId());
		DPRINTLNHEADER(pHeader);

		pTargetPort->sendMessage(pMessage, messageSize);
		pTargetPort->listen();
		return true;
	}
	ASSERTP(!pTargetPort, "SerialNodeNet::forward> unexpected pTargetPort");

// if CR create a link an send to all ports without ports to the source system

	if (pHeader->cmd != CMD_CR) { //create a link only by connection request
		DPRINTLNSVAL("SerialNodeNet::forward> no link found to system: ",
				pHeader->toAddr.sysId);
		return false;
	}


	if (!lcbList.getOpenLcb(pHeader)) {
		lcbList.createLcb(pHeader, pSourcePort);
		DPRINTLNSVAL("SerialNodeNet::forward> open LINK created from: ",
				pSourcePort->getId());
	} else {
		DPRINTLNS("SerialNodeNet::forward> open LINK found");
	}


	SerialPort* pport = SerialPort::pSerialPortList;

	byte cnt = 0;
	// send CR only to foreign systems, and not to system where CR was sent.
	while (pport) {
		if (pport->getId() != pSourcePort->getId()) {
			// add a acb for the forwarded request
			// ??? same sys aktid but differnent portIDs
			// send and wait or aktid mapping is needed

			// wait , if (acb deleted, check link or next port) timeout next CR
			DPRINTLNSVAL("SerialNodeNet::forward> send message to port : ",
					pport->getId());
			DPRINTLNHEADER(pHeader);
			pport->sendMessage(pMessage, messageSize);
			pport->listen();
			++cnt;
//TODO while acb.count > 0 checkConnection
		}
		pport = (SerialPort*) pport->getNext();
	}

	if (cnt) {
		//success
		return true;
	}

	DPRINTS("SerialNodeNet::Update> forward for CR failed: "); DPRINTLNADDR(pHeader->fromAddr); DPRINTLNADDR(pHeader->toAddr);
	return false;

}

void SerialNodeNet::processNodes(bool lifeCheck) {
	DPRINTLNS("SerialNodeNet::processNodes> [start]");
	SerialPort::readNextOnAllPorts();

	if (pProcessingNode == NULL) {
		pProcessingNode = getRootNode();

	}



	AcbList::removeOldAcbs();

	if (!pProcessingNode) {
		//DPRINTLNS("SerialNodeNet::processNodes> any nodes found");
		if (SoftSerialPort::count() > 1) {
			SoftSerialPort::cycleListenerPort();
		}
		return;
	}




	/*if (SoftSerialPort::count() > 1) {
		byte acbcnt = AcbList::countAll(
				SoftSerialPort::getListenerPort()->getId());
		if (acbcnt > 0) {
			pProcessingNode =
					(SerialNode*) pProcessingNode->cycleNextNodeOnPort();
			MPRINTLNSVAL("SerialNodeNet::processNodes> ACB's : ", acbcnt);
		} else {
			pProcessingNode = (SerialNode*) pProcessingNode->getNext();
		}
	} else {
		pProcessingNode = (SerialNode*) pProcessingNode->getNext();
	}*/

	if (AcbList::countAll() == 0) {
		pProcessingNode = (SerialNode*) pProcessingNode->getNext();
	}



	if (pProcessingNode == NULL) {
		pProcessingNode = getRootNode();
	}

	if (pProcessingNode->getPort()) {
		pProcessingNode->getPort()->listen();

	}

	XPRINTSVAL("SerialNodeNet::processNode :---------msec : ", millis());
	XPRINTLNSVAL(" -------------------------------- ", pProcessingNode->getId());

	ASSERTP(pProcessingNode, "SerialNodeNet::processNodes> no nodes found");

	if (lifeCheck) {
		checkConnection(pProcessingNode);
	}

}

void SerialNodeNet::setOnMessageHandler(OnMessageHandler* pOnMessageHandler) {
	this->pOnMessageHandler = pOnMessageHandler;
}

void SerialNodeNet::callOnMessage(const tSerialHeader* pHeader,
		const byte* pData, size_t dataSize, SerialNode* pNode) {
	if (pOnMessageHandler) {
		pOnMessageHandler->onMessage(pHeader, pData, dataSize, pNode);
	}
}

void SerialNodeNet::setOnPreConnectHandler(
		OnPreConnectHandler* pOnPreConnectHandler) {
	this->pOnPreConnectHandler = pOnPreConnectHandler;
}

void SerialNodeNet::callOnPreConnect(SerialNode* pNode) {
	if (pOnPreConnectHandler) {
		pOnPreConnectHandler->onPreConnect(pNode);
	}
}

LcbList* SerialNodeNet::getLcbList(){
	return &lcbList;
}

void SerialNodeNet::checkConnection(SerialNode* pNode, tStamp period) {
	DPRINTLNS("SerialNodeNet::checkConnection> [start]");
	tStamp now = millis();
	if (pNode->getLastLifeCheckTime()
			&& (now - pNode->getLastLifeCheckTime()) < period) {
		return;
	}

	tAcb* prevSendAcb = AcbList::getInstance()->getAcbEntry(
			pNode->getLastSendAktId());

	if (!pNode->isConnected()) {
		MPRINTLNS("SerialNodeNet::checkConnection> not connected");
		if (prevSendAcb && prevSendAcb->cmd == CMD_CR) {
			MPRINTLNS(
					"SerialNodeNet::checkConnection> CR already sent, waiting for reply ");
		} else {
			pNode->reconnect();
		}
	} else if (pNode->isLifeCheckExpired()) {
		// set all nodes setReady(false)
		MPRINTLNS("SerialNodeNet::checkConnection> isLifeCheckExpired");
		if (prevSendAcb && prevSendAcb->cmd == CMD_CR) {
			MPRINTLNS("SerialNodeNet::checkConnection> CR already sent, waiting for reply ");
		} else {
			pNode->reconnect();
		}

	} else if (pNode->isLifeCheckLate()) {
		//if (AcbList::getInstance()->count(pNode->getPort()->remoteSysId) == 0) { // node waited on other nodes
		//	pNode->lastReceiveTimeStamp = now; // reset time stamp
		//}

		DPRINTLNSVAL("SerialNodeNet::checkConnection> lastSendAktId: " ,pNode->getLastSendAktId());

		if (!prevSendAcb) {
			pNode->send(CMD_LIVE);
			//pNode->lastReceiveTimeStamp = now;
		} else {
			DPRINTLNS("SerialNodeNet::checkConnection> acb found, waiting for reply ");
		}
	}

	pNode->setLastLifeCheckTime(now);

}
;
}
;
