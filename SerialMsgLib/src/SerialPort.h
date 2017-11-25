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
	/*
	 * static SerialPort* GetNextPort(SerialPort* port) ;
	 * return the port after port.
	 * at the end of the list, it return the first port
	 */
	static SerialPort* GetNextPort(SerialPort* port) ;
	static SerialPort* GetPortList() ;
	static SerialPort* pSerialPortList = NULL;
	SerialPort();
	virtual ~SerialPort();

	virtual byte read()=0;
	virtual bool write(byte b)=0;
	virtual size_t write(byte* bb, size_t len)=0;
	virtual void begin(long speed)=0;
	virtual bool listen()=0;
	virtual int  available()=0;
	virtual bool isListening()=0;
	void* pNext = NULL;
	byte id;
private:


};




#endif /* SERIALPORT_H_ */
