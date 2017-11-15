/*
 * SerialHeaderTx.h
 *
 *  Created on: 14.11.2017
 *      Author: rea
 */

#ifndef SERIALHEADERTX_H_
#define SERIALHEADERTX_H_

#include "Arduino.h"
#include "SerialTx.h"



#define MAXACBS 10
typedef struct {
	unsigned long aktid;
	int  maxRetries; // -1  ... infintity
	bool confirmed;
	byte cmd;
	byte fromAddr;
	byte toAddr;
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
	void send(byte fromAddr,byte toAddr, byte cmd ,byte *pData,bool confirm=false );
	void send(byte fromAddr,byte toAddr, byte cmd ,unsigned long& aktid,byte *pData,bool confirm=false );
	bool sendAnswer(byte fromAddr,byte toAddr, byte cmd ,unsigned long onAktid,byte *pData,bool confirm=false);
	bool sendAndWait(byte fromAddr,byte toAddr, byte cmd ,byte *pData);
	bool isConfirmed(unsigned long aktid);
	bool sendCR(byte fromAddr,byte toAddr,unsigned long& aktid);
	bool sendACK(byte fromAddr,byte toAddr);
	bool sendNAK(byte fromAddr,byte toAddr);


	/* internal callback from SerialHeaderRx */
	void internalCallBack(const byte* pData, size_t data_size);



	virtual ~SerialHeaderTx();
private:
	SerialTx* pSerialTx;
	SerialHeaderRx* pSerialHeaderRx;
	unsigned long aktid = 2;
	tAcb  	acb[];
};

#endif /* SERIALHEADERTX_H_ */
