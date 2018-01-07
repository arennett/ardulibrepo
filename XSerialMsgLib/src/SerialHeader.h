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

#define XPRINTADDR(x)   XPRINT(x.sysId);XPRINTS(".");XPRINT(x.nodeId)
#define XPRINTLNADDR(x) XPRINTADDR(x);XPRINTLNS("");
#define XPRINTLNHEADER(pH) XPRINTADDR(pH->fromAddr);XPRINTS(" to ");XPRINTLNADDR(pH->toAddr);\
			XPRINTSVAL(" aktId: ",pH->aktid);XPRINTS(" cmd: ");XPRINTSS(tSerialHeader::cmd2Str(pH->cmd));XPRINTLNSVAL(" par: ",pH->par);


#ifdef MPRINT_ON
	#define MPRINTADDR(x)   XPRINTADDR(x)
	#define MPRINTLNADDR(x) XPRINTLNADDR(x)
	#define MPRINTLNHEADER(pH)  XPRINTLNHEADER(pH)

#else
	#define MPRINTADDR(x)
	#define MPRINTLNADDR(x)
	#define MPRINTLNHEADER(pH)
#endif



#ifdef DPRINT_ON
#define DPRINTADDR(x) 	MPRINTADDR(x)
#define DPRINTLNADDR(x) MPRINTLNADDR(x)
#define DPRINTLNHEADER(pH) MPRINTLNHEADER(pH)
#else
	#define DPRINTADDR(x)
	#define DPRINTLNADDR(x)
	#define DPRINTLNHEADER(pH)
#endif
#ifdef DPRINT_ON
#define DPRINTADDR(x) 	   MPRINTADDR(x)
#define DPRINTLNADDR(x)    MPRINTLNADDR(x)
#define DPRINTLNHEADER(pH) MPRINTLNHEADER(pH)
#else
	#define DPRINTADDR(x)
	#define DPRINTLNADDR(x)
	#define DPRINTLNHEADER(pH)
#endif


typedef enum {
	// application and internal use
	CMD_NULL=  0,
	CMD_NAK = 240,	//  < NOT ACKNOWLEDGED
	CMD_ACK,		// 	< ACKNOWLEDGED

	// internal use
	CMD_LIVE,		//	> LIVE (reply exp.)
	CMD_CR,			// 	> CONNECTION REQUEST
	CMD_CD, 		//	> CONNECTION DOWN

	// application use
	CMD_ACD,		//	> Application Command	(opt. data)
	CMD_ARQ,		//	> Application Request  	(opt. data)	(reply exp.)
	CMD_ARP		 	//	> Application Reply   	(opt. data)
} tSerialCmd;
typedef  unsigned int tAktId;
typedef  unsigned long tStamp;
#define MAX_AKTID  65535

const char STR_NAK[] PROGMEM = { "NAK" };
const char STR_ACK[] PROGMEM = { "ACK" };
const char STR_LIVE[] PROGMEM = { "LIVE" };
const char STR_CR[] PROGMEM = { "CR" };
const char STR_CD[] PROGMEM = { "CD" };
const char STR_ACD[] PROGMEM = { "ACD" };
const char STR_ARQ[] PROGMEM = { "ARQ" };
const char STR_ARP[] PROGMEM = { "ARP" };
const char STR_UNKOWN_CMD[] PROGMEM = { "UNKOWN_CMD" };



class tAddr {
public:
	tAddr() {
	}
	;
	~tAddr() {
	}
	;

	tAddr(byte sysid, byte nodeid) {
		sysId = sysid;
		nodeId = nodeid;
	}
	byte sysId = 0;
	byte nodeId = 0;

	bool operator==(const tAddr& right) const {
		return (sysId == right.sysId) && (nodeId == right.nodeId);
	}

	bool operator!=(const tAddr& right) const {
		return !operator==(right);
	}
};

//Connection Control Block
typedef struct {
	tAddr localAddr;
	tAddr remoteAddr;    //remote address
#define  CONNECTION_STATUS_NOT_READY  	1
#define  CONNECTION_STATUS_READY 		2
#define  CONNECTION_STATUS_DISCONNECTED 3
#define  CONNECTION_STATUS_CONNECTED 	4
	byte status = 0;bool active = false;
} tCcb;

class tSerialHeader {
public:
	tSerialHeader() {
	}
	;
	~tSerialHeader() {
	}
	;

	tAddr fromAddr;
	tAddr toAddr;
	tAktId aktid = 0;
	tSerialCmd cmd = CMD_NULL;
	byte par = 0;
	static const char* cmd2Str(tSerialCmd cmd) {
		switch (cmd) {
		case CMD_NAK:  return STR_NAK;
		case CMD_ACK:  return STR_ACK;
		case CMD_LIVE: return STR_LIVE;
		case CMD_CR:   return STR_CR;
		case CMD_ACD:  return STR_ACD;
		case CMD_ARQ:  return STR_ARQ;
		case CMD_ARP:  return STR_ARP;
		default:  return STR_UNKOWN_CMD;
		}
	}

	static bool isReplyExpected(tSerialCmd cmd) {
		switch (cmd) {
		case CMD_NAK:  return false;
		case CMD_ACK:  return false;
		case CMD_LIVE: return true;
		case CMD_CR:   return true;
		case CMD_ACD:  return false;
		case CMD_ARQ:  return true;
		case CMD_ARP:  return false;
		default:  return false;
		}
	}
	bool isReplyExpected() {
	  return tSerialHeader::isReplyExpected(cmd);
	}


};



#define SERIALHEADER_SIZE  sizeof(tSerialHeader);

#endif /* SERIALHEADER_H_ */
