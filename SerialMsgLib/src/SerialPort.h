/*
 * SerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include "Arduino.h"
class SerialRx;

class SerialPort {
public:
	static SerialPort* pSerialPortList;
	static SerialPort* getPort(byte remoteSysId);

	SerialPort();

	/**
	 * SerialPort(byte remoteSysId);
	 * the SerialPort get the ID of the remote system
	 * and is linked into a port list
	 */
	SerialPort(byte remoteSysId);

	virtual ~SerialPort();
	void    createBuffer(size_t size);


	virtual byte read()=0;
	virtual bool write(byte b)=0;
	virtual size_t write(const byte* bb, size_t len)=0;
	virtual void begin(long speed)=0;
	virtual bool listen()=0;
	virtual int  available()=0;
	virtual bool isListening()=0;
	void* pNext = NULL;
	byte remoteSysId=0;
	byte* pBuffer=NULL;
	size_t bufferSize=0;
	SerialRx* pSerialRx=NULL;

private:


};


//SerialPort* pSerialPortList = NULL;

#endif /* SERIALPORT_H_ */
