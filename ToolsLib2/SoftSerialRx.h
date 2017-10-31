/*
 * SoftSerialRx.h
 *
 *  Created on: 30.10.2017
 *      Author: User
 */

#ifndef SOFTSERIALRX_H_
#define SOFTSERIALRX_H_
#include "Arduino.h"
#include <SoftwareSerial.h>

//byte serPreamble[]  = {1,2,3,4};
//byte serPostamble[] = {4,3,2,1};

class SoftSerialRx {
public:
	SoftSerialRx(byte pinRx,byte pinTy,size_t bufferSize);
	void setUpdateCallback(void (*ptr)(byte* data, size_t data_size));

	void begin(long speed);

	void readNext();
	bool listen ();
	SoftwareSerial* getSoftSerial();
	virtual ~SoftSerialRx();
private:
	void (*updateCallback)(byte* data, size_t data_size);
	byte* pRecBuffer;
	SoftwareSerial* pSoftSerial;
	byte preAmCount=0;
	byte postAmCount=0;
	byte dataCount=0;;
	bool dataCollect=false;
	size_t bufferSize=0;
	byte serPreamble[4]  = {1,2,3,4};
	byte serPostamble[4] = {4,3,2,1};
};

#endif /* SOFTSERIALRX_H_ */
