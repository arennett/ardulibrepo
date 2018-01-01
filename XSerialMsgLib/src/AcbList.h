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
	byte    portId = 0;
	unsigned long timeStamp= 0;
	void* 	pNext = NULL;
} tAcb;



class AcbList {
public:

	//static
	static AcbList instance;

	AcbList();

	virtual ~AcbList();
	tAcb* 	getRoot();
	unsigned int getNextAktId();
	tAcb* 	createAcb(tSerialHeader* pHeader) ;
	tAcb* 	createOrUseAcb(tSerialHeader* pHeader);
	//void 	mprintAcb(tAcb* pAcb);
	tAcb*   getAcbEntry(tAktId aktId);
	tAcb*   getLastestAcbEntry(byte portId);

	/*
	 * tAcb*   getAcbEntry(tCcb* pCcb,byte cmd);
	 * get the acb with a specific cmd for a given connection
	 * i.e. an acb by connection request is reused until connection
	 * or timeout.
	 * > pCcb 	connection of a node
	 * > cmd	message cmd
	 */
	tAcb*   getAcbEntry(tCcb* pCcb,byte cmd);
	tAcb* 	getLastAcbEntry();

	/**
	 * unsigned int count(byte portId=0);
	 * count open acbs
	 * > portId 	== 0	...all acbs
	 * 				> 0 	...acbs on a port
	 */
	unsigned int count(byte portId=0);
	void 	deleteAcbList();
	bool  	deleteAcbEntry(tAktId aktId);
	bool  	deleteAcbEntry(tCcb* pCcb,byte cmd);
	void	printList();



private:
	unsigned int aktId;
	tAcb* pRoot = NULL;
};

#endif /* ACBLIST_H_ */
