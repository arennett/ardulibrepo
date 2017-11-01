/*
 * SoftSerialRx.h
 *
 *  Created on: 30.10.2017
 *      Author: User
 */

#ifndef SOFTSERIALRX_H_
#define SOFTSERIALRX_H_
#include "SoftSerial.h"
#include "Arduino.h"
#include <SoftwareSerial.h>

//byte serPreamble[]  = {1,2,3,4};
//byte serPostamble[] = {4,3,2,1};

class SoftSerialRx {
public:
	SoftSerialRx(byte pinRx,byte pinTy,size_t maxDataSize);
	SoftSerialRx(SoftwareSerial* pSoftSerial, size_t maxDataSize);
	void setUpdateCallback(void (*ptr)(byte* data, size_t data_size));
	void begin(long speed);
	bool readNext();
	bool readNext(byte* b);
	bool waitOnMessage(byte* data, size_t& data_size, unsigned long timeout);
	bool listen ();
	SoftwareSerial* getSoftSerial();
	virtual ~SoftSerialRx();
private:
	void (*updateCallback)(byte* data, size_t data_size);
	byte* pRecBuffer;
	SoftwareSerial* pSoftSerial;
	bool deleteSoftSerial = false;
	byte preAmCount=0;
	byte postAmCount=0;
	byte dataCount=0;
	byte prevDataCount=0;
	bool dataCollect=false;
	size_t bufferSize=0;
	byte serPreamble[4]  = PREAMBLE;
	byte serPostamble[4] = POSTAMBLE;
	byte _byte ;

};

#endif /* SOFTSERIALRX_H_ */
