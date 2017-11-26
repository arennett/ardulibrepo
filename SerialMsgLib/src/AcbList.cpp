/*
 * AcbList.cpp
 *
 *  Created on: 26.11.2017
 *      Author: User
 */

#include "AcbList.h"

AcbList::AcbList() {
	// TODO Auto-generated constructor stub

}

	tAcb* 	AcbList::getRoot(){
		return pRoot;
	}
	tAcb* 	AcbList::createAcb(tAktId aktId) {
		tAcb* pLast = getLastAcbEntry();
		tAcb* pNext = new tAcb();
		;
		if (pLast == NULL) {
			pRoot = pNext;
		} else {
			pLast->pNext = pNext;
		}
		pNext->aktid = aktId;
		return pNext;

	}
	tAcb* 	AcbList::createOrUseAcb(byte cmd, byte fromAddr, byte toAddr, tAktId aktId){
		tAcb* pAcb = pRoot;
			while (pAcb) {
				if (pAcb->cmd == cmd && pAcb->fromAddr == fromAddr
						&& pAcb->toAddr == toAddr) {
					pAcb->aktid = aktId;
					pAcb->cntRetries++;
					break;
				}
				pAcb = (tAcb*) pAcb->pNext;

			}
			if (!pAcb) {
				pAcb = createAcb(aktId);
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

	//void 	mprintAcb(tAcb* pAcb);
tAcb* AcbList::getLastAcbEntry(){
		tAcb* pLast = pRoot;
		while (pLast && pLast->pNext) {
			pLast = (tAcb*) pLast->pNext;
		}
		return pLast;
	}

	//unsigned int AcbList::getCountAcbEntries();

	void 	AcbList::deleteAcbList(){
		tAcb* pLastEntry;
		while ((pLastEntry = getLastAcbEntry()) != NULL) {
			delete pLastEntry;
		}
	}

	bool AcbList::deleteAcbEntry(tAktId aktId) {

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


AcbList::~AcbList() {
	deleteAcbList();
}

