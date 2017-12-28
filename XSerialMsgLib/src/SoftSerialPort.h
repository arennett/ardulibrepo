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

	static SoftSerialPort* pSoftSerialPortList;
	static SoftSerialPort*  getListenerPort();
	/*
	 * switch to next port if listen time expired
	 * and no data available
	 * only for software serial ports important
	 */
	static void cycleListenerPort();

	SoftSerialPort();
	SoftSerialPort(byte pinRx, byte pinTx,byte remoteSysId) ;
	SoftSerialPort(SoftwareSerial* pSoftwareSerial,byte remoteSysId);


	virtual ~SoftSerialPort();


	SoftSerialPort* cycleNextSoftSerialPort();

	virtual tPortType getType();
	virtual byte read();
	virtual bool write(byte b);
	virtual size_t write(const byte* bb,size_t len);

	virtual void begin(long speed);
	virtual bool listen();
	virtual int  available();
	virtual bool isListening();

private:
	SoftwareSerial* pSoftwareSerial;
	};

#endif /* SOFTSERIALPORT_H_ */
