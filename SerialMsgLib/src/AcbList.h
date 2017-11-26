/*
 * AcbList.h
 *
 *  Created on: 26.11.2017
 *      Author: User
 */

#include <stddef.h>
#include "SerialHeader.h"

#ifndef ACBLIST_H_
#define ACBLIST_H_

typedef struct {
	tAktId aktid;
	byte cmd;
	byte fromAddr;
	byte toAddr;
#define	ACB_STATUS_CREATED		0
#define ACB_STATUS_CLOSED		1
#define ACB_STATUS_OPEN			2
	byte status = ACB_STATUS_CREATED;
	unsigned int cntRetries = 0;
	void* pNext = NULL;
} tAcb;

class AcbList {
public:
	AcbList();
	tAcb* 	getRoot();
	tAcb* 	createAcb(tAktId aktId) ;
	tAcb* 	createOrUseAcb(byte cmd, byte fromAddr, byte toAddr, tAktId aktidTx);
	//void 	mprintAcb(tAcb* pAcb);
	tAcb*   getAcbEntry(tAktId aktId);
	tAcb* 	getLastAcbEntry();
	unsigned int getCountAcbEntries();
	void 	deleteAcbList();
	virtual ~AcbList();
	bool  	deleteAcbEntry(tAktId aktId);

	tAcb* pRoot = NULL;

};

#endif /* ACBLIST_H_ */
