/*
 * SerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include "Arduino.h"



typedef struct {
	//used from SerialRx
	byte* pBuffer;
	size_t bufferSize=0;
	byte preAmCount=0;
	byte postAmCount=0;
	byte dataCount=0;
	byte prevDataCount=0;
	bool dataCollect=false;
	byte lastByte=0;
} tSerialRxState;


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
	virtual size_t write(const byte* bb, size_t len);
	virtual void begin(long speed)=0;
	virtual bool listen()=0;
	virtual int  available()=0;
	virtual bool isListening()=0;
	void* pNext = NULL;
	byte remoteSysId=0;



	tSerialRxState serialRxState;

private:


};


//SerialPort* pSerialPortList = NULL;

#endif /* SERIALPORT_H_ */
