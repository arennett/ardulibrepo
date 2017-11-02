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

#include "src/SoftSerial.h"

class SoftSerialTx {
public:

	/**
	 * SoftSerialTx(byte pinRx,byte pinTx)
	 * > constructor for serial message transmitter
	 * > using an internal SoftSerial port
	 * pinRx	pin for receiving data
	 * pinRy	pin for transmitting data
	 */
	SoftSerialTx(byte pinRx,byte pinTx);

	/**
	 * SoftSerialTx(byte pinRx,byte pinTx)
	 * > constructor for serial message transmitter
	 * > using an external SoftSerial port
	 * pinRx	pin for receiving data
	 * pinRy	pin for transmitting data
	 */
	SoftSerialTx(SoftwareSerial* pSoftSerial);


	/**
	 * void begin(long speed)
	 * > like Serial.begin , inits serial connection
	 */
	void begin(long speed);


	/**
	 * void sendData(byte* data, size_t datasize);
	 * > like Serial.begin , inits serial connection
	 */
	void sendData(byte* data, size_t datasize);


	/**
	 * SoftwareSerial* getSoftSerial();
	 * <returns: pointer on internal SoftwareSerial connection
	 */
	SoftwareSerial* getSoftSerial();

	/**
	 * ~SoftSerialTx() {
	 * > Destructor
	 */
	virtual ~SoftSerialTx();
private:
	SoftwareSerial* pSoftSerial;
	bool deleteSoftSerial = false;
	byte serPreamble[4]  = PREAMBLE;
	byte serPostamble[4] = POSTAMBLE;
};

#endif /* SOFTSERIALTX_H_ */
