/*
 * LcbList.cpp
 *
 *  Created on: 30.11.2017
 *      Author: rea
 */

#include "LcbList.h"

LcbList::LcbList() {
	// TODO Auto-generated constructor stub

}

LcbList::~LcbList() {
	// TODO Auto-generated destructor stub
}

tLcb* LcbList::getRoot() {
	return pRoot;
}
tLcb* LcbList::createLcb(tSerialHeader* pHeader, SerialPort* pFromPort) {

	if (pHeader->cmd != CMD_CR) { //create a link only by connection request
		return NULL;
	}

	tLcb* pLcb = getOpenLcb(pHeader);

	if (pLcb) { //already one created update the aktid
		pLcb->aktid = pHeader->aktid;
		return pLcb;
	}

	pLcb = getLinkedLcb(pHeader);
	if (pLcb) { //a link was found , open the link (reconnect)
		pLcb->aktid = pHeader->aktid;
		pLcb->pPortB = NULL;
		return pLcb;
	}

	pLcb = new tLcb();
	tLcb* pLast = getLastLcbEntry();
	if (pLast) {
		pLast->pNext = pLcb;
	} else {
		pRoot = pLcb;
	}
	pLcb->addrA = pHeader->fromAddr;
	pLcb->pPortA = pFromPort;
	pLcb->aktid = pHeader->aktid;

	return pLcb;
}

tLcb* LcbList::getLinkedLcb(tSerialHeader* pHeader) {
	tLcb* pLcb = pRoot;
	while (pLcb) {
		if (pLcb->pPortA && pLcb->pPortB
				&& (pLcb->addrA == pHeader->fromAddr
						|| pLcb->addrA == pHeader->toAddr)
				&& (pLcb->addrB == pHeader->fromAddr
						|| pLcb->addrB == pHeader->toAddr)) {	// link found
			return pLcb;
		}
		pLcb = pLcb->pNext;
	}
	return NULL;
}

tLcb* LcbList::getOpenLcb(tSerialHeader* pHeader) {
	tLcb* pLcb = pRoot;
	while (pLcb) {
		if (pLcb->pPortA && !(pLcb->pPortB)
				&& (pLcb->addrA == pHeader->fromAddr && pHeader->cmd == CMD_CR)
				&& (pLcb->addrA == pHeader->toAddr && pHeader->cmd == CMD_ACK)
				&& pLcb->aktid == pHeader->aktid) {
			return pLcb;
		}

		pLcb = pLcb->pNext;
	}
	return NULL;
}

//void 	mprintAcb(tAcb* pAcb){}
tLcb* LcbList::getLastLcbEntry() {
	tLcb* pLast = pRoot;
	while (pLast && pLast->pNext) {
		pLast = pLast->pNext;
	}
	return pLast;

}
unsigned int LcbList::count(){
	tLcb* p = pRoot;
	unsigned int cnt= 0;
	while (p ) {
		++cnt;
		p=p->pNext;
	}
	return cnt;;
}
void LcbList::deleteLcbList() {
	tLcb* pLastEntry;
	while ((pLastEntry = getLastLcbEntry()) != NULL) {
		delete pLastEntry;
	}
}
