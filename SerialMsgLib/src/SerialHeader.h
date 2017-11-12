/*
 * SerialHeader.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADER_H_
#define SERIALHEADER_H_

#include "Arduino.h"


typedef struct{
	byte addrFrom=0;
	byte addrTo=0;
	byte aktid=0;
	byte cmd=0;
	byte par=0;
} tSerialHeader;



#endif /* SERIALHEADER_H_ */
