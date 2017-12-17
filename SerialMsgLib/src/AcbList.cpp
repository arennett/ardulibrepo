/*
 * AcbList.cpp
 *
 *  Created on: 26.11.2017
 *      Author: User
 */

#include <tools.h>
#include "AcbList.h"

AcbList::AcbList() {
	aktId=0;

}

tAcb* AcbList::getRoot() {
	return pRoot;
}


unsigned int AcbList::getNextAktId() {
	if (aktId >= 65535) {
		aktId = 0;
	}
	return ++aktId;
}

tAcb* AcbList::createAcb(tSerialHeader* pHeader) {
	tAcb* pLast = getLastAcbEntry();
	tAcb* pNew = new tAcb();

	;
	if (!pLast) {
		pRoot = pNew;
	} else {
		pLast->pNext = pNew;
	}
	pNew->cmd   	= pHeader->cmd;
	pNew->fromAddr 	= pHeader->fromAddr;
	pNew->toAddr   	= pHeader->toAddr;
	pNew->status 	= ACB_STATUS_CREATED;
	pHeader->aktid	= pNew->aktid = getNextAktId();
	MPRINTLNSVAL("AcbList::createAcb> aktId: ",pNew->aktid);
	MPRINTLNSVAL("AcbList::createAcb> count: ",count());
	return pNew;

}

tAcb* AcbList::createOrUseAcb(tSerialHeader* pHeader) {
	tAcb* pAcb = pRoot;
	while (pAcb) {
		if (pAcb->cmd == pHeader->cmd && pAcb->fromAddr == pHeader->fromAddr
			&& pAcb->toAddr == pHeader->toAddr) {
				//reuse open (unacknowledged) acb
			    MPRINTSVAL("AcbList::createOrUseAcb> reuse acb: ",pAcb->aktid);
				pHeader->aktid = pAcb->aktid = getNextAktId();
				pAcb->status = ACB_STATUS_OPEN;

			break;
		}
		pAcb = (tAcb*) pAcb->pNext;

	}
	if (!pAcb) {
		pAcb = createAcb(pHeader);
	}else{
		MPRINTLNSVAL(" , new aktid: ",pAcb->aktid);
	}


	//pAcb->timeStamp = millis();
	return pAcb;
}

tAcb* AcbList::getAcbEntry(tAktId aktId) {
	tAcb* pAcb = pRoot;
	while (pAcb && pAcb->aktid != aktId) {
		pAcb = (tAcb*) pAcb->pNext;
	}
	return pAcb;
}

tAcb* AcbList::getAcbEntry(tCcb* pCcb,byte cmd){
	tAcb* pAcb = pRoot;

	while (pAcb && !(pAcb->cmd == cmd && pAcb->fromAddr == pCcb->localAddr && pAcb->toAddr == pCcb->remoteAddr)) {
		pAcb = (tAcb*) pAcb->pNext;
	}

	return pAcb;
}


//void 	mprintAcb(tAcb* pAcb);
tAcb* AcbList::getLastAcbEntry() {
	tAcb* pLast = pRoot;
	while (pLast && pLast->pNext) {
		pLast = (tAcb*) pLast->pNext;
	}
	return pLast;
}

unsigned int AcbList::count(){
	MPRINTLNSVAL("address of proot : ", ((int)(void*) pRoot));
	tAcb* p = pRoot;
	unsigned int cnt= 0;
		while (p ) {
		assert(p!=p->pNext);
		++cnt;
		p= (tAcb*) p->pNext;
		MPRINTLNSVAL("cnt: " ,cnt);
	}
	return cnt;;
}

//unsigned int AcbList::getCountAcbEntries();

void AcbList::deleteAcbList() {
	tAcb* pLastEntry;
	while ((pLastEntry = getLastAcbEntry()) != NULL) {
		delete pLastEntry;
	}
}

bool AcbList::deleteAcbEntry(tAktId aktId) {
	MPRINTLNSVAL("AcbList::deleteAcbEntry>2> ",aktId);
	tAcb* pAcb = getAcbEntry(aktId);
	if (pAcb) {
		if (pAcb->pNext) {
			tAcb* pPrev = pRoot;
			while (pPrev && pPrev->pNext) {
				if (pPrev->pNext == pAcb) {
					break;
				}
				pPrev = (tAcb*) pPrev->pNext;
			}
			if (pPrev) { // set next of previos to next of acb to be deleted
				pPrev->pNext = pAcb->pNext;
			}
		}
		delete pAcb;


		MPRINTLNSVAL("AcbList::deleteAcbEntry> count: ",count());
		return true;
	} else {
		MPRINTLNSVAL("AcbList::deleteAcbEntry> NOT FOUND: ",aktId);
		return false;
	}
}

bool  	AcbList::deleteAcbEntry(tCcb* pCcb,byte cmd) {
	tAcb* pAcb = getAcbEntry(pCcb,cmd);
	if (pAcb) {
		return deleteAcbEntry(pAcb->aktid);
	}

	return false;
}

AcbList::~AcbList() {
	deleteAcbList();
}

