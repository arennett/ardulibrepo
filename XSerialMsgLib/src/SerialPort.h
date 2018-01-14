/*
 * SerialPort.h
 *
 *  Created on: 04.11.2017
 *      Author: User
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_



#include "Arduino.h"
#include "SerialHeader.h"
#include "SerialPortRxTxMapper.h"

namespace SerialMsgLib {
class SerialRx;
class SerialTx;

enum tPortType{
PORTTYPE_MOCK		=0,
PORTTYPE_SOFTSERIAL	=1,
PORTTYPE_HARDSERIAL	=2,
};

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


	/**
	 * SerialPort(byte remoteSysId);
	 * the SerialPort get the ID of the remote system
	 * and is linked into a port list
	 */
	SerialPort(byte remoteSysId);

	virtual ~SerialPort();


	byte getId();
	SerialPort* getNext();


	/*
	 *  createDataBuffer(size_t dataSize);
	 *  creates (recreates) the data buffer for receiving messages
	 *  > datasize	...	size for optional data (discount the header)
	 */
	void    createDataBuffer(size_t dataSize);

	void sendMessage(tSerialHeader* pHeader,const byte* data, size_t datsize);
	void sendMessage(const byte* message, size_t messagesize);


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




	virtual tPortType getType()=0;
	virtual byte read()=0;
	virtual bool write(byte b)=0;
	virtual size_t write(const byte* bb, size_t len)=0;
	virtual void begin(long speed)=0;
	virtual bool listen()=0;
	virtual int  available()=0;
	virtual bool isListening()=0;


	SerialPortRxTxMapper* pPortRxTxMapper =NULL;

	tStamp listenTimeStamp = 0;


private:
	void* pNext = NULL;
	byte portType = 0;
	byte remoteSysId=0;

	SerialPort();

};
};
//SerialPort* pSerialPortList = NULL;

#endif /* SERIALPORT_H_ */
