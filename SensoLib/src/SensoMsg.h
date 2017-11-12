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

typedef enum {
	CMD_NULL = 0,
	CMD_SET_LED,
	CMD_SET_NEO_PIXEL_CMD,
	CMD_SET_DSP,
	CMD_SET_DSP_CLEAR,
	CMD_SET_DSP_RECT,
	CMD_SET_DSP_CIRCLE
} tSensoCmd;

typedef struct SensoMsgHdr {
	unsigned long aktId=0;
	tSensoCmd cmd = CMD_NULL;
	byte par = 0;
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
	void send(SerialTx* pSerialTx);

	void mprint();

	size_t getDataSize();

	void setDataSize(size_t size);

	tSensoMsgHdr* getMsgHdr();

	void setCmd(byte cmd);

	byte getCmd();

	void setPar(byte par);

	byte getPar();

	int  getBitArray();

	void setBitArray(int bits);

	byte* getData();

	void setData(byte* pData);

	void setBit(byte num);

	bool isBitSet(byte num);

private:
	tSensoMsgHdr* pMsgHdr;
	bool sendType = false;
	bool deleteMsgHdr = false;
	unsigned long aktId=0;

};

#endif	/* _LEDSTATUS_H */
