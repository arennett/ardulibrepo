/*
 * SoftSerialTx.h
 *
 *  Created on: 31.10.2017
 *      Author: User
 */

#ifndef SOFTSERIALTX_H_
#define SOFTSERIALTX_H_
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "SoftSerial.h"

class SoftSerialTx {
public:
	SoftSerialTx(byte pinRx,byte pinTx);
	SoftSerialTx(SoftwareSerial* pSoftSerial);

	void begin(long speed);
	void sendData(byte* data, size_t datasize);
	SoftwareSerial* getSoftSerial();

	virtual ~SoftSerialTx();
private:
	SoftwareSerial* pSoftSerial;
	bool deleteSoftSerial = false;
	byte serPreamble[4]  = PREAMBLE;
	byte serPostamble[4] = POSTAMBLE;
};

#endif /* SOFTSERIALTX_H_ */
