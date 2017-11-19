/*
 * SerialHeaderRx.cpp
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#include <tools.h>
#include "SerialHeader.h"
#include "SerialRx.h"
#include "SerialHeaderRx.h"
#include "SerialHeaderTx.h"
#include "SerialMsg.h"

SerialHeaderRx::SerialHeaderRx(SerialPort* pSerialPort, size_t maxDataSize) {
	this->pSerialRx = new SerialRx(pSerialPort,
			maxDataSize + sizeof(tSerialHeader));
	((SerialRx*) pSerialRx)->setSerialHeaderRx(this);

}

SerialHeaderRx::~SerialHeaderRx() {
	delete pSerialRx;
	deleteCcbList();
}

/* for each addr one call back
 * callback must be installed before setReady(addr) is called*/
void SerialHeaderRx::setUpdateCallback(
		void (*ptr)(const byte* pData, size_t data_size), byte localAddr, byte remoteAddr) {
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,true);
	pCcb->pUserCallBack = ptr;
}

bool SerialHeaderRx::isReadyToConnect(byte localAddr,byte remoteAddr) {
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,true);
	return pCcb && (pCcb->status == CONNECTION_STATUS_READY);
}

bool SerialHeaderRx::isConnected(byte localAddr,byte remoteAddr) {
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,true);
	return pCcb && (pCcb->status == CONNECTION_STATUS_CONNECTED);
}


bool SerialHeaderRx::setConnectionStatus(byte localAddr, byte remoteAddr,byte status) {
	tCcb* pCallBackMapper = getCcbEntry(localAddr,remoteAddr,true);
	pCallBackMapper->status =status;
	return true;
}

byte SerialHeaderRx::getConnectionStatus(byte localAddr, byte remoteAddr){
	tCcb* pCcb= getCcbEntry(localAddr, remoteAddr);
	if (pCcb) {
		return pCcb->status;
	}else{
		return 0;
	}
}


//is called by serialRx when serialRx receives a message
void SerialHeaderRx::internalReceive(const byte* pData, size_t data_size) {
	tSerialHeader* pSerialHeader=(tSerialHeader*) pData;

	byte cmd=pSerialHeader->cmd;
	/* for received replies*/

	if (getConnectionStatus(pSerialHeader->toAddr,pSerialHeader->fromAddr)!=CONNECTION_STATUS_CONNECTED) {
		if ((cmd != SERIALHEADER_CMD_ACK) &&
			(cmd != SERIALHEADER_CMD_NAK) &&
			(cmd != SERIALHEADER_CMD_CR)) {

			MPRINTSVAL("SerialHeaderRx::internalReceive> unallowed message in unconnected status :",pSerialHeader->aktid);
		}
		return;
	}

	if (pSerialHeaderTx!=NULL) {
		bool reply = (cmd == SERIALHEADER_CMD_ACK  || cmd == SERIALHEADER_CMD_NAK
		                ||cmd == SERIALHEADER_CMD_DREP || cmd == SERIALHEADER_CMD_DRAQ  );
		if(reply) {
			pSerialHeaderTx->internalReceive(pData, data_size);
			return;
		}
	}
	if (cmd == SERIALHEADER_CMD_CR){
		if (isReadyToConnect(pSerialHeader->toAddr,pSerialHeader->fromAddr)) { //add and readystatus from CallBackMapper
			pSerialHeaderTx->replyACK(pSerialHeader->aktid);
			setConnectionStatus(pSerialHeader->toAddr, pSerialHeader->fromAddr, CONNECTION_STATUS_CONNECTED);
		}else{
			pSerialHeaderTx->replyNAK(pSerialHeader->aktid);
		}
		return;
	}

	if (cmd == SERIALHEADER_CMD_CD){
			pSerialHeaderTx->replyACK(pSerialHeader->aktid);
			setConnectionStatus(pSerialHeader->toAddr, pSerialHeader->fromAddr, CONNECTION_STATUS_DISCONNECTED);
		return;
	}

	if (cmd == SERIALHEADER_CMD_LIVE){
		if (isConnected(pSerialHeader->toAddr,pSerialHeader->fromAddr)){
			pSerialHeaderTx->replyACK(pSerialHeader->aktid);
		}
	}

	const byte* pUserData = pData + sizeof(pSerialHeader);
		tCcb* pNext = pCcbList;


	while (pNext != NULL) {
		if (pSerialHeader->toAddr == pNext->localAddr) {
			pNext->pUserCallBack(pUserData, data_size - sizeof(tSerialHeader));
		}
		pNext = (tCcb*) pNext->pNext;
	}
}

bool SerialHeaderRx::waitOnMessage(byte*& pData, size_t& data_size,
		unsigned long timeout, unsigned long checkPeriod, byte addr,tAktId onAktId) {
	DPRINTLN("SerialHeaderRx::waitOnMessage");
	tSerialHeader* pHeader;
	if (timeout==0){
		timeout= WAITED_READ_TIMEOUT_DEFAULT_MSEC;
	};
	unsigned long endMillis = millis() + timeout;

	while (millis() < endMillis) {
		if (pSerialRx->waitOnMessage(pData, data_size, endMillis - millis(),
				checkPeriod)) {
			pHeader = (tSerialHeader*) pData;
			if (pHeader->toAddr == addr && (onAktId > 0)? (onAktId==pHeader->aktid): true ) {
				MPRINTSVAL(
						"SerialHeaderRx::waitOnMessage -message received ,addr: ",
						pHeader->toAddr);
				DPRINTSVAL("restOfTime: " ,endMillis-millis());
				return true;
			} else {
				MPRINTSVAL(
						"SerialHeaderRx::waitOnMessage -unknown! message received ,addr:",
						pHeader->toAddr);
				DPRINTSVAL("restOfTime: " ,endMillis-millis());
			}
		}
	} DPRINTSVAL("TIMEOUT! restOfTime:" ,endMillis-millis());
	return false;

}

bool SerialHeaderRx::waitOnMessage(byte*& pData, size_t& data_size,
		unsigned long timeout, byte addr,tAktId onAktId) {
	return waitOnMessage(pData, data_size, timeout,0, addr,onAktId);
}

tCcb* SerialHeaderRx::getLastCcbEntry() {
	tCcb* pLast = pCcbList;
	while (pLast != NULL && pLast->pNext != NULL) {
		pLast = (tCcb*) pLast->pNext;
	}
	return pLast;
}
tCcb* SerialHeaderRx::getCcbEntry(byte localAddr,byte remoteAddr,bool create){
	tCcb* pCcb = pCcbList;
	while (pCcb != NULL && pCcb->localAddr != localAddr && pCcb->remoteAddr != remoteAddr ) {
		pCcb = (tCcb*) pCcb->pNext;
	}
	if (!pCcb && create) {
		pCcb=getLastCcbEntry();
		pCcb->pNext= new tCcb();
		pCcb=(tCcb*)pCcb->pNext;
		pCcb->localAddr=localAddr;
		pCcb->remoteAddr=remoteAddr;
	}
	return pCcb;
}

void SerialHeaderRx::deleteCcbList() {
	/* recursive way ->bad
	 if (pEntry!=NULL) {
	 if (((tCallBackMapper*)pEntry)->pNext !=NULL) {
	 deleteCallBackList(((tCallBackMapper*)pEntry)->pNext);
	 }
	 delete pEntry;
	 }
	 */
	tCcb* pLastEntry;
	while ((pLastEntry = getLastCcbEntry()) != NULL) {
		delete pLastEntry;
	}

}
