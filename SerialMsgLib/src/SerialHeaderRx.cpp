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
	deleteCallBackList();
}

/* for each addr one call back
 * callback must be installed before setReady(addr) is called*/
void SerialHeaderRx::setUpdateCallback(
		void (*ptr)(const byte* pData, size_t data_size), byte addr) {
	tCallBackMapper* pLast = getLastCallBackMapperEntry();
	tCallBackMapper* pNext = new tCallBackMapper();
	;
	if (pLast == NULL) {
		pCallBackMapperList = pNext;
	} else {
		pLast->pNext = pNext;
	}
	pNext->addr = addr;
	pNext->pUserCallBack = ptr;
}

bool SerialHeaderRx::isReadyToConnect(byte addr) {
	tCallBackMapper* pCallBackMapper = getCallBackMapperEntry(addr);
	return pCallBackMapper && (pCallBackMapper->status == CALLBACKMAPPER_STATUS_READY);
}

void SerialHeaderRx::setConnected(bool connected, byte addr) {
	tCallBackMapper* pCallBackMapper = getCallBackMapperEntry(addr);
	if (connected){
		if (isReadyToConnect(addr) ){
			pCallBackMapper->status =CALLBACKMAPPER_STATUS_CONNECTED;
		}
	}else{
		pCallBackMapper->status =CALLBACKMAPPER_STATUS_DISCONNECTED;
	}

}

//is called by serialRx when serialRx receives a message
void SerialHeaderRx::internalReceive(const byte* pData, size_t data_size) {
	tSerialHeader* pSerialHeader;
	byte cmd=pSerialHeader->cmd;
	pSerialHeader = (tSerialHeader*) pData;

	/* for received replies*/

	if (pSerialHeaderTx!=NULL) {

		bool reply = (cmd == SERIALHEADER_CMD_ACK  || cmd == SERIALHEADER_CMD_NAK
		                ||cmd == SERIALHEADER_CMD_DREP || cmd == SERIALHEADER_CMD_DRAQ  );
		if(reply) {
			pSerialHeaderTx->internalReceive(pData, data_size);
		}

	}


	if (cmd == SERIALHEADER_CMD_CR){
		if (isReadyToConnect(pSerialHeader->toAddr)) { //add and readystatus from CallBackMapper
			pSerialHeaderTx->replyACK(pSerialHeader->aktid);
			setConnected(true,pSerialHeader->toAddr);

		}else{
			pSerialHeaderTx->replyNAK(pSerialHeader->aktid);
		}
		return;
	}


	const byte* pUserData = pData + sizeof(pSerialHeader);
		tCallBackMapper* pNext = pCallBackMapperList;


	while (pNext != NULL) {
		if (pSerialHeader->toAddr == pNext->addr) {
			pNext->pUserCallBack(pUserData, data_size - sizeof(tSerialHeader));
		}
		pNext = (tCallBackMapper*) pNext->pNext;
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

tCallBackMapper* SerialHeaderRx::getLastCallBackMapperEntry() {
	tCallBackMapper* pLast = pCallBackMapperList;
	while (pLast != NULL && pLast->pNext != NULL) {
		pLast = (tCallBackMapper*) pLast->pNext;
	}
	return pLast;
}
tCallBackMapper* SerialHeaderRx::getCallBackMapperEntry(byte addr){
	tCallBackMapper* pCbm = pCallBackMapperList;
	while (pCbm != NULL && pCbm->addr != addr) {
		pCbm = (tCallBackMapper*) pCbm->pNext;
	}
	return pCbm;

}

void SerialHeaderRx::deleteCallBackList() {
	/* recursive way ->bad
	 if (pEntry!=NULL) {
	 if (((tCallBackMapper*)pEntry)->pNext !=NULL) {
	 deleteCallBackList(((tCallBackMapper*)pEntry)->pNext);
	 }
	 delete pEntry;
	 }
	 */
	tCallBackMapper* pLastEntry;
	while ((pLastEntry = getLastCallBackMapperEntry()) != NULL) {
		delete pLastEntry;
	}

}

