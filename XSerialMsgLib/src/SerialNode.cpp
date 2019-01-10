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
#include "SerialNodeNet.h"

tAddr fromAddr;
tAddr toAddr;
tAktId aktid = 0;
byte cmd = 0;
byte par = 0;



namespace SerialMsgLib {

SerialNode::SerialNode(byte localNodeId, bool active, byte remoteSysId, byte remoteNodeId, SerialPort* pPort) {

	DPRINTLNS("SerialNode::SerialNode>");
	ASSERTP(SerialNodeNet::getInstance()->getSystemId() > 0, "please call System.init(yourSystemId) before you create nodes.");

	this->pSerialPort = pPort;
	this->pCcb = new tCcb();
	tAddr lAddr(SerialNodeNet::getInstance()->getSystemId(), localNodeId);
	tAddr rAddr(remoteSysId, remoteNodeId);

	pCcb->localAddr = lAddr;
	pCcb->remoteAddr = rAddr;
	setActive(active);

	SerialNode* pNode = SerialNodeNet::getInstance()->getRootNode();

	while (pNode && pNode->pNext) {
		pNode = (SerialNode*) pNode->pNext;
	}
	if (!pNode) {
		SerialNodeNet::getInstance()->setRootNode(this);
	} else {
		pNode->pNext = this;
	}DPRINTS("SerialNode:SerialNode> created : ");
	DPRINTLNADDR(pCcb->localAddr);

}

SerialNode::~SerialNode() {
	if (this->pNext) {
		SerialNode* pPrev = SerialNodeNet::getInstance()->getRootNode();
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
		XPRINTSVAL(" SerialNode::isLifeCheckLate> node late: " ,getId());XPRINTLNSVAL(" : ",millis() - lastReceiveTimeStamp);
		return true;
	}

	return false;

}

bool SerialNode::isLifeCheckExpired() {
	if (!isActive()) {
		return false;
	}


	if ((millis() - lastReceiveTimeStamp) > SERIALNODE_TIME_LIFECHECK_LATE_EXPIRED_MSEC) {
		XPRINTSVAL(" SerialNode::isLifeCheckExpired> node very late ,expired: " ,getId());XPRINTLNSVAL(" : ",millis() - lastReceiveTimeStamp);
		return true;
	}

	return false;
}

void SerialNode::onMessage(tSerialHeader* pSerialHeader, const byte* pData, size_t datasize, SerialPort* pPort) {

	DPRINTLNS("SerialNode::onMessage>");
	assert(pSerialHeader);
	DPRINTLNHEADER(pSerialHeader);
	assert(pData ? datasize > 0 : datasize == 0);
	assert(pPort);
	assert(pCcb->remoteAddr.sysId != SerialNodeNet::getInstance()->getSystemId());
	lastReceiveTimeStamp = millis();

	tSerialCmd cmd = pSerialHeader->cmd;
	tAcb* pAcb = NULL;
	/* for received replies*/

	bool connected = isConnected();
	bool userCall = false;
	bool acbNotFound = false;

	XPRINTSVALS("SerialNode::onMessage> node : ", getId(), "  <<< ");
	XPRINTSS(tSerialHeader::cmd2Str(cmd));
	XPRINTLNS(" <<<< ");
	DPRINTLNHEADER(pSerialHeader);

	if (!connected) {
		if (!(cmd == CMD_ACK || cmd == CMD_NAK || cmd == CMD_CR)) {
			DPRINTLNSVAL("SerialNode::onMessage> unallowed message, status <> connected, aktid  :",
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
			SerialNodeNet::getInstance()->callOnPreConnect(this);

			if (!isReadyToConnect()){
				DPRINTLNSVAL("SerialNode::onMessage> node not ready :" ,getId());
				return;
			}

			if (pCcb->remoteAddr.sysId == 0) {
				// connect, no matter who it is
				pCcb->remoteAddr = pSerialHeader->fromAddr;
				DPRINTLNS("pcb remote address:");
				DPRINTLNADDR(pCcb->remoteAddr);
			} else if (pCcb->remoteAddr != pSerialHeader->fromAddr) {
				// unknown node wants to connect, send NAK
				DPRINTSVAL("SerialNode::onMessage> unknown remote node wants to connect: ", pCcb->localAddr.nodeId);DPRINTLNS("SerialNode::onMessage> unknown remote node address:");
				DPRINTLNADDR(pCcb->remoteAddr);
				send(CMD_NAK, pSerialHeader->aktid);

			}
			this->pSerialPort = pPort;
			pCcb->status = CONNECTION_STATUS_CONNECTED;
			pCcb->remoteAddr = pSerialHeader->fromAddr;
			send(CMD_ACK, pSerialHeader->aktid);
			DPRINTLNSVAL("SerialNode::onMessage> CONNECTION_STATUS_CONNECTED node : ", pCcb->localAddr.nodeId);

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
		DPRINTSVALS("SerialNode::onMessage> CMD_ACD : ", getId(), "  <<< ");
		break;
	case CMD_ARQ:
		userCall=true;
		break;
	case CMD_ARP:
		userCall=true;
		pAcb = AcbList::getInstance()->getAcbEntry(pSerialHeader->aktid);
		if (!pAcb) {
			userCall = false;
			acbNotFound = true;
		}
		break;
	case CMD_ACK:
		pAcb = AcbList::getInstance()->getAcbEntry(pSerialHeader->aktid);
		if (pAcb) {
			switch (pAcb->cmd) {
			case CMD_CR:
				pSerialPort = pPort;
				if (!isActive()) {
					DPRINTSVAL("SerialNode::onMessage>ACK ON CR node is not active: ", pCcb->localAddr.nodeId);
				} else if (isConnected()) {
					DPRINTLNSVAL("SerialNode::onMessage>node is already connected: ", pCcb->localAddr.nodeId);

				} else if (!isReadyToConnect()) {
					DPRINTSVAL("SerialNode::onMessage>ACK ON CR node is not ready: ", pCcb->localAddr.nodeId);

				} else {
					pCcb->remoteAddr = pSerialHeader->fromAddr;
					pCcb->status = CONNECTION_STATUS_CONNECTED;
					pSerialPort = pPort;
					DPRINTLNSVAL("SerialNode::onMessage> CONNECTION_STATUS_CONNECTED node (active) : ", getId());
					DPRINTLNADDR(pCcb->localAddr)
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
		pAcb = AcbList::getInstance()->getAcbEntry(pSerialHeader->aktid);
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
		SerialNodeNet::getInstance()->callOnMessage(pSerialHeader, pData, datasize, this);
	}
	if (pAcb) {

		MPRINTLNSVAL("SerialNode::onMessage> round trip time : ", millis() - pAcb->timeStamp);

		unsigned int count = AcbList::getInstance()->count();

		DPRINTLNSVAL("SerialNode::onMessage> ACB COUNT : ",count );

		AcbList::getInstance()->deleteAcbEntry(pSerialHeader->aktid);
		assert(AcbList::getInstance()->count() < count);
		MPRINTFREE
		;

	} else {
		// old reply, acb already reused
		//assert(false);
	}

// user request /reply
}




void SerialNode::reconnect() {
	// call on reconnect();
	DPRINTLNS("SerialNode:reconnect()> [start]");
	setReady(false);
	if (isActive()) {
		SerialNodeNet::getInstance()->callOnPreConnect(this);
	}else{
		DPRINTLNSVAL("SerialNode:reconnect()> waiting for CR message, node: ", getId());
		return;
	}

	if (isReadyToConnect()) {
		DPRINTLNSVAL("SerialNode:reconnect()> send CR for node ", getId());
		send(CMD_CR);
	}else {
		DPRINTLNSVAL("SerialNode:reconnect> node not ready: ",getId());
		return;
	}
}



SerialPort* SerialNode::getPort() {
	return pSerialPort;
}


SerialNode* SerialNode::getNext() {
	return (SerialNode*) pNext;
}


SerialNode* SerialNode::cycleNextNodeOnPort() {
	SerialNode * _pnext =this;
	do {

		_pnext=_pnext->getNext();
		if (!_pnext) {
			_pnext=SerialNodeNet::getInstance()->getRootNode();
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


tAktId SerialNode::send(tSerialCmd cmd, tAktId replyOn, byte par, const byte* pData, size_t datasize, byte replyToSys,byte replyToNode) {

	tSerialHeader header, *pHeader;
	pHeader = &header;
	//delay(20);
	XPRINTS("SerialNode::send>  >>> ");XPRINTSS(tSerialHeader::cmd2Str(cmd));XPRINTLNS(" >>>");
	assert(pCcb->remoteAddr.sysId != SerialNodeNet::getInstance()->getSystemId());

// not connected
	if (pCcb->status != CONNECTION_STATUS_CONNECTED && !(cmd == CMD_CR || cmd == CMD_ACK || cmd == CMD_NAK)) {
		DPRINTLNSVAL("SerialNode::send > not connected, cmd not allowed : ", cmd);
		return 0;
	}




	header.fromAddr.sysId = SerialNodeNet::getInstance()->getSystemId();
	if (replyOn > 0) {
		DPRINTLNSVAL("SerialNode::send> reply to:", replyOn);
		header.aktid = replyOn;
		if (replyToSys > 0) {
			header.toAddr.sysId = replyToSys;
			header.toAddr.nodeId = replyToNode;

		} else {
			//DPRINTLNSVAL("Address of node: ", long((void* ) this));
			if (this->isConnected()) {
				header.toAddr = pCcb->remoteAddr;
			} else {
				DPRINTLNS("SerialNode::send> node, not connected , unknown remote address:");
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
					pPort = (SerialPort*) pPort->getNext();
				}DPRINTLNS("SerialNode::send> to port in list - success");
				return header.aktid;
			}
		}
	}DPRINTLNS("SerialNode::send> failed");
	return 0;
}

tAktId SerialNode::writeToPort(tSerialHeader* pHeader, const byte* pData, size_t datasize, SerialPort* pPort) {
	MPRINTSVALS("node : ", pHeader->fromAddr.nodeId, "  >>> ");
	DPRINTSVALS("SerialNode::writeToPort> node : ", pHeader->fromAddr.nodeId, "  >>> ");
	MPRINTSS(tSerialHeader::cmd2Str(pHeader->cmd));
	MPRINTLNSVAL(" >>> port: ", pPort->getId());

	// no outstanding replies required by CR or LIVE messages ( for better round trip time)
//	if (pHeader->cmd == CMD_LIVE && AcbList::getInstance()->count() >0  && pPort->isListening() ) {
//		DPRINTLNS("SerialNode::writeToPort> waiting for reply,  LIVE message canceled");
//		return 0;
//	}

	if (pPort->getType() == PORTTYPE_SOFTSERIAL && !pPort->isListening()) {
		SoftSerialPort* pListener = SoftSerialPort::getListenerPort();
		ASSERTP(pListener != pPort, "SerialNode::writeToPort> GLEICHER LISTENER ?");
		unsigned int listenerAcbCount = AcbList::countAll(pListener->getId());
		// open acbs on other SoftSerialPort
		if (listenerAcbCount > 0) {

			if (pHeader->cmd == CMD_LIVE) {
				MPRINTLNSVAL("SerialNode::writeToPort> waiting for replies on other soft serial port,  cancel, acb count: " ,listenerAcbCount);
				return 0;
			} else if (pHeader->cmd == CMD_ARQ) {
				MPRINTSVAL("SerialNode::writeToPort> waiting for replies on port: ",pListener->getId());
				MPRINTLNSVAL(" acb count: " ,listenerAcbCount);
				MPRINTFREE;
				tStamp now= millis();
				pListener->listen(); // to be sure
				while(listenerAcbCount > 0) {
					while (pListener->available() > 0)  {
						pListener->pPortRxTxMapper->getRx()->readNext();

					}
					tAcb* pAcb= AcbList::getInstance()->getLastestAcbEntry(pListener->getId());
					if (pAcb) {
						SerialNode* pWaitNode=SerialNodeNet::getInstance()->getNodeByAcb(pAcb);
						if (pWaitNode) {
							SerialNodeNet::getInstance()->checkConnection(pWaitNode);
						}
					}
					listenerAcbCount = AcbList::countAll(pListener->getId());
				}
				MPRINTSVAL("SerialNode::writeToPort> waiting end, node : ",getId());

				MPRINTLNSVAL(" waited for : " ,millis()-now);
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



	pPort->sendMessage(pHeader,pData,datasize);
	lastSendAcbAktId = pHeader->aktid;

	SerialNodeNet::getInstance()->setProcessingNode(this);
	DPRINTLNS("SerialNode::writeToPort> success , header...");
	DPRINTLNHEADER(pHeader);

	return pHeader->aktid;
}

bool SerialNode::waitOnReply(tAktId, tStamp) {

	MPRINTLNS("SerialNode::waitOnReply , not implemented yet");
	return false;

};
};
