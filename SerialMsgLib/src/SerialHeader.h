/*
 * SerialHeader.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADER_H_
#define SERIALHEADER_H_
#include "Arduino.h"
#include <tools.h>

#define PRINTADDR(x) MPRINT(x.sysId);MPRINTS(".");MPRINT(x.nodeId)
#define PRINTLNADDR(x) PRINTADDR(x);MPRINTLNS("");

#define PRINTLNHEADER(pH) PRINTADDR(pH->fromAddr);MPRINTS(" to ");PRINTLNADDR(pH->toAddr);\
						MPRINTSVAL(" aktId: ",pH->aktid);MPRINTSVAL(" cmd: ",pH->cmd);MPRINTLNSVAL(" par: ",pH->par);



typedef enum {
				// application and internal use
				CMD_NAK	=240,	//  < NOT ACKNOWLEDGED
				CMD_ACK,		// 	< ACKNOWLEDGED

				// internal use
				CMD_LIVE,		//	> LIVE
				CMD_CR,			// 	> CONNECTION REQUEST
				CMD_CD, 		//	> CONNECTION DOWN
				CMD_AFA,			//	> ASK FOR ADDRESS


				// application use
				CMD_ACD,		//	> Application Command	(opt. data)
				CMD_ARQ,		//	> Application Request  	(opt. data)
				CMD_ARP		 	//	> Application Reply   	(opt. data)
} tSerialCmd;
typedef unsigned int tAktId;
#define MAX_AKTID  65535



class tAddr {
public:
		tAddr(){};
		~tAddr(){};

		tAddr (byte sysid,byte nodeid) {
			sysId =sysid;
			nodeId=nodeid;
		}
		byte  sysId=0;
		byte  nodeId=0;

		bool operator==(const tAddr& right) const{
			return (sysId == right.sysId) && (nodeId == right.nodeId);
		}

		bool operator!=(const tAddr& right)const{
			return !operator==(right);
		}
} ;

//Connection Control Block
typedef struct {
	tAddr localAddr;
	tAddr remoteAddr;    //remote address
	#define  CONNECTION_STATUS_NOT_READY  	1
	#define  CONNECTION_STATUS_READY 		2
	#define  CONNECTION_STATUS_DISCONNECTED 3
	#define  CONNECTION_STATUS_CONNECTED 	4
	byte status=0;
	bool active=false;
} tCcb;

typedef struct{
	tAddr fromAddr;
	tAddr toAddr;
	tAktId aktid=0;
	byte cmd=0;
	byte par=0;
} tSerialHeader;

#define SERIALHEADER_SIZE  sizeof(tSerialHeader);



#endif /* SERIALHEADER_H_ */
