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
#define ACB_REPLYTIME_EXPIRED_CHECK_PERIOD_MSEC 	200
#define ACB_REPLYTIME_EXPIRED_MSEC 	6000       // if we didnt hear anything , acb is deleted after 3 seconds

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
	tStamp timeStamp= 0;
	void* 	pNext = NULL;
} tAcb;


namespace SerialMsgLib {
class AcbList {
public:

	/*
	 * static AcbList* getInstance();
	 * creates the AcbList for the own SerialNodeNet (systemId) , if not created yet.
	 * < return		... the system AcbList for handling self created request messages.
	 */
	static AcbList* getInstance();

	static AcbList* getRootAcbList();


	/*
	 * static AcbList* getList(byte portId,bool create=false);
	 * create/returns AcbLists of forwarded request messages
	 * > portId, of the port who send the request
	 * > create a new list if not found
	 */
	static AcbList* getList(byte portId,bool create=false);

	/*
	 * static void removeOldAcbs();
	 * removes old unreplied acbs
	 */
	static void removeOldAcbs();
	static tStamp lastAcbCheck;

	AcbList(byte sysId);

	virtual ~AcbList();
	tAcb* 	getRoot();
	unsigned int getNextAktId();
	tAcb* 	createAcb(tSerialHeader* pHeader) ;
	tAcb* 	createOrUseAcb(tSerialHeader* pHeader);
	//void 	mprintAcb(tAcb* pAcb);
	tAcb*   getAcbEntry(tAktId aktId);

	/**
	 * getLastestAcbEntry(byte portId=0);
	 * > portId 	...port, default all ports
	 * > returns 	...the latest acb entry
	 */
	tAcb*   getLastestAcbEntry(byte portId=0);

	/*
	 * tAcb*   getAcbEntry(tCcb* pCcb,byte cmd);
	 * get the acb with a specific cmd for a given connection
	 * i.e. an acb by connection request is reused until connection
	 * or timeout.
	 * > pCcb 	connection of a node
	 * > cmd	message cmd
	 */
	tAcb*   getAcbEntry(tCcb* pCcb,byte cmd);

	/* tAcb* 	getLastAcbEntry();
	 * return the last created acb in list
	 */
	tAcb* 	getLastAcbEntry();

	/**
	 * unsigned int count(byte portId=0);
	 * count open acbs
	 * > portId 	== 0	...all acbs
	 * 				> 0 	...acbs on a port
	 */
	unsigned int count(byte portId=0);
	static unsigned int countAll(byte portId=0);

	void 	deleteAcbList();
	bool  	deleteAcbEntry(tAktId aktId);
	bool  	deleteAcbEntry(tCcb* pCcb,byte cmd);
	void    setAktId(tAktId aktId);
	byte 	getId();
	void	printList();
	void*	pNext=NULL;


private:
	unsigned int aktId;
	byte sysId;
	tAcb* pRoot = NULL;
	static AcbList* pAcbListList;
};
};
#endif /* ACBLIST_H_ */
