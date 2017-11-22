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
#define SERIALHEADER_CMD_LIVE	243	 // > LIVE    			  Rx: ACK / NAK
#define SERIALHEADER_CMD_CR		244  // > CONNECTION REQUEST  Rx: ACK / NAK
#define SERIALHEADER_CMD_CD		245  // > CONNECTION DOWN
#define SERIALHEADER_CMD_DATA	246  // > USER DATA			  Rx: ACK /NAK
#define SERIALHEADER_CMD_DREQ	247  // > DATA REQUEST        Rx: DREP / DRAQ / ACK
#define SERIALHEADER_CMD_DREP	248  // < DATA RESPONSE


typedef unsigned int tAktId;
#define MAX_AKTID  65535


typedef struct{
	byte fromAddr=0;
	byte toAddr=0;
	tAktId aktid=0;
	byte cmd=0;
	byte par=0;
} tSerialHeader;



#endif /* SERIALHEADER_H_ */
