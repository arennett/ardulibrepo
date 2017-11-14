/*
 * SerialHeader.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADER_H_
#define SERIALHEADER_H_

#include "Arduino.h"


#define SERIALHEADER_CMD_NAK	240	 // NOT ACKNOWLEDGED
#define SERIALHEADER_CMD_ACK	241	 // ACKNOWLEDGED
#define SERIALHEADER_CMD_INI	243	 // ACKNOWLEDGED
#define SERIALHEADER_CMD_CUR	244  // CONNECTION UP	  REQUEST
#define SERIALHEADER_CMD_CDR	245  // CONNECTION DOWN REQUEST
#define SERIALHEADER_CMD_DATA	246  // USER Message

typedef struct{
	byte addrFrom=0;
	byte addrTo=0;
	unsigned long aktid;
	byte cmd=0;
	byte par=0;
} tSerialHeader;



#endif /* SERIALHEADER_H_ */
