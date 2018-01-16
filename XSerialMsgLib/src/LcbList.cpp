/*
 * LcbList.cpp
 *
 *  Created on: 30.11.2017
 *      Author: rea
 */

#include <tools.h>
#include "LcbList.h"

namespace SerialMsgLib {
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
	pLcb->sysIdA = pHeader->fromAddr.sysId;
	pLcb->pPortA = pFromPort;
	pLcb->aktid = pHeader->aktid;
	MPRINTLNSVAL("cbList::createLcb> new LCB created, count :",count());
	return pLcb;
}

tLcb* LcbList::getLinkedLcb(tSerialHeader* pHeader) {
	tLcb* pLcb = pRoot;
	while (pLcb) {
		if (pLcb->pPortA && pLcb->pPortB
				&& (pLcb->sysIdA == pHeader->fromAddr.sysId
						|| pLcb->sysIdA == pHeader->toAddr.sysId)
				&& (pLcb->sysIdB == pHeader->fromAddr.sysId
						|| pLcb->sysIdB == pHeader->toAddr.sysId)) {	// link found
			return pLcb;
		}
		pLcb = pLcb->pNext;
	}
	return NULL;
}

SerialPort* LcbList::getTargetPort(tSerialHeader* pHeader) {
	tLcb* pLcb = getLinkedLcb(pHeader);
	if (pLcb) {
		if (pLcb->sysIdA == pHeader->fromAddr.sysId
				&& pLcb->sysIdB == pHeader->toAddr.sysId) {
			return pLcb->pPortB;
		} else if (pLcb->sysIdB == pHeader->fromAddr.sysId
				&& pLcb->sysIdA == pHeader->toAddr.sysId) {
			return pLcb->pPortA;
		} else {
			DPRINTLNS("LcbList::getTargetPort ERROR");
		}
		return NULL;
	}
	return NULL;
}

tLcb* LcbList::getOpenLcb(tSerialHeader* pHeader) {
	tLcb* pLcb = pRoot;
	while (pLcb) {

		DPRINTLNS("LcbList::getOpenLcb> [START] ");
		DPRINTLNHEADER(pHeader);
		DPRINTS("pLcb addrA");
		DPRINTLNADDR(pLcb->sysIdA);
		DPRINTS("pLcb addrB");
		DPRINTLNADDR(pLcb->sysIdB);


		if ( (pLcb->pPortA && !(pLcb->pPortB) )
				&& ( (pLcb->sysIdA == pHeader->fromAddr.sysId   && pHeader->cmd == CMD_CR)
					 || ( pLcb->sysIdA == pHeader->toAddr.sysId	&& pHeader->cmd == CMD_ACK
						  //&& (pLcb->aktid == pHeader->aktid)
					 	)
					)
			) {
			return pLcb;
		}

		pLcb = pLcb->pNext;
	}
	return NULL;
}

//void 	DPRINTAcb(tAcb* pAcb){}
tLcb* LcbList::getLastLcbEntry() {
	tLcb* pLast = pRoot;
	while (pLast && pLast->pNext) {
		pLast = pLast->pNext;
	}
	return pLast;

}
unsigned int LcbList::count() {
	tLcb* p = pRoot;
	unsigned int cnt = 0;
	while (p) {
		++cnt;
		p = p->pNext;
	}
	return cnt;;
}

unsigned int LcbList::countCompleted() {
	tLcb* p = pRoot;
	unsigned int cnt = 0;
	while (p) {
		if(p->pPortB) {
			++cnt;
		}
		p = p->pNext;
	}
	return cnt;;
}

void LcbList::deleteLcbList() {
	tLcb* pLastEntry;
	while ((pLastEntry = getLastLcbEntry()) != NULL) {
		delete pLastEntry;
	}
}
}
;
