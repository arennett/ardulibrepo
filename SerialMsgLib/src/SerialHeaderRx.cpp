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

void SerialHeaderRx::addConnection(byte localAddr, byte remoteAddr,bool master){
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,true);
	pCcb->status = CONNECTION_STATUS_NOT_READY;
	pCcb->master=master;
}


/* for each addr one call back
 * callback must be installed before setReady(addr) is called*/
void SerialHeaderRx::setUpdateCallback(
	void (*ptr)(const byte* pData, size_t data_size), byte localAddr, byte remoteAddr) {
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,false);
	if (!pCcb) {
		MPRINTLN("SerialHeaderRx::isReadyToConnect :Connection not found: ");
		MPRINT(localAddr);MPRINT("-");MPRINTLN(remoteAddr);
		return;
	}

	pCcb->pUserCallBack = ptr;
}

bool SerialHeaderRx::isReadyToConnect(byte localAddr,byte remoteAddr) {
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,false);
	if (!pCcb) {
		MPRINTLN("SerialHeaderRx::isReadyToConnect :Connection not found: ");
		MPRINTHEX(localAddr);MPRINT("-");MPRINTHEX(remoteAddr);
		return NULL;
	}
	return pCcb && (pCcb->status == CONNECTION_STATUS_READY);
}

bool SerialHeaderRx::isConnected(byte localAddr,byte remoteAddr) {
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,false);
		if (!pCcb) {
			MPRINTLN("SerialHeaderRx::isConnected :Connection not found: ");
			MPRINTHEX(localAddr);MPRINT("-");MPRINTHEX(remoteAddr);
		    return false;
		}
	return pCcb && (pCcb->status == CONNECTION_STATUS_CONNECTED);
}

bool SerialHeaderRx::setConnectionStatus(byte localAddr, byte remoteAddr,byte status) {
	tCcb* pCcb = getCcbEntry(localAddr,remoteAddr,false);
	if (!pCcb) {
		MPRINTLN("SerialHeaderRx::setConnectionStatus :Connection not found: ");
		MPRINTHEX(localAddr);MPRINT("-");MPRINTHEX(remoteAddr);
	    return false;
	}
	pCcb->status =status;
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


/* Is called by serialRx when serialRx receives a message
 *
 */
void SerialHeaderRx::internalReceive(const byte* pData, size_t data_size) {
	tSerialHeader* pSerialHeader=(tSerialHeader*) pData;

	byte cmd=pSerialHeader->cmd;
	/* for received replies*/

	if (!isConnected(pSerialHeader->toAddr,pSerialHeader->fromAddr)) {
		if ((cmd != SERIALHEADER_CMD_ACK) &&
			(cmd != SERIALHEADER_CMD_NAK) &&
			(cmd != SERIALHEADER_CMD_CR)) {

			MPRINTSVAL("SerialHeaderRx::internalReceive> unallowed message in unconnected status :",pSerialHeader->aktid);
		}
		return;
	}


	// protocol requests from remote
	 if (getTx()) {
		 if (cmd == SERIALHEADER_CMD_CR){
			if ( isReadyToConnect(pSerialHeader->toAddr,pSerialHeader->fromAddr) ) { //add and readystatus from CallBackMapper
				getTx()->replyACK(pSerialHeader->aktid);
				setConnectionStatus(pSerialHeader->toAddr, pSerialHeader->fromAddr, CONNECTION_STATUS_CONNECTED);
			}else{
				getTx()->replyNAK(pSerialHeader->aktid);
			}
			return;
		}

		 if (isConnected(pSerialHeader->toAddr,pSerialHeader->fromAddr)){
			if (cmd == SERIALHEADER_CMD_CD){
				pSerialHeaderTx->replyACK(pSerialHeader->aktid);
				setConnectionStatus(pSerialHeader->toAddr, pSerialHeader->fromAddr, CONNECTION_STATUS_DISCONNECTED);
			}else if (cmd == SERIALHEADER_CMD_LIVE){
				pSerialHeaderTx->replyACK(pSerialHeader->aktid);
			}
			return;
	 	 }// of isConnected

	 }//getTx()

	 //protocol replies from remote

	if(cmd == SERIALHEADER_CMD_ACK  || cmd == SERIALHEADER_CMD_NAK || cmd == SERIALHEADER_CMD_DREP ) {
		if (getTx()){
			if (!pSerialHeaderTx->internalReceive(pData, data_size)){
					return ; //no user callback for receiveid reply  needed;
			}
		}
	}

	// user request /reply
	const byte* pUserData = pData + sizeof(pSerialHeader);
		tCcb* pNext = pCcbList;


	while (pNext != NULL) {
		if (pSerialHeader->toAddr == pNext->localAddr) {
			pNext->pUserCallBack(pUserData, data_size - sizeof(tSerialHeader));
		}
		pNext = (tCcb*) pNext->pNext;
	}
}

bool SerialHeaderRx::connect(unsigned long int timeout, unsigned long int reqPeriod){
	bool up = false;
	tCcb* pCcb =this->pCcbList;
	if (timeout==0){
		timeout= WAITED_READ_TIMEOUT_DEFAULT_MSEC;
	};
	unsigned long endMillis = millis() + timeout;
	unsigned long reqMillis =0;
		up=true;
	if (pCcb) {
		bool up = true;;
		while (millis() < endMillis) {

			if (pCcb->status!=CONNECTION_STATUS_CONNECTED) {
				if (pCcb->master) {
					if (millis() < reqMillis) {

						pSerialHeaderTx->sendCR(pCcb->localAddr,pCcb->remoteAddr);
						reqMillis = millis() + reqPeriod;
					}
				}
				up=false;
			}
			if (!pCcb->pNext) { // the last one
				if(up) { // all connections must be up
					MPRINTLN("SerialHeaderRx::connect: UP");
					return up;
				}
				up=true; // next trial
				pCcb=pCcbList;
				delay(10);
			}else{
				pCcb= (tCcb*)pCcb->pNext;
			}

		}
	}
	if (millis() >= endMillis) {
		MPRINTLN("SerialHeaderRx::connect: TIMEOUT");
	}else{
		MPRINTLN("SerialHeaderRx::connect: NO CONNECTION FOUND");
	}
	return up; // no connections or timeout
}


bool SerialHeaderRx::waitOnMessage(byte*& rpData, size_t& rdata_size,
		unsigned long timeout, unsigned long checkPeriod, byte toAddr,tAktId onAktId) {
	DPRINTLN("SerialHeaderRx::waitOnMessage");
	tSerialHeader* pHeader;
	if (timeout==0){
		timeout= WAITED_READ_TIMEOUT_DEFAULT_MSEC;
	};
	unsigned long endMillis = millis() + timeout;

	while (millis() < endMillis) {
		if (pSerialRx->waitOnMessage(rpData, rdata_size, endMillis - millis(),
				checkPeriod)) {
			pHeader = (tSerialHeader*) rpData;
			if (pHeader->toAddr == toAddr && (onAktId > 0)? (onAktId==pHeader->aktid): true ) {
				MPRINTSVAL(
						"SerialHeaderRx::waitOnMessage -message received ,toAddr: ",
						pHeader->toAddr);
				DPRINTSVAL("restOfTime: " ,endMillis-millis());
				return true;
			} else {
				MPRINTSVAL(
						"SerialHeaderRx::waitOnMessage -unknown! message received ,toAddr:",
						pHeader->toAddr);
				DPRINTSVAL("restOfTime: " ,endMillis-millis());
			}
		}
	} DPRINTSVAL("TIMEOUT! restOfTime:" ,endMillis-millis());
	return false;

}

bool SerialHeaderRx::waitOnMessage(byte*& rpData, size_t& rdata_size,
		unsigned long timeout, byte toAddr,tAktId onAktId) {
	return waitOnMessage(rpData, rdata_size, timeout,0, toAddr,onAktId);
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
	while (!pCcb){
	   if( pCcb->localAddr == localAddr && pCcb->remoteAddr == remoteAddr ) {
		   // found
		   return pCcb;
	   }
		pCcb = (tCcb*) pCcb->pNext;
	}

	// not found
	if(create){
		pCcb=getLastCcbEntry();
		if (!pCcb ) { //emplTy List
			pCcbList= new tCcb();
			pCcb=pCcbList;
			MPRINTLN("SerialHeaderRx::getCcbEntry> FIRST Ccb created");
		}else {
			pCcb->pNext= new tCcb();
			pCcb=(tCcb*)pCcb->pNext;

			MPRINTLN("SerialHeaderRx::getCcbEntry> ANOTHER Ccb created");
		}
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

void SerialHeaderRx::mprintCcb(tCcb* pCcb){
	  MPRINTLN("CCB-----------------");
	MPRINTSVAL("local     : " ,pCcb->remoteAddr);
	MPRINTSVAL("remote    : " ,pCcb->localAddr);
	MPRINTSVAL("status    : " ,pCcb->status);
	  MPRINTLN("--------------------");
};

void SerialHeaderRx::mprintCcbList() {
	  MPRINTLN("--------------------");
	  tCcb* pCcb= pCcbList;
	  while(pCcb){
		mprintCcb(pCcb);
		pCcb =(tCcb*)pCcb->pNext;
	  }
};


