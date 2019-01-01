/*
 * OledMessage.h
 *
 *  Created on: 26.01.2018
 *      Author: User
 */
#include <Arduino.h>
#include <stddef.h>

#ifndef OLEDMESSAGE_H_
#define OLEDMESSAGE_H_
#define OLEDMESSAGE_SIZE 8

#define BLACK 0
#define WHITE 1
#define INVERSE 2

typedef enum {
	CMD_UPDATE = 1, CMD_CLEAR, CMD_LINE, CMD_RECTANGL, CMD_CIRCLE, CMD_BMP_START,CMD_BMP_GAME_SELECT,CMD_BMP_GAME_1,CMD_BMP_GAME_2,CMD_GAME_OVER
} tOledCmd;

typedef union p {
	struct {
		byte color;
		union {
			byte v;
			byte x1;
		} p0;
		union {
			byte v;
			byte y1;
		} p1;
		union {
			byte v;
			byte x2;
			byte r;
		} p2;
		union {
			byte v;
			byte y2;
		} p3;
	} a;
	byte *buff[5];

} tParams;

class OledMessage {
public:
	OledMessage();
	OledMessage(tOledCmd cmd, const byte* pParams = NULL,
			size_t paramsSize = 0);
	virtual ~OledMessage();
	byte buffer[8];

	void *pNext = NULL;
	byte getCmd() {
		return buffer[0];
	}

	byte getParamsSize() {
		return buffer[1];
	}

	byte* getParams() {
		return &buffer[2];
	}

	tParams* getTParams() {
		return (tParams*) &buffer[2];
	}

};

#endif /* OLEDMESSAGE_H_ */
