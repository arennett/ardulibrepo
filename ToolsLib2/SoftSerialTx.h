/*
 * SoftSerialTx.h
 *
 *  Created on: 31.10.2017
 *      Author: User
 */

#ifndef SOFTSERIALTX_H_
#define SOFTSERIALTX_H_
#include "SoftSerial.h"

class SoftSerialTx {
public:
	SoftSerialTx(byte pinRx,byte pinTy);

	void sendData(byte* data, size_t datasize);

	virtual ~SoftSerialTx();
private:
	SoftwareSerial* pSoftSerial;
	byte serPreamble[4]  = PREAMBLE;
	byte serPostamble[4] = POSTAMBLE;
};

#endif /* SOFTSERIALTX_H_ */
