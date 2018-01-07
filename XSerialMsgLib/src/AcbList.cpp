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

AcbList* AcbList::getInstance(){
	return getList(SerialNodeNet::getInstance()->getSystemId());
}

AcbList* AcbList::getList(byte sysId){
	AcbList* pAcbList = pAcbListList;
	if (!pAcbListList) {
		pAcbListList = new AcbList(sysId);
	    return pAcbListList;
	}
	while(pAcbList) {
		if (pAcbList->getId() == sysId) {
			break;
		}
		if (!pAcbList->pNext) {
			pAcbList->pNext= new AcbList(sysId);
			pAcbList = (AcbList*)  pAcbList->pNext;
			break;
		}
		pAcbList = (AcbList*)  pAcbList->pNext;
	};
	return pAcbList;
}

AcbList::AcbList(byte sysId) {
	this->sysId=sysId;
	aktId = 0;
	XPRINTLNSVAL("AcbList::AcbList() free ",freeRam());
	XPRINTLNSVAL("AcbList::AcbList() created for system : ",sysId);
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
	MPRINTLNSVAL("AcbList::createAcb> aktId: ", pNew->aktid);
	MPRINTLNSVAL("AcbList::createAcb> count: ", count());
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
				MPRINTLNSVAL("AcbList::createOrUseAcb> reuse acb   : ", pAcb->aktid);
				pAcb->status = ACB_STATUS_OPEN;

				break;
			}
			pAcb = (tAcb*) pAcb->pNext;

		}
	}
	if (!pAcb) {
		pAcb = createAcb(pHeader);
	} else {
		DPRINTLNSVAL(" , new aktid: ", pAcb->aktid);
	}
	pAcb->timeStamp=millis();
	pAcb->aktid=pHeader->aktid;
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

