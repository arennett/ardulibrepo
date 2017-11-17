/*
 * SerialHeader.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADER_H_
#define SERIALHEADER_H_

#include "Arduino.h"


#define SERIALHEADER_CMD_NAK	240	 // < NOT ACKNOWLEDGED
#define SERIALHEADER_CMD_ACK	241	 // < ACKNOWLEDGED
#define SERIALHEADER_CMD_LIVE	243	 // > LIVE    			Response: ACK /NAK
#define SERIALHEADER_CMD_CR		244  // > CONNECTION REQUEST  Response: ACK /NAK
#define SERIALHEADER_CMD_DATA	246  // > USER DATA
#define SERIALHEADER_CMD_DREQ	247  // > DATA REQUEST        Response: DRES
#define SERIALHEADER_CMD_DRES	248  // < DATA RESPONSE



typedef unsigned int tAktId;
#define MAX_AKTID  65535


typedef struct{
	byte addrFrom=0;
	byte addrTo=0;
	tAktId aktid=0;
	byte cmd=0;
	byte par=0;
} tSerialHeader;



#endif /* SERIALHEADER_H_ */
