/*
 * SoftSerialRx.h
 *
 *  Created on: 30.10.2017
 *      Author: Andre Rennett
 *
 *     a message receiver for a software serial connection
 */

#ifndef SERIALRX_H_
#define SERIALRX_H_
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "SerialMsg.h"
#include "SerialPort.h"
namespace SerialMsgLib {

class SerialRx {
public:

	/*
	 *  SerialRx()
	 *  receiver without buffer
	 *  use createBuffer to create a receivebuffer a buffer
	 */
	SerialRx();


	/**
	 * SerialRx(size_t maxDataSize);
	 * - constructor
	 * > maxDataSize 	...data size, to create an internal buffer (dataSize + postamble size)
	 */
	SerialRx(size_t maxDataSize);

	SerialRx(SerialPort* pSerialPort,size_t maxDataSize/*,void (*ptr)(const byte* data, size_t data_size,SerialPort* pPort)*/) ;


	/*
	 * *byte getBuffer();
	 * < pointer on receive buffer
	 */
	byte* getBuffer();

	/*
	 * size_t getBufferSize();
	 * < return size of receive buffer
	 */
	size_t getBufferSize();

	/**
	 * void createBuffer(size_t maxDatasize);
	 * - existing buffer is deleted
	 * - new buffer is created
	 * > maxDatasize	...size of longest expected message
	 */
	void createBuffer(size_t maxDatasize);


	/**
	 * void setPort(SerialPort* pSerialPort);
	 * - Set the the port for for listen
	 * > returns true if another port was deactivated
	 */
	bool setPort(SerialPort* pSerialPort);


//	/**
//	 * void setUpdateCallback(void (*ptr)(byte* data, size_t data_size));
//	 * > registering a static callback method
//	 * > method will be called when data is completely received
//	 * > see also readNext()
//	 * pData 		...pointer on the received data
//	 * data_size 	...what you think ? ;-)
//	 * pSerialPort  ...the port from which the message was received
//	 */
//	void setUpdateCallback(void (*ptr)(const byte* pData, size_t data_size,SerialPort* pPort));




	/**
	 * readNext();
	 * > reads next byte into the buffer
	 * < return : true when data complete and callBack was called
	 */
	bool readNext();






	/**
	 * bool waitOnMessage(byte* data, size_t& data_size, tStamp timeout);
	 * > waits until complete message is received or timeout is expired
	 * ppData 		...reference for : pointer on the received data
	 * data_size 	...reference data size
	 * timeout		...timeout msecs
	 * checkPeriod  ...time until next read trial is done
	 *
	 */
	bool waitOnMessage(byte*& pData, size_t& data_size, tStamp timeout, tStamp checkPeriod);

	/**
		 * bool waitOnMessage(byte* data, size_t& data_size, tStamp timeout);
		 * > waits until complete message is received or timeout is expired
		 * > the checkPeriod is 10msec
		 * ppData 		...reference for : pointer on the received data
		 * data_size 	...reference data size
		 * timeout		...timeout msecs
		 *
		 *
		 */
	bool waitOnMessage(byte*& pData, size_t& data_size, tStamp timeout);

	/**
	 * bool listen ();
	 * >if multiple software serials are used, listen
	 * >activate this software serial connection
	 * >since they are concurrent
	 * <returns: true ...if other connection was deactivated
	 */
	bool listen ();

	/**
	 *  SerialPort* getPort()
	 * <returns: pointer on port which
	 * was set , see setPort();
	 */
	 SerialPort* getPort();

	/**
	 * ~SoftSerialRx();
	 * > destructor deletes internal softSerial
	 * > if constructor with softSerial was used, it does not
	 */
	virtual ~SerialRx();
private:
	//void (*updateCallback)(const byte* data, size_t data_size,SerialPort* pPort);
	byte* pRecBuffer=NULL;
	SerialPort* pSerialPort= NULL;
	byte preAmCount=0;
	byte postAmCount=0;
	byte dataCount=0;
	byte dataSize=0;
	bool dataCollect=false;
	size_t bufferSize=0;
	byte lastByte=0;

};
};
#endif /* SERIALRX_H_ */
