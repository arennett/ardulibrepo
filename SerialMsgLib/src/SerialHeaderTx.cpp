/*
 * SerialHeaderTx.cpp
 *
 *  Created on: 14.11.2017
 *      Author: rea
 */

#include "stddef.h"
#include "Arduino.h"
#include "tools.h"
#include "SerialPort.h"
#include "SerialTx.h"
#include "SerialHeader.h"
#include "SerialHeaderTx.h"
#include "SerialHeaderRx.h"

SerialHeaderTx::SerialHeaderTx(SerialPort* pSerialPort) {
	this->pSerialHeaderRx = NULL;
	this->pSerialTx = new SerialTx(pSerialPort);
}

SerialHeaderTx::SerialHeaderTx(SerialHeaderRx* pSerialHeaderRx) {
	this->pSerialTx = new SerialTx(pSerialHeaderRx->getSerialPort());
	this->pSerialHeaderRx = pSerialHeaderRx;
	this->pSerialHeaderRx->setSerialHeaderTx(this);

}

SerialHeaderTx::~SerialHeaderTx() {
	delete pSerialTx;
}

tAcb* SerialHeaderTx::createAcb(tAktId aktId) {
	tAcb* pLast = getLastAcbEntry();
	tAcb* pNext = new tAcb();
	;
	if (pLast == NULL) {
		pAcbList = pNext;
	} else {
		pLast->pNext = pNext;
	}
	pNext->aktid = aktId;
	return pNext;
}

void SerialHeaderTx::mprintAcb(tAcb* pAcb){
	MPRINTSVAL("ACB      :" ,pAcb->aktid);
	MPRINTSVAL("CMD      :" ,pAcb->cmd);
	MPRINTSVAL("from     :" ,pAcb->fromAddr);
	MPRINTSVAL("to       :" ,pAcb->toAddr);
	MPRINTSVAL("retrials :" ,pAcb->cntRetries);
	MPRINTSVAL("status   :" ,pAcb->status);
	MPRINTSVAL("wait msec:" ,millis()-pAcb->timeStamp);
	MPRINTLN("--------------------");
};

void SerialHeaderTx::mprintAcbList() {
	MPRINTLN("--------------------");
	MPRINTSVAL("ACBLIST Count: ",getCountAcbEntries());
	tAcb* pAcb= pAcbList;
	while(pAcb){
		mprintAcb(pAcb);
		pAcb =(tAcb*)pAcb->pNext;
	}
};


tAcb* SerialHeaderTx::createOrUseAcb(byte cmd,byte fromAddr,byte toAddr,tAktId aktId){

	    tAcb* pAcb = pAcbList;
		while (pAcb) {
			if ( pAcb->cmd==cmd && pAcb->fromAddr==fromAddr && pAcb->toAddr==toAddr) {
				pAcb->aktid=aktId;
				pAcb->cntRetries++;
			    break;
			}
			pAcb = (tAcb*) pAcb->pNext;

		}
		if (!pAcb) {
			pAcb=createAcb(aktId);
		}
		pAcb->timeStamp=millis();
		return pAcb;

       return createAcb(aktId);
}

tAcb* SerialHeaderTx::getLastAcbEntry() {
	tAcb* pLast = pAcbList;
	while (pLast && pLast->pNext) {
		pLast = (tAcb*) pLast->pNext;
	}
	return pLast;
}

unsigned int SerialHeaderTx::getCountAcbEntries() {
	int count = 0;
	tAcb* pLast = pAcbList;
	if (pLast) {
		count++;
	}
	while (pLast && pLast->pNext) {
		pLast = (tAcb*) pLast->pNext;
		count++;
	}
	return count;
}

tAcb* SerialHeaderTx::getAcbEntry(tAktId aktId) {
	tAcb* pAcb = pAcbList;
	while (pAcb && pAcb->aktid != aktId) {
		pAcb = (tAcb*) pAcb->pNext;
	}
	return pAcb;
}

bool SerialHeaderTx::deleteAcbEntry(tAktId aktId) {

	tAcb* pAcb = getAcbEntry(aktId);
	if (pAcb) {
		if (pAcb->pNext) {
			tAcb* pPrev = pAcbList;
			while (pPrev && pPrev->pNext) {
				if (pPrev->pNext == pAcb) {
					break;
				}
				pPrev = (tAcb*) pPrev->pNext;
			}
			if (pPrev) {
				pPrev->pNext = pAcb->pNext;
				delete pAcb;
			}
		}
		return true;
	} else {

		return false;
	}
}

void SerialHeaderTx::deleteAcbList() {
	tAcb* pLastEntry;
	while ((pLastEntry = getLastAcbEntry()) != NULL) {
		delete pLastEntry;
	}
}

tAktId SerialHeaderTx::send(byte fromAddr, byte toAddr, byte cmd, byte *pData, size_t dataSize,tAktId aktId) {
	memset(&this->sHeader, 0, sizeof(tSerialHeader));
    if(aktId ==0) {// no reply ..new aktidTx
		if (aktidTx < MAX_AKTID) {
			aktidTx++;
		} else {
			aktidTx = 0;
		}
    }
	bool confirm = (cmd == SERIALHEADER_CMD_LIVE || cmd == SERIALHEADER_CMD_CR
		         || cmd == SERIALHEADER_CMD_DREQ || cmd == SERIALHEADER_CMD_DRAQ );
	if (confirm ) {
		//create acb
		tAcb* pAcb = NULL;
		if (cmd == SERIALHEADER_CMD_LIVE || cmd == SERIALHEADER_CMD_CR) {
			pAcb = createOrUseAcb(cmd,fromAddr,toAddr,aktidTx);
		} else {
			pAcb = createAcb(aktidTx);
		}

		pAcb->cmd=cmd;
		pAcb->fromAddr=fromAddr;
		pAcb->toAddr=toAddr;
		pAcb->status=ACB_STATUS_OPEN;
		pAcb->timeStamp=millis();
	}
	sHeader.cmd=cmd;
	sHeader.fromAddr=fromAddr;
	sHeader.toAddr = toAddr;
	sHeader.aktid=(aktId==0) ? aktidTx : aktId; // aktId > 0 use reply aktId
	pSerialTx->sendPreamble();
	pSerialTx->sendData((byte*)&sHeader, sizeof(sHeader));
	pSerialTx->sendData(pData, dataSize);
	pSerialTx->sendPostamble();
	if (confirm) {
		return aktidTx;
	}

	return 0;

}

void SerialHeaderTx::reply(byte cmd ,tAktId onAktid,byte *pData, size_t dataSize) {
	tSerialHeader* pHeader = (tSerialHeader*)pData;
	send(pHeader->toAddr, pHeader->fromAddr,cmd, pData, dataSize, onAktid);
}

bool SerialHeaderTx::sendAndWait(byte fromAddr, byte toAddr, byte cmd,	byte *pData, size_t datasize,unsigned long int timeout,tAktId onAktId) {
	send(fromAddr,toAddr,cmd,pData,datasize,onAktId);
	return pSerialHeaderRx->waitOnMessage(pData, datasize, timeout, toAddr, onAktId);
}


tAktId SerialHeaderTx::sendCR(byte fromAddr, byte toAddr){
	return send(fromAddr,toAddr,SERIALHEADER_CMD_CR,NULL,0,0);
}

tAktId SerialHeaderTx::connect(byte localAddr, byte remoteAddr){
	pSerialHeaderRx->setConnectionStatus(localAddr, remoteAddr, CONNECTION_STATUS_READY);
}

bool   waitOnConnectionsUp(unsigned int timeout);


void SerialHeaderTx::replyACK(tAktId onAktId){
	reply(SERIALHEADER_CMD_ACK, onAktId,NULL, 0);
}
void SerialHeaderTx::replyNAK(tAktId onAktId){
	reply(SERIALHEADER_CMD_NAK, onAktId,NULL, 0);
}

/* internal callback from SerialHeaderRx */
void SerialHeaderTx::internalReceive(const byte* pData, size_t data_size){
	tSerialHeader* pHeader = (tSerialHeader*) pData;


	byte cmd = pHeader->cmd;
	bool reply = (cmd == SERIALHEADER_CMD_ACK  || cmd == SERIALHEADER_CMD_NAK
                ||cmd == SERIALHEADER_CMD_DREP || cmd == SERIALHEADER_CMD_DRAQ  );

	if (reply) {

		tAcb* pAcb =getAcbEntry(pHeader->aktid);


		if(pAcb && pAcb->toAddr == pHeader->fromAddr) {
				// Action closed

			    if (pAcb->cmd==SERIALHEADER_CMD_ACK) {
			    	   pSerialHeaderRx->setConnectionStatus(pAcb->fromAddr, pAcb->toAddr, CONNECTION_STATUS_CONNECTED);
			    	   MPRINTSVAL("CONNECTED FROM: ",pAcb->fromAddr);
			    	   MPRINTSVAL("            TO: ",pAcb->toAddr);
			    	   // for a connection the acb is permanent???? NO!!!!
			    	   // status connect true in connection entry
			    	   // former callBackMap Entry in RX
			    	   // YES MUCH BETTER  !!!!!!!!

			    }else{
			    	pAcb->status=ACB_STATUS_CLOSED;
					MPRINTLN("ACB closed");
					mprintAcb(pAcb);
					deleteAcbEntry(pHeader->aktid);
			    }

		}else{
			MPRINTSVAL("SerialHeaderTx::internalReceive> acb not found: ",pHeader->aktid);
		}

		if(data_size >0 ){
			MPRINTSVAL("SerialHeaderTx::internalReceive> data found: ",pHeader->aktid);

		}


	}else{
		MPRINTLN("SerialHeaderTx::internalReceive> received a no-reply-message");

	}



}
