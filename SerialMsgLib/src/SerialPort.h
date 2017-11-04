/*
 * SerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include "Arduino.h"

class SerialPort {
public:
	SerialPort();
	virtual ~SerialPort();

	virtual byte read();
	virtual bool write(byte b);
	virtual size_t write(byte* bb, size_t len);
	virtual void begin(long speed);
	virtual bool listen();
	virtual int  available();
	virtual bool isListening();


};

#endif /* SERIALPORT_H_ */
