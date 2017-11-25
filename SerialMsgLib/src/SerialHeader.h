/*
 * SerialHeader.h
 *
 *  Created on: 12.11.2017
 *      Author: User
 */

#ifndef SERIALHEADER_H_
#define SERIALHEADER_H_

#include "Arduino.h"


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


// internal use
typedef struct{
	byte fromAddr=0;
	byte toAddr=0;
	tAktId aktid=0;
	byte cmd=0;
	byte par=0;
} tSerialHeader;

#define SERIALHEADER_SIZE  sizeof(tSerialHeader);



#endif /* SERIALHEADER_H_ */
