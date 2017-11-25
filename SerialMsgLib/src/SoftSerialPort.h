/*
 * SoftSerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef SOFTSERIALPORT_H_
#define SOFTSERIALPORT_H_

#include "Arduino.h"
#include <SoftwareSerial.h>
#include "SerialMsg.h"
#include "SerialPort.h"



class SoftSerialPort: public SerialPort {
public:
	SoftSerialPort(byte pinRx, byte pinTx,byte id=0);
	SoftSerialPort(SoftwareSerial* pSoftwareSerial,byte id=0);
	virtual ~SoftSerialPort();

	virtual byte read();
	virtual bool write(byte b);
	virtual size_t write(byte* bb,size_t len);

	virtual void begin(long speed);
	virtual bool listen();
	virtual int  available();
	virtual bool isListening();

private:
	SoftwareSerial* pSoftwareSerial;
	bool deleteSoftwareSerial = false;

};

#endif /* SOFTSERIALPORT_H_ */
