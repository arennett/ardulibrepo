/*
 * WQDefines.h
 *
 *  Created on: 27.01.2019
 *      Author: User
 */

#ifndef WQDEFINES_H_
#define WQDEFINES_H_


typedef	union {
		uint16_t 	  cmd;
		union  {
			byte	  data[8];
			char	  str[8];
			uint8_t   unint8;
			uint16_t  unint16;
			uint32_t  unint32;
			uint64_t  unint64;
			int8_t    int8;
			int16_t   int16;
			int32_t   int32;
			int64_t   int64;
		};
		byte bytes[sizeof(cmd)+sizeof(data)];
	}
	tWQMessage;

#define WQ_MESSAGE_LENGTH sizeof(tWQMessage)

#endif /* WQDEFINES_H_ */
