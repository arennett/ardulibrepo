/*
 * SerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#define MAX_LISTENTIME 400 // only by multiple SoftSerialPorts important

#include "Arduino.h"
#include "SerialPortRxTxMapper.h"


class SerialRx;
class SerialTx;

class SerialPort {
public:
	static SerialPort* pSerialPortList;
	static SerialPort* getPort(byte remoteSysId);

	/*
	 * void readNextOnAllPorts() ;
	 * for all instantiated SerialPorts
	 * read next byte into the port buffer
	 */
	static void readNextOnAllPorts() ;


	/*
	 * switch to next port if listen time expired
	 * and no data available
	 * only for software serial ports important
	 */
	static void cycleListenerPort();


	/**
	 * SerialPort(byte remoteSysId);
	 * the SerialPort get the ID of the remote system
	 * and is linked into a port list
	 */
	SerialPort(byte remoteSysId);

	virtual ~SerialPort();

	SerialPort* cycleNextPort();


	/*
	 *  createDataBuffer(size_t dataSize);
	 *  creates (recreates) the data buffer for receiving messages
	 *  > datasize	...	size for optional data (discount the header)
	 */
	void    createDataBuffer(size_t dataSize);

	/*
	 * SerialRx* getRx();
	 * < returns	...the receiver for this port
	 */
	SerialRx* getRx();

	/*
	 * SerialRx* getTx();
	 * < returns	...the transmitter for this port
	 */

	SerialTx* getTx();



	virtual byte read()=0;
	virtual bool write(byte b)=0;
	virtual size_t write(const byte* bb, size_t len)=0;
	virtual void begin(long speed)=0;
	virtual bool listen()=0;
	virtual int  available()=0;
	virtual bool isListening()=0;
	void* pNext = NULL;
	byte remoteSysId=0;


	SerialPortRxTxMapper* pPortRxTxMapper =NULL;

	unsigned long listenTimeStamp = 0;


private:
	SerialPort();

};

//SerialPort* pSerialPortList = NULL;

#endif /* SERIALPORT_H_ */
