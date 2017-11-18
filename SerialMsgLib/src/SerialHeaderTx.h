/*
 * SerialHeaderTx.h
 *
 *  Created on: 14.11.2017
 *      Author: rea
 */

#ifndef SERIALHEADERTX_H_
#define SERIALHEADERTX_H_

#include "Arduino.h"
#include "SerialHeader.h"
#include "SerialTx.h"

#define MAXACBS 10
typedef struct {
	tAktId aktid;
	byte cmd;
	byte fromAddr;
	byte toAddr;
#define	ACB_STATUS_CREATED		0
#define	ACB_STATUS_WAIT_FOR_TX  1
#define ACB_STATUS_WAIT_FOR_RX	2
#define ACB_STATUS_CLOSED		3
#define ACB_STATUS_CONNECTED	4

	byte status = ACB_STATUS_CREATED;
	unsigned int cntRetries = 0;
	unsigned long timeStamp = 0;
	void* pNext = NULL;
} tAcb;

class SerialHeaderRx;

class SerialHeaderTx {
public:

	/**
	 * SerialHeaderTx(SerialPort* serialPort);
	 * only one direction , no confirmations, answers
	 * > serialPort	Serial Port
	 */
	SerialHeaderTx(SerialPort* serialPort);

	/**
	 * SerialHeaderTx(SerialHeaderRx* pSerialHeaderRx);
	 * two directions,confirmations answers possible
	 * > pSerialHeaderRx	Receiver
	 */
	SerialHeaderTx(SerialHeaderRx* pSerialHeaderRx);

	/**
	 *   send(byte fromAddr,byte toAddr, byte cmd ,byte *pData,bool confirm=false );
	 * - sends a byte array to a the serialport
	 *   SerialHeaderTx adds a preamble,postamble and a serialHeader
	 *   If confirm is true , a generated aktid is sent and saved
	 *   If the answer receives , a callback method will called
	 *
	 * > pData address of the the byte array
	 * > size of data
	 */
	tAktId send(byte fromAddr, byte toAddr, byte cmd, byte *pData,
			size_t dataSize, tAktId aktId = 0);
	void reply(byte cmd, tAktId onAktid, byte *pData, size_t dataSize);bool sendAndWait(
			byte fromAddr, byte toAddr, byte cmd, byte *pData, size_t dataSize,
			tAktId aktId = 0, unsigned long int timeout);
	tAktId sendCR(byte fromAddr, byte toAddr);
	void replyACK(tAktId onAktId);
	void replyNAK(tAktId onAktId);

	/* internal callback from SerialHeaderRx */
	void internalCallBack(const byte* pData, size_t data_size);

	virtual ~SerialHeaderTx();
private:
	tAcb* createAcb(tAktId aktid);
	tAcb* createOrUseAcb(byte cmd, byte fromAddr, byte toAddr, tAktId aktidTx);
	void mprintAcb(tAcb* pAcb);
	void mprintAcbList();
	tAcb* getLastAcbEntry();
	tAcb* getAcbEntry(tAktId aktid);
	unsigned int getCountAcbEntries();bool deleteAcbEntry(tAktId aktid);
	void deleteAcbList();

	SerialTx* pSerialTx;
	SerialHeaderRx* pSerialHeaderRx;
	tSerialHeader sHeader;
	tAktId aktidTx = 0;
	tAcb* pAcbList;
};

#endif /* SERIALHEADERTX_H_ */
