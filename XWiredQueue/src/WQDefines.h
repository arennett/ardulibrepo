/*
 * WQDefines.h
 *
 *  Created on: 27.01.2019
 *      Author: User
 */

#ifndef WQDEFINES_H_
#define WQDEFINES_H_
#include <Queue.h>

typedef	union {
	    union  {
			uint16_t  cmd;
			byte	  data[4];
			char	  str[4];
			uint32_t  uint32;
			int32_t   int32;
			union {
				uint16_t sub_cmd;
				void*    ptr;
			};
			union {
				int16_t u_int16;
				int16_t int16;
			};
	    };
		byte bytes[sizeof(cmd)+sizeof(data)];
	}
	tWQMessage;

#define WQ_MESSAGE_LENGTH sizeof(tWQMessage)

#endif /* WQDEFINES_H_ */
