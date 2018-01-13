/*
 * AcbList.cpp
 *
 *  Created on: 26.11.2017
 *      Author: User
 */

#include <tools.h>
#include "SerialNodeNet.h"
#include "AcbList.h"


namespace SerialMsgLib {


AcbList* AcbList::pAcbListList=NULL;


AcbList* AcbList::getRootAcbList(){
	return pAcbListList;
}

AcbList* AcbList::getInstance(){
	return getList(SerialNodeNet::getInstance()->getSystemId(),true);
}


AcbList* AcbList::getList(byte sysId,bool create){
	AcbList* pAcbList = pAcbListList;
	if (!pAcbListList) {
		pAcbListList = new AcbList(sysId);
	    return pAcbListList;
	}
	while(pAcbList) {
		if (pAcbList->getId() == sysId) {
			break;
		}
		if (!pAcbList->pNext) { // end is reached
			if (create) {
				pAcbList->pNext= new AcbList(sysId);
			}
			pAcbList = (AcbList*)  pAcbList->pNext;
			break;
		}
		pAcbList = (AcbList*)  pAcbList->pNext;
	};
	return pAcbList;
}

tStamp AcbList::lastAcbCheck=0;
void AcbList::removeOldAcbs(){
	tStamp now =millis();
	if ((now-lastAcbCheck) < ACB_REPLYTIME_EXPIRED_CHECK_PERIOD_MSEC){
		return ;
	}
	AcbList* pAcbList = AcbList::getRootAcbList();
	while (pAcbList) {
		unsigned int acb_count =pAcbList->count();
		if (acb_count > 0) {
			tAcb* pAcb = pAcbList->getRoot();
			DPRINTLNSVAL("AcbList::removeOldAcbs> check acbs (start) sys: ",pAcbList->getId());
			DPRINTLNSVAL(" acbs count : ",acb_count);
			while (pAcb) {
				if ((millis()-pAcb->timeStamp) > ACB_REPLYTIME_EXPIRED_MSEC) {

					MPRINTLNSVAL("AcbList::removeOldAcbs>::isLifeCheckExpired> reply time expired for aktid: " ,pAcb->aktid);
					MPRINTS("cmd : ");MPRINTLNSS(tSerialHeader::cmd2Str(pAcb->cmd));
					MPRINTS("from: ");MPRINTLNADDR(pAcb->fromAddr);
					MPRINTS("to  : ");MPRINTLNADDR(pAcb->toAddr);
					MPRINTLNSVAL(" round trip time : ",millis()-pAcb->timeStamp);
					tAcb* pNext = (tAcb*) pAcb->pNext; //save next pointer
					pAcbList->deleteAcbEntry(pAcb->aktid);
					pAcb=pNext;
				}else{
					pAcb=(tAcb*) pAcb->pNext;
				}
			}

			acb_count =pAcbList->count();
			DPRINTLNSVAL("AcbList::removeOldAcbs> check acbs (end) sys: ",pAcbList->getId());
			DPRINTLNSVAL("acbs count : ",acb_count);
		}
		pAcbList=(AcbList*) pAcbList->pNext;
	}
}

AcbList::AcbList(byte sysId) {
	this->sysId=sysId;
	aktId = 0;
	XPRINTSVAL("AcbList::AcbList(",sysId);XPRINTLNSVAL(") free ",freeRam());
}

AcbList::~AcbList() {
	deleteAcbList();
}

byte AcbList::getId(){
	return sysId;
}


tAcb* AcbList::getRoot() {
	return pRoot;
}

void    AcbList::setAktId(tAktId aktId){
	this->aktId=aktId;
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
	pNew->cmd = pHeader->cmd;
	pNew->fromAddr = pHeader->fromAddr;
	pNew->toAddr = pHeader->toAddr;
	pNew->status = ACB_STATUS_CREATED;
	if (!pHeader->aktid) {
		pHeader->aktid = pNew->aktid = getNextAktId();
	}else {
		pNew->aktid = pHeader->aktid;
	}
	pNew->timeStamp=millis();
	MPRINTSVAL("AcbList::createAcb> sys: ",sysId);MPRINTLNSVAL(" aktId: ", pNew->aktid);
	MPRINTSVAL("AcbList::createAcb> count : ", count());MPRINTLNSVAL(" all :", countAll());
	return pNew;

}

tAcb* AcbList::createOrUseAcb(tSerialHeader* pHeader) {
	tAcb* pAcb = NULL;
	if (pHeader->cmd==CMD_CR|| pHeader->cmd==CMD_LIVE || pHeader->cmd==CMD_CD){
		pAcb = pRoot;
		while (pAcb) {
			if (pAcb->cmd == pHeader->cmd
					&& pAcb->fromAddr == pHeader->fromAddr
					&& pAcb->toAddr == pHeader->toAddr) {
				//reuse open (unacknowledged) acb
				pAcb->status = ACB_STATUS_OPEN;

				break;
			}
			pAcb = (tAcb*) pAcb->pNext;

		}
	}
	if (!pAcb) {
		pAcb = createAcb(pHeader);


	} else {
		pAcb->aktid=pHeader->aktid;
		pAcb->timeStamp=millis();
		MPRINTSVAL("AcbList::reUseAcb> sys: ",sysId);MPRINTLNSVAL(" aktId: ", pAcb->aktid);
		MPRINTSVAL("AcbList::reUseAcb> count : ", count());MPRINTLNSVAL(" all :", countAll());

	}



	return pAcb;
}

tAcb* AcbList::getAcbEntry(tAktId aktId) {
	tAcb* pAcb = pRoot;
	while (pAcb && pAcb->aktid != aktId) {
		pAcb = (tAcb*) pAcb->pNext;
	}
	return pAcb;
}

tAcb* AcbList::getLastestAcbEntry(byte portId) {
	tAcb* pAcb = pRoot;

	while (pAcb) {
		if (pAcb->portId==portId){
			break;
		}
		pAcb = (tAcb*) pAcb->pNext;
	}

	tAcb* pLatestAcb = pAcb;
	while (pAcb) {
		if (pAcb->portId==portId){
			if (pAcb->timeStamp < pLatestAcb->timeStamp) {
				pLatestAcb=pAcb;
			}
		}
		pAcb = (tAcb*) pAcb->pNext;
	}
	return pLatestAcb;
}




tAcb* AcbList::getAcbEntry(tCcb* pCcb, byte cmd) {
	tAcb* pAcb = pRoot;

	while (pAcb
			&& !(pAcb->cmd == cmd && pAcb->fromAddr == pCcb->localAddr
					&& pAcb->toAddr == pCcb->remoteAddr)) {
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

unsigned int AcbList::count(byte portId) {
		tAcb* p = pRoot;
	unsigned int cnt = 0;
	while (p){
		if (portId ? p->portId==portId: true) {
			++cnt;
		}
		p = (tAcb*) p->pNext;
	}
	return cnt;
}

unsigned int AcbList::countAll(byte portId) {
	unsigned int cnt=0;
	AcbList* pAcbList =pAcbListList;
	while(pAcbList) {
		cnt+= pAcbList->count(portId);
		pAcbList = (AcbList*) pAcbList->pNext;
	}
	return cnt;
}



void AcbList::deleteAcbList() {
	while (pRoot) {
		deleteAcbEntry(pRoot->aktid);
	}
}

bool AcbList::deleteAcbEntry(tAktId aktId) {
	MPRINTLNSVAL("AcbList::deleteAcbEntry> ", aktId);
	tAcb* pAcb = getAcbEntry(aktId);

	if (pAcb) {
		if (pAcb->pNext) {
			if (pAcb == pRoot) {
				pRoot = (tAcb*)pRoot->pNext;
			} else {
				tAcb* pPrev = pRoot;
				while (pPrev && pPrev->pNext) {
					if (pPrev->pNext == pAcb) {
						pPrev->pNext = pAcb->pNext;
						break;
					}
					pPrev = (tAcb*) pPrev->pNext;
				}
			}
		} else {
			if (pAcb == pRoot) {
				pRoot = NULL;
			} else {
				tAcb* pPrev = pRoot;
				while (pPrev && pPrev->pNext) {
					if (pPrev->pNext == pAcb) {
						pPrev->pNext = NULL;
						break;
					}
					pPrev = (tAcb*) pPrev->pNext;
				}
			}

			//set prev->pNext=null

		}
		delete pAcb;

		DPRINTLNSVAL("AcbList::deleteAcbEntry> count: ", count());
		return true;
	} else {
		DPRINTLNSVAL("AcbList::deleteAcbEntry> NOT FOUND: ", aktId);
		return false;
	}
}

bool AcbList::deleteAcbEntry(tCcb* pCcb, byte cmd) {
	tAcb* pAcb = getAcbEntry(pCcb, cmd);
	if (pAcb) {
		return deleteAcbEntry(pAcb->aktid);
	}

	return false;
}

void AcbList::printList(){
	tAcb* pEntry=pRoot;
	MPRINTLNSVAL("AcbList::printList>>>>>>>>begin>>>>>>>> count: " ,count());
	while (pEntry) {
		DPRINTLNSVAL("AcbList::printList> ACB: ", pEntry->aktid);
		pEntry=(tAcb*)pEntry->pNext;
	}
	DPRINTLNS("AcbList::printList<<<<<<<<end<<<<<<<<<<<<<<<<");
}
};

