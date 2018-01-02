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

//static
SerialNodeNet* SerialNodeNet::pInst =NULL;

SerialNodeNet*  SerialNodeNet::init(byte systemId){
	if (pInst) {
		delete pInst;
	}

	pInst = new SerialNodeNet(systemId);
	return pInst;
}

SerialNodeNet::SerialNodeNet(byte systemId) {
	this->systemId=systemId;
}

SerialNodeNet::~SerialNodeNet() {
	// TODO Auto-generated destructor stub
}

byte SerialNodeNet::getSystemId(){
	return systemId;
}

SerialNode* SerialNodeNet::getProcessingNode() {
	return pProcessingNode;
}

void SerialNodeNet::setProcessingNode(SerialNode* pNode) {
	pProcessingNode=pNode;
}

SerialNode* SerialNodeNet::createNode(byte localNodeId, bool active, byte remoteSysId, byte remoteNodeId,
		SerialPort* pPort) {
	return new SerialNode(localNodeId, active, remoteSysId, remoteNodeId, pPort);
}


SerialNode* SerialNodeNet::getRootNode(){
	return pSerialNodeList;
}

void SerialNodeNet::setRootNode(SerialNode* pNode){
	pSerialNodeList=pNode;
}

bool SerialNodeNet::areAllNodesConnected() {
	static bool bStart=false;
	SerialNode* pNode = SerialNodeNet::pInst->getRootNode();
	bool connected = true;

	while (pNode) {
		connected = connected && pNode->isConnected();
		pNode = (SerialNode*) pNode->getNext();
	}
	if (connected ) {
		if (!bStart) {
			MPRINTLNS(" SerialNodeNet::areAllNodesConnected()> *** ALL NODES CONNECTED WITH REMOTE ***");
			//AcbList::instance.deleteAcbList();
			//AcbList::instance.printList();
			bStart = true;
		}
	}else{
		bStart=false;
	}
	return connected;
}
void SerialNodeNet::update(const byte* pMessage, size_t messageSize, SerialPort* pPort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	DPRINTLNSVAL("SerialNodeNet::update> size: ", messageSize);
	assert(pHeader);
	DPRINTLNHEADER(pHeader);

	size_t hsize = sizeof(tSerialHeader);
	size_t dsize = 0;
	if (messageSize >= hsize) {
		dsize = messageSize - hsize;
	}
	if (pHeader->cmd >= CMD_NAK && pHeader->cmd < CMD_ACD) { //System telegramm
		if (dsize > 0) {
			MPRINTLNSVAL("SerialNodeNet::update> message has corrupted size, expected : ", hsize);
			return;
		}
	}

	if (pHeader->toAddr.sysId == 0) {
		MPRINTLNS("SerialNodeNet::update> systemId must be > 0 ");
		return;
	}

	if (pHeader->toAddr.sysId == pInst->getSystemId()) {
		//local system
		SerialNode* pNode = pInst->getRootNode();
		if (!pNode) {
			MPRINTLNS("SerialNodeNet::update> no nodes found");
			assert(false);
		}

		if (pHeader->toAddr.nodeId > 0) {
			// find node in our system
			while (pNode) {
				if (pNode->getCcb()->localAddr == pHeader->toAddr) {
					// node found
					DPRINTLNS("SerialNodeNet::update> node found");
					pNode->onMessage(pHeader, (dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize, pPort);
					return;
				}
				pNode = (SerialNode*) pNode->getNext();
			}DPRINTLNS("SerialNodeNet::update> node not found : ");
			DPRINTLNHEADER(pHeader);
			return;
		}

		// send CR to first found passive but ready(to connect) node without remote node id
		// and whithout remote system id or equal system id to headers from system id

		if (pHeader->cmd == CMD_CR) {
			SerialNode* pNode = pInst->getRootNode();
			while (pNode) {
				if (!pNode->isActive() && pNode->isReadyToConnect()
						&& (pNode->getCcb()->remoteAddr.sysId == 0
								|| pNode->getCcb()->remoteAddr.sysId == pHeader->fromAddr.sysId)
						&& pNode->getCcb()->remoteAddr.nodeId == 0) {		// only as comment
					pNode->onMessage(pHeader, (dsize > 0) ? (byte*) pHeader + hsize : NULL, dsize, pPort);
					return;
				}
				pNode = (SerialNode*) pNode->getNext();
			}MPRINTLNS("SerialNodeNet::update> no ready node for CR found");
			return;
		} else {
			MPRINTLNS("SerialNodeNet::update> message without nodeId in toAddr received");
			return;
		}
		// end of local system
	}

	if (pHeader->fromAddr.sysId == pInst->getSystemId()) {
		MPRINTLNS("SerialNodeNet::update> remote system has the same id as local system , message discarded");
		return;
	}

	//remote system
	pInst->forward(pMessage, messageSize, pPort);

}

bool SerialNodeNet::forward(const byte* pMessage, size_t messageSize, SerialPort* pSourcePort) {
	tSerialHeader* pHeader = (tSerialHeader*) pMessage;
	MPRINTLNS("SerialNodeNet::forward>");
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
		MPRINTSVAL("SerialNodeNet::Update> no link found to system: ", pHeader->toAddr.sysId);
		return false;
	}

	if (lcbList.createLcb(pHeader, pSourcePort)) {
		MPRINTS("SerialNodeNet::Update> LINK created: ");
		MPRINTLNADDR(pHeader->fromAddr);
	}

	SerialPort* pport = SerialPort::pSerialPortList;

	byte cnt = 0;
	// send CR only to foreign systems, and not to system where CR was sent.
	while (pport) {
		if(pport->remoteSysId != pSourcePort->remoteSysId){
			pport->getTx()->sendData(pMessage, messageSize);
			++cnt;
		}
		pport = (SerialPort*) pport->pNext;
	}

	if (cnt) {
		//success
		return true;
	}

	MPRINTS("SerialNodeNet::Update> forward for CR failed: ");
	DPRINTLNADDR(pHeader->fromAddr);
	DPRINTLNADDR(pHeader->toAddr);
	return false;

}

void SerialNodeNet::processNodes(bool lifeCheck) {

	if (pProcessingNode == NULL) {
		pProcessingNode =  getRootNode();

	}

	if (SoftSerialPort::count()>1) {
		byte acbcnt=AcbList::instance.count(SoftSerialPort::getListenerPort()->remoteSysId);
		if (acbcnt > 0) {
			pProcessingNode = (SerialNode*) pProcessingNode->cycleNextNodeOnPort();
			MPRINTLNSVAL("SerialNodeNet::processNodes> ACB's : " ,acbcnt);
		}else{
			pProcessingNode = (SerialNode*) pProcessingNode->getNext();
		}
	}else{
		pProcessingNode = (SerialNode*) pProcessingNode->getNext();
	}

	//

	if (pProcessingNode == NULL) {
		pProcessingNode = getRootNode();
	}

	if (pProcessingNode->getPort()) {
		pProcessingNode->getPort()->listen();

	}

    MPRINTSVAL("SerialNodeNet::processNode :---------msec : ",millis());MPRINTLNSVAL(" -------------------------------- ",pProcessingNode->getId());

	ASSERTP(pProcessingNode, "SerialNodeNet::processNodes> no nodes found");



	if (lifeCheck) {
		checkConnection(pProcessingNode);
	}
	SerialPort::readNextOnAllPorts();
}


void SerialNodeNet::setOnMessageCallBack(
		void (*ptr)(const tSerialHeader* pHeader, const byte* pData, size_t datasize, SerialNode* pNode)) {
	pCallBackOnMessage = ptr;
}

void SerialNodeNet::callOnMessage(const tSerialHeader* pHeader, const byte* pData, size_t datasize, SerialNode* pNode){
	if (pCallBackOnMessage) {
		pCallBackOnMessage(pHeader,pData,datasize,pNode);
	}
}


void SerialNodeNet::setOnPreConnectCallBack(void (*ptr)(SerialNode* pNode)) {
	pCallBackOnPreConnect = ptr;
}

void SerialNodeNet::callOnPreConnect(SerialNode* pNode){
	if (pCallBackOnPreConnect) {
		pCallBackOnPreConnect(pNode);
	}
}

void SerialNodeNet::checkConnection(SerialNode* pNode, tStamp period) {

	tStamp now = millis();

	tAcb* pLatestAcb = AcbList::instance.getLastestAcbEntry();

	if (pLatestAcb) {


		if ((millis()-pLatestAcb->timeStamp) > SERIALNODE_TIME_LIFECHECK_REPLYTIME_EXPIRED_MSEC) {
			SerialNode* pNodeExpired = SerialNode::getNode(pLatestAcb);
			MPRINTSVAL("SerialNodeNet::checkConnection::isLifeCheckExpired> reply time expired for aktid: " ,pLatestAcb->aktid);
			MPRINTSVAL(" on node: " ,pNodeExpired ? pNodeExpired->getId(): 0);
			MPRINTLNSVAL(" round trip time : ",millis()-pLatestAcb->timeStamp);
			AcbList::instance.deleteAcbEntry(pLatestAcb->aktid);
		}
	}

	if (pNode->getLastLifeCheckTime() && (now - pNode->getLastLifeCheckTime()) < period) {
		return;
	}

	if (!pNode->isConnected()) {
		MPRINTLNS("SerialNodeNet::checkConnection> not connected");
		pNode->reconnect();
	} else if (pNode->isLifeCheckExpired()) {
		// set all nodes setReady(false)
		MPRINTLNS("SerialNodeNet::checkConnection> isLifeCheckExpired");
		pNode->reconnect();

	} else if (pNode->isLifeCheckLate()) {
		//if (AcbList::instance.count(pNode->getPort()->remoteSysId) == 0) { // node waited on other nodes
		//	pNode->lastReceiveTimeStamp = now; // reset time stamp
		//}
		tAcb* prevSendAcb = AcbList::instance.getAcbEntry(pNode->getLastSendAktId());
		MPRINTLNSVAL("SerialNodeNet::checkConnection> lastSendAktId: " ,pNode->getLastSendAktId());

		if (!prevSendAcb) {
			pNode->send(CMD_LIVE);
			//pNode->lastReceiveTimeStamp = now;
		}else{
			MPRINTLNS("SerialNodeNet::checkConnection> acb found, waiting for reply ");
		}
	}

	pNode->setLastLifeCheckTime(now);

}
