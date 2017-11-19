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
#define ACB_STATUS_CLOSED		1
#define ACB_STATUS_OPEN			2
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
	void reply(byte cmd, tAktId onAktid, byte *pData, size_t dataSize);
	bool sendAndWait(byte fromAddr, byte toAddr, byte cmd, byte *pData, size_t dataSize,
					 unsigned long int timeout,tAktId aktId = 0);
	tAktId sendCR(byte fromAddr, byte toAddr);
	void replyACK(tAktId onAktId);
	void replyNAK(tAktId onAktId);

	/* internal callback from SerialHeaderRx */
	void internalReceive(const byte* pData, size_t data_size);
	virtual ~SerialHeaderTx();

	/*
	 * void connect(byte fromAddr, byte toAddr,bool master);
	 * adds a connection (tCcb) to the tx
	 * > if master
	 */
	void addConnection(byte fromAddr, byte toAddr,bool master);
	bool   connect(unsigned long timeout=WAITED_READ_TIMEOUT_DEFAULT_MSEC, unsigned long reqPeriod=100);
	SerialHeaderRx* pSerialHeaderRx;

protected:
	void mprintAcbList();
	tAcb* getAcbEntry(tAktId aktid);
	bool deleteAcbEntry(tAktId aktid);

private:
	tAcb* createAcb(tAktId aktid);
	tAcb* createOrUseAcb(byte cmd, byte fromAddr, byte toAddr, tAktId aktidTx);
	void mprintAcb(tAcb* pAcb);

	tAcb* getLastAcbEntry();
	unsigned int getCountAcbEntries();
	void deleteAcbList();

	SerialTx* pSerialTx;

	tSerialHeader sHeader;
	tAktId aktidTx = 0;
	tAcb* pAcbList =NULL;
};

#endif /* SERIALHEADERTX_H_ */
