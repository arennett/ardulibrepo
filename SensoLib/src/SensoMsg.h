/*
 * This is the declaration of the LedStatus class whose instance (object) will
 * receive and deserialize the incomming serial data.
 */

#ifndef _SENSOMESSAGE_H
#define	_SENSOMESSAGE_H
#include "Arduino.h"
#include <SerialMsgLib.h>
#include <SerialPort.h>
#include <tools.h>

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
	SensoMsg(bool send);

	virtual ~SensoMsg();

	void  receive(byte* pData ,size_t len) ;
	void send(SerialPort* pSerialPort);

	size_t getDataSize();

	void setDataSize(size_t size);

	tSensoMsgHdr* getMsgHdr();

	void setCmd(byte cmd);

	byte getCmd();

	void setSubCmd(byte subcmd);

	byte getSubCmd();

	int  getBitArray();

	void setBitArray(int bits);

	byte* getData();

	void setData(byte* pData);

	void setBit(byte num);

	bool isBitSet(byte num);

private:
	tSensoMsgHdr* pMsgHdr;

	bool deleteMsgHdr = false;

};

#endif	/* _LEDSTATUS_H */
