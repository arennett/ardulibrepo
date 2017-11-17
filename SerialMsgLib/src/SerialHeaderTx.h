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
	int  maxRetries; // -1  ... infintity
	bool confirmed;
	byte cmd;
	byte fromAddr;
	byte toAddr;
	void* pNext =NULL;
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
	tAktId send(byte fromAddr,byte toAddr, byte cmd ,byte *pData);
	bool  reply(byte fromAddr,byte toAddr, byte cmd ,tAktId onAktid,byte *pData);
	bool sendAndWait(byte fromAddr,byte toAddr, byte cmd ,byte *pData);
	bool isConfirmed(tAktId aktid);
	tAktId sendCR(byte fromAddr,byte toAddr);
	tAktId sendACK(byte fromAddr,byte toAddr);
	tAktId sendNAK(byte fromAddr,byte toAddr);


	/* internal callback from SerialHeaderRx */
	void internalCallBack(const byte* pData, size_t data_size);



	virtual ~SerialHeaderTx();
private:
	tAcb* createAcb(tAktId aktid);
	tAcb* getLastAcbEntry();
	tAcb* getAcbEntry(tAktId aktid);
	void  deleteAcbEntry(tAktId aktid);
	tAcb* deleteAcbList();

	SerialTx* pSerialTx;
	SerialHeaderRx* pSerialHeaderRx;
	tSerialHeader sHeader;
	tAktId aktidTx = 0;
	tAcb*  pAcbList;
};

#endif /* SERIALHEADERTX_H_ */
