/*
 * SoftSerialTx.h
 *
 *  Created on: 31.10.2017
 *      Author: User
 */

#ifndef SERIALTX_H_
#define SERIALTX_H_
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "SerialMsg.h"
#include "SerialPort.h"

class SerialTx {
public:


	/**
	 * SoftSerialTx(byte pinRx,byte pinTx)
	 * > constructor for serial message transmitter
	 * > using an external SerialPort port

	 */
	SerialTx(SerialPort* pSerialPort);
	/**
	 * ~SoftSerialTx() {
	 * > Destructor
	 */
	virtual ~SerialTx();


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
	 * <returns: pointer on internal serial port
	 */
	SerialPort* getSerialPort();


private:
	SerialPort* pSerialPort;
	//byte serPreamble[4]  = PREAMBLE;
	//byte serPostamble[4] = POSTAMBLE;
};

#endif /* SERIALTX_H_ */
