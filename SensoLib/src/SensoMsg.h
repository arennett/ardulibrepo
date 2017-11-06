/*
 * This is the declaration of the LedStatus class whose instance (object) will
 * receive and deserialize the incomming serial data.
 */

#ifndef _SENSOMESSAGE_H
#define	_SENSOMESSAGE_H
#include "Arduino.h"
#include "SerialMsg.h"
#include "SerialPort.h"

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
	byte* pData=NULL;
} tSensoMsgHdr;

class SensoMsg {
public:

	/**
	 * recdata   header, userdata
	 */
	SensoMsg(byte* recdata) {
		pMsgHdr=recdata;

	}

	SensoMsg(byte cmd, byte subcmd, void* pData, size_t datlen) {
		pMsgHdr=new tSensoMsgHdr;
		pMsgHdr->dataSize=datlen;
		pMsgHdr->pData = (byte*) pData;
	}

	SensoMsg(byte cmd, byte subcmd, void* pData, size_t datlen) {
		pMsgHdr=new tSensoMsgHdr;
		pMsgHdr->dataSize=datlen;
		pMsgHdr->pData = (byte*) pData;
	}




	void writeTo(SerialPort* pSerialPort) {
		pSerialPort->write(serPreamble,sizeof serPreamble);
		pSerialPort->write(p
		pSerialPort->write(pMsgHdr->pData,pMsgHdr->dataSize);
		pSerialPort->write(serPostamble,sizeof serPostamble);
		DPRINTLN("write data");

	}


	size_t getDataSize() {
		return pMsgHdr->dataSize;
	}

	tSensoMsgHdr getMsgHdr() {
		return pMsgHdr;
	}

	void setCmd(byte cmd) {
		this->pMsgHdr->cmd = cmd;
	}
	;
	byte getCmd() {
		return this->pMsgHdr->cmd;
	}
	;
	void setSubCmd(byte subcmd) {
		this->pMsgHdr->subcmd = subcmd;
	}
	;

	byte getSubCmd() {
		return this->pMsgHdr->subcmd;
	}

	int getBitArray() {
		return pMsgHdr->bitArray;
	}

	void setBit(byte num) {
		if (num < 16) {
			bitWrite(pMsgHdr->bitArray, num, 1);
		}
	}
	bool isBitSet(byte num) {
		if (num < 16) {
			return (pMsgHdr->bitArray & (1 << num));
		}
	}



private:
	tSensoMsgHdr* pMsgHdr;

};

#endif	/* _LEDSTATUS_H */
