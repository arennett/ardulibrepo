/*
 * This is the declaration of the LedStatus class whose instance (object) will
 * receive and deserialize the incomming serial data.
 */

#ifndef _SENSOMESSAGE_H
#define	_SENSOMESSAGE_H
#include "Arduino.h"

enum {
	CMD_NULL = 0, CMD_SET_LED, CMD_SET_NEO_PIXEL_CMD, CMD_SET_DSP
} tSensoCmd;
enum {
	CMDSUB_NULL = 0, CMDSUB_SET_DSP_CLEAR, CMDSUB_SET_DSP_RECT, CMDSUB_DSP_WRITE
} tSensoSubCmd;

typedef struct SensoMsgHdr {
	byte cmd = CMD_NULL;
	byte subcmd = CMDSUB_NULL;
	int bitArray = 0;
	size_t dataSize; // if more data follows
} tSensoMsgHdr;

class SensoMsg {
public:
	size_t getDataSize() {
		return msgHdr.dataSize;
	}

	tSensoMsgHdr getMsgHdr() {
		return msgHdr;
	}

	void setCmd(byte cmd) {
		this->msgHdr.cmd = cmd;
	}
	;
	byte getCmd() {
		return this->msgHdr.cmd;
	}
	;
	void setSubCmd(byte subcmd) {
		this->msgHdr.subcmd = subcmd;
	}
	;

	byte getSubCmd() {
		return this->msgHdr.subcmd;
	}

	int getBitArray() {
		return msgHdr.bitArray;
	}

	void setBit(byte num) {
		if (num < 16) {
			bitWrite(msgHdr.bitArray, num, 1);
		}
	}
	bool isBitSet(byte num) {
		if (num < 16) {
			return (msgHdr.bitArray & (1 << num));
		}
	}

private:
	tSensoMsgHdr msgHdr;

};

#endif	/* _LEDSTATUS_H */
