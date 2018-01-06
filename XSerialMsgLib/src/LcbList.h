/*
 * LcbList.h
 *
 *  Created on: 30.11.2017
 *      Author: rea
 */

#ifndef LCBLIST_H_
#define LCBLIST_H_
#include "SerialHeader.h"


namespace SerialMsgLib {
class SerialPort;
/* a link between two ports*/
struct tLcb {
	//aktid of CR and corresponding ACK
	tAktId   aktid;
	//addr  of CR
	tAddr     addrA;
	tAddr     addrB;
	SerialPort* pPortA=NULL;
	SerialPort* pPortB=NULL;
	tLcb* pNext=NULL;
};

class LcbList {
public:
	LcbList();
	virtual 	~LcbList();
	tLcb* 		getRoot();
	tLcb* 		createLcb(tSerialHeader* pHeader,SerialPort* fromPort) ;
	tLcb*   	getLinkedLcb(tSerialHeader* pHeader);
	SerialPort* getTargetPort(tSerialHeader* pHeader);
	tLcb* 		getOpenLcb(tSerialHeader* pHeader);
		//void 	mprintAcb(tAcb* pAcb);
	tLcb* 	getLastLcbEntry();
	unsigned int count();
	void 	deleteLcbList();
	tLcb* pRoot = NULL;
};
};
#endif /* LCBLIST_H_ */
