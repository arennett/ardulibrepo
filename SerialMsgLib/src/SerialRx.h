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



class SerialRx {
public:
	/**
	 * SoftSerialRx(byte pinRx,byte pinTy,size_t maxDataSize);
	 * > constructor
	 * pinRx  		...serial rx pin number
	 * pinrRy 		...serial tx pin number
	 * maxDataSize 	...data size, to create an internal buffer (dataSize + postamble size)
	 */
	SerialRx(SerialPort* pSerialPort,size_t maxDataSize);


	/**
	 * void setUpdateCallback(void (*ptr)(byte* data, size_t data_size));
	 * > registering a callback method
	 * > method will be called when data is completely received
	 * > see also readNext()
	 * pData 		...pointer on the received data
	 * data_size 	...what you think ? ;-)
	 */
	void setUpdateCallback(void (*ptr)(byte* pData, size_t data_size));

	/**
	 * void begin(long speed);
	 * > like Serial.begin , inits serial connection
	 */
	void begin(long speed);

	/**
	 * readNext();
	 * > reads next byte into the buffer
	 * < return : true when data complete and callBack was called
	 */
	bool readNext();

	/**
	 * bool readNext(byte* b);
	 * > reads next byte into the buffer
	 * < return : true when data complete and callBack was called
	 * b	...current byte (can be data or pre-/postamble bytes
	 */

	bool readNext(byte* b);

	/**
	 * bool waitOnMessage(byte* data, size_t& data_size, unsigned long timeout);
	 * > waits until complete message is received or timeout is expired
	 * ppData 		...reference for : pointer on the received data
	 * data_size 	...reference data size
	 * timeout		...timeout msecs
	 * checkPeriod  ...time until next read trial is done
	 *
	 */
	bool waitOnMessage(byte*& pData, size_t& data_size, unsigned long timeout, unsigned long checkPeriod);

	/**
		 * bool waitOnMessage(byte* data, size_t& data_size, unsigned long timeout);
		 * > waits until complete message is received or timeout is expired
		 * > the checkPeriod is 10msec
		 * ppData 		...reference for : pointer on the received data
		 * data_size 	...reference data size
		 * timeout		...timeout msecs
		 *
		 *
		 */
	bool waitOnMessage(byte*& pData, size_t& data_size, unsigned long timeout);

	/**
	 * bool listen ();
	 * >if multiple software serials are used, listen
	 * >activate this software serial connection
	 * >since they are concurrent
	 * <returns: true ...if other connection was deactivated
	 */
	bool listen ();

	/**
	 *  SerialPort* getSerialPort()
	 * <returns: pointer on internal port
	 */
	 SerialPort* getSerialPort();

	/**
	 * ~SoftSerialRx();
	 * > destructor deletes internal softSerial
	 * > if constructor with softSerial was used, it does not
	 */
	virtual ~SerialRx();
private:
	void (*updateCallback)(byte* data, size_t data_size);
	byte* pRecBuffer;
	SerialPort* pSerialPort;
	byte preAmCount=0;
	byte postAmCount=0;
	byte dataCount=0;
	byte prevDataCount=0;
	bool dataCollect=false;
	size_t bufferSize=0;

	byte lastByte ;

};

#endif /* SERIALRX_H_ */
