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
	tAktId 	aktid;
	byte 	cmd;
	tAddr 	fromAddr;
	tAddr 	toAddr;
#define	ACB_STATUS_CREATED		0
#define ACB_STATUS_CLOSED		1
#define ACB_STATUS_OPEN			2
	byte 	status = ACB_STATUS_CREATED;
	unsigned int cntRetries = 0;
	void* 	pNext = NULL;
} tAcb;



class AcbList {
public:
	AcbList();
	virtual ~AcbList();
	tAcb* 	getRoot();
	tAcb* 	createAcb(tAktId aktId) ;
	tAcb* 	createOrUseAcb(tSerialHeader* pHeader);
	//void 	mprintAcb(tAcb* pAcb);
	tAcb*   getAcbEntry(tAktId aktId);
	tAcb* 	getLastAcbEntry();
	unsigned int count();
	void 	deleteAcbList();

	bool  	deleteAcbEntry(tAktId aktId);

	tAcb* pRoot = NULL;

};

#endif /* ACBLIST_H_ */
